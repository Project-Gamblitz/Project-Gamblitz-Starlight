// ============================================================================
// MatchJointLan.cpp — LAN MatchJoint implementation for Splatfest/League
// ============================================================================
//
// On NEX (INET), the MatchJoint flow works like this:
//   1. Two teams of 4 each fill their own matchmaking sessions
//   2. stateMoveJointReady: session master calls reqMoveJoint()
//   3. reqMoveJoint: calls DefaultMatchingSeq::req(10) — PIA MoveJoint
//   4. PIA/NEX server merges both sessions into one 8-player joint session
//   5. stateMoveJoint: waits for merge (isMovingJoint), then syncs clones
//   6. updateJointSessionState: manages joint session lifecycle (close/open)
//   7. checkRecoverySyncCloneAfterMoveJoint: validates all clones synced
//   8. Transitions to game start
//
// On LAN, step 3-6 fail because PIA's LAN layer has no joint session support.
//
// Our fix: Replace req(10) (MoveJoint) with req(0) (autoMatch) configured
// for 8 players. One team creates the 8-player session, the other joins it.
// We bypass the INET-specific joint session management entirely.
// ============================================================================

#include "types.h"
#include "flexlion/ProcessMemory.hpp"
#include "flexlion/FsLogger.hpp"
#include "Cmn/StaticMem.h"
#include "gamblitz/MatchJointLan.h"

// ---------------------------------------------------------------------------
// Function pointers to game functions (resolved at init time)
// ---------------------------------------------------------------------------

// Lp::Utl::Matching::isInetMode() — returns true if INET, false if LAN
// 5.5.2 addr: 0x19F29E4
static bool (*isInetMode)() = nullptr;

// Lp::Utl::Matching::isSessionMaster() — returns true if this console is session master
// 5.5.2 addr: 0x19F29A0
static bool (*isSessionMaster)() = nullptr;

// Lp::Utl::Matching::getConnectedNum() — returns number of connected stations
// We resolve this dynamically
static int (*getConnectedNum)() = nullptr;

// NOTE: We do NOT store "original" function pointers for any address that is
// hooked via `B` in the .slpatch. The `B` instruction replaces the first word
// of the target function with an unconditional branch to our hook. Calling the
// address back would just re-enter our hook → infinite recursion.
// Hooked addresses (DO NOT resolve these): 0x194bf8c, 0x36B82C,
// 0x36D258, 0x36D358, 0x373F7C.
// (0x36CE68 and 0x399B24 were previously hooked but are no longer patched.)

// Cnet::MatchmakeExeBase::checkCloneConnect (NOT hooked — safe to call)
// 5.5.2 addr: 0x36D484
static bool (*checkCloneConnectOriginal)(void* thisPtr) = nullptr;

// Cnet::MatchmakeExeBase::checkRecoverySyncCloneAfterMoveJoint (original)
// 5.5.2 addr: 0x36DA7C
static bool (*checkRecoverySyncCloneAfterMoveJointOriginal)(void* thisPtr) = nullptr;

// Cnet::PlayerInfoSharingExe::Arg::Arg
// 5.5.2 addr: derived from disasm of stateMoveJoint
static void (*playerInfoSharingExeArgCtor)(void* arg) = nullptr;

// Cnet::PlayerInfoSharingExe::start
static void (*playerInfoSharingExeStart)(void* exe, void* arg) = nullptr;

// Cnet::Utl::setMySeqState
static void (*setMySeqState)(int state) = nullptr;

// Cnet::PlayerInfoSharingExe::reqSendFlagFixStandby
static void (*reqSendFlagFixStandby)(void* exe, bool flag) = nullptr;

// Cnet::MatchmakeExeBase::checkDisconnectSameSessionPlayer
static bool (*checkDisconnectSameSessionPlayer)(void* thisPtr) = nullptr;

// Lp::Utl::StateMachine::changeState
static void (*changeState)(void* sm, int state) = nullptr;

// Cnet::MatchmakeTimer::setResetTimeByAfterJoint
static void (*setResetTimeByAfterJoint)(void* timer) = nullptr;

// Lp::Utl::Limit::setNetworkErrorForAppSession
static void (*setNetworkErrorForAppSession)(int errorCode) = nullptr;

// Lp::Net::EnlTask::sInstance
static u64 *enlTaskInstance = nullptr;

// DefaultMatchingSeq req function
// req(0) = autoMatch, req(1) = createMatch, req(5) = joinMatch, req(10) = moveJoint
static bool (*defaultMatchingSeqReq)(void* seq, int reqType) = nullptr;

// enl::PiaMatchmakeCondition functions
static void (*setCreateMaxParticipants)(void* cond, int max) = nullptr;
static void (*setCreateOpenParticipation)(void* cond, int open) = nullptr;
static void (*simpleSetting)(void* cond) = nullptr;
static void (*makeRandomCryptoKey)(void* cond, const u32* key, u32 keyLen, void* random) = nullptr;

// Lp::Net::MatchingSeqBase functions
static void* (*getMatchmakeCondition)(void* seq) = nullptr;
static void (*setDefaultMatchConfig)(void* seq, int config) = nullptr;

// Lp::Utl::Limit::getDefaultMatchingSeq
static void* (*getDefaultMatchingSeq)(void* thisPtr) = nullptr;

// Cnet::Utl::setMatchmakeFlag
static void (*setMatchmakeFlag)(int a, int b) = nullptr;

// sead::Random::init
static void (*seadRandomInit)(void* random, u32 seed) = nullptr;

// Static crypto key array used in reqJoinMatch/reqCreateMatch for LAN
// 5.5.2 addr: data at 0x24BE99C (same as used in reqJoinMatch)
static u32 *cryptoKeyData = nullptr;

// Cnet::PacketLocalSessionData::PacketLocalSessionData() — LAN session data ctor
// Called on a 74-byte stack buffer; initialises the struct to its defaults.
// 5.5.2 addr: 0x37D704
static void (*packetLocalSessionDataCtor)(void* pkt) = nullptr;

// enl::PiaMatchmakeCondition::setApplicationData(data, size)
// Attaches the 74-byte LAN session blob to the matchmake condition.
// 5.5.2 addr: 0x151B9B4
static void (*setApplicationData)(void* cond, const void* data, u16 size) = nullptr;

// Flag tracking whether we're in a LAN joint
static bool sLanJointActive = false;

// Frame counter for LAN joint (reused in stateMoveJoint)
static int sLanJointFrameCounter = 0;

// 2-tick delay flag for stateAutoJointSessionHook.
// On the FIRST tick DefaultMatchingSeq enters state 16, we do NOT transition yet.
// This guarantees calc() has at least one tick to see state 16 and set the 0x1000
// flag in MatchmakeExeBase, regardless of whether calc() or the state handler runs
// first within a given tick. On the SECOND tick, 0x1000 is already set, and we
// safely transition DefaultMatchingSeq to state 13 (cMatching).
static bool sAutoJointPrimed = false;

// ---------------------------------------------------------------------------
// Initialization — resolve all function pointers
// ---------------------------------------------------------------------------
void MatchJointLan::init() {
    // Core matching utility functions (addresses from 5.5.1/5.5.2 IDA)
    isInetMode = (bool (*)())ProcessMemory::MainAddr(0x19F29E4);
    isSessionMaster = (bool (*)())ProcessMemory::MainAddr(0x19F29A0);
    // NOTE: stateAutoJointSession (0x194bf8c) is HOOKED via B — do NOT resolve it

    // NON-hooked MatchmakeExeBase functions (safe to call — addresses NOT patched)
    checkCloneConnectOriginal = (bool (*)(void*))ProcessMemory::MainAddr(0x36D484);
    checkRecoverySyncCloneAfterMoveJointOriginal = (bool (*)(void*))ProcessMemory::MainAddr(0x36DA7C);
    checkDisconnectSameSessionPlayer = (bool (*)(void*))ProcessMemory::MainAddr(0x36D988);

    // PlayerInfoSharingExe
    // Arg ctor: 0x370164, start: 0x370A88, reqSendFlagFixStandby: 0x370B50
    playerInfoSharingExeArgCtor = (void (*)(void*))ProcessMemory::MainAddr(0x370164);
    playerInfoSharingExeStart = (void (*)(void*, void*))ProcessMemory::MainAddr(0x370A88);
    reqSendFlagFixStandby = (void (*)(void*, bool))ProcessMemory::MainAddr(0x370B50);

    // State management
    setMySeqState = (void (*)(int))ProcessMemory::MainAddr(0x39977C);
    changeState = (void (*)(void*, int))ProcessMemory::MainAddr(0x19F8578);
    setNetworkErrorForAppSession = (void (*)(int))ProcessMemory::MainAddr(0x19F31A0);
    setMatchmakeFlag = (void (*)(int, int))ProcessMemory::MainAddr(0x399AD4);

    // Matching timer
    setResetTimeByAfterJoint = (void (*)(void*))ProcessMemory::MainAddr(0x37A058);

    // DefaultMatchingSeq
    defaultMatchingSeqReq = (bool (*)(void*, int))ProcessMemory::MainAddr(0x1948E00);

    // PiaMatchmakeCondition
    setCreateMaxParticipants = (void (*)(void*, int))ProcessMemory::MainAddr(0x151B010);
    setCreateOpenParticipation = (void (*)(void*, int))ProcessMemory::MainAddr(0x151B420);
    simpleSetting = (void (*)(void*))ProcessMemory::MainAddr(0x151BC78);
    makeRandomCryptoKey = (void (*)(void*, const u32*, u32, void*))ProcessMemory::MainAddr(0x151BB00);

    // MatchingSeqBase
    getMatchmakeCondition = (void* (*)(void*))ProcessMemory::MainAddr(0x1953BD8);
    setDefaultMatchConfig = (void (*)(void*, int))ProcessMemory::MainAddr(0x1953E20);

    // getDefaultMatchingSeq
    getDefaultMatchingSeq = (void* (*)(void*))ProcessMemory::MainAddr(0x19F33C8);

    // sead::Random::init(u32 seed) — Xorshift PRNG initialiser
    // 5.5.1 verified: _ZN4sead6Random4initEj @ IDA 0x71017379A0 → offset 0x17379A0
    // sead library is unchanged between 5.5.1 and 5.5.2; same offset applies.
    // (0x159D68 was WRONG — that address is a float-math function → crash)
    seadRandomInit = (void (*)(void*, u32))ProcessMemory::MainAddr(0x17379A0);

    // Crypto key data for LAN sessions (same data used in reqJoinMatch LAN path)
    cryptoKeyData = (u32*)ProcessMemory::MainAddr(0x24BE99C);

    // AutoMatch LAN init helpers (reqCreateMatch LAN block)
    packetLocalSessionDataCtor = (void (*)(void*))ProcessMemory::MainAddr(0x37D704);
    setApplicationData = (void (*)(void*, const void*, u16))ProcessMemory::MainAddr(0x151B9B4);

    // Connected num
    getConnectedNum = (int (*)())ProcessMemory::MainAddr(0x19F294C);

    // EnlTask singleton
    enlTaskInstance = (u64*)ProcessMemory::MainAddr(0x2D02AB0);

    sLanJointActive = false;
    sLanJointFrameCounter = 0;
    sAutoJointPrimed = false;

    FsLogger::LogFormatDefaultDirect("[MatchJointLan] Initialized.\n");
}

// ---------------------------------------------------------------------------
// Helper: get the MatchmakeExeBase "this" pointer fields
// The MatchmakeExeBase layout (from IDA decompilation):
//   +0x00 vtable
//   +0x08 StateMachine
//   +0x38 timer (int, frame counter for current state)
//   +0x58 PlayerInfoSharingExe*
//   +0x60 MatchSessionInfo*
//   +0x70 MatchmakeTimer*
//   +0x80 maxParticipants (int) — offset 0x80 = field at (this+32) in decompile
//   +0x84 jointMaxParticipants (int) — offset 0x84 = field at (this+33)
//   +0x9C expectedPlayers (int) — offset 0x9C = (this+39)
//   +0xAC moveJointFrameCounter (int)
//   +0xB8 flags (int)
//   +0xB9 flags2 (byte at +185)
// ---------------------------------------------------------------------------

static inline u64 getVtable(void* thisPtr) {
    return *(u64*)thisPtr;
}

static inline void* getPlayerInfoSharingExe(void* thisPtr) {
    return *(void**)((u8*)thisPtr + 0x58);
}

static inline void* getMatchmakeTimer(void* thisPtr) {
    return *(void**)((u8*)thisPtr + 0x70);
}

static inline int getMoveJointFrameCounter(void* thisPtr) {
    return *(int*)((u8*)thisPtr + 0xAC);
}

static inline void setMoveJointFrameCounter(void* thisPtr, int val) {
    *(int*)((u8*)thisPtr + 0xAC) = val;
}

static inline int getFlags(void* thisPtr) {
    return *(int*)((u8*)thisPtr + 0xB8);
}

static inline void setFlags(void* thisPtr, int val) {
    *(int*)((u8*)thisPtr + 0xB8) = val;
}

static inline u8 getFlagsByte185(void* thisPtr) {
    return *((u8*)thisPtr + 0xB9);
}

static inline int getTimerFrames(void* thisPtr) {
    return *(int*)((u8*)thisPtr + 0x38);
}

static inline void* getStateMachine(void* thisPtr) {
    return (void*)((u8*)thisPtr + 0x08);
}

// ---------------------------------------------------------------------------
// stateAutoJointSessionHook — Replaces DefaultMatchingSeq::stateAutoJointSession
//
// FULL REPLACEMENT (slpatch `B` at 0x194bf8c). Cannot call original.
//
// On INET, state 16 (cAutoJointSession) waits for the NEX joint session
// isMerged() vtable check (at *(sFrameworkInstance+0x28) vtable+0xC0).
// On LAN that call always returns false → state 16 loops forever, the 0x1000
// flag never gets set, and MatchmakeExeBase never advances past state 12.
//
// Fix: Replicate the INET success path for LAN, with a 1-tick delay —
//   Tick 1 in state 16: return immediately (stay in state 16 one extra tick so
//   calc() can call isMovingJoint() = true and latch 0x1000 in MatchmakeExeBase).
//   Tick 2 in state 16: 0x1000 is now latched. Clear disconnect bits, clear retry
//   flag, set source=3, then transition DefaultMatchingSeq to state 13 (cMatching).
//   The latched 0x1000 then drives MatchmakeExeBase 12→13→14 normally.
//
// a1 = DefaultMatchingSeq*; all byte offsets verified from 5.5.1 IDA decompile.
// ---------------------------------------------------------------------------
void stateAutoJointSessionHook(void* a1) {
    // 2-tick delay: on the FIRST tick in state 16, return without transitioning.
    // This guarantees calc() sees state 16 → isMovingJoint() = true → sets 0x1000
    // in MatchmakeExeBase, regardless of whether calc() or this handler runs first
    // within the same tick. On the SECOND tick, 0x1000 is already latched and the
    // guard in calc() prevents it from being cleared, so we can safely transition.
    if (!sAutoJointPrimed) {
        sAutoJointPrimed = true;
        FsLogger::LogFormatDefaultDirect("[MatchJointLan] stateAutoJointSession LAN — tick 1, waiting for calc() to latch 0x1000\n");
        return;
    }
    sAutoJointPrimed = false;

    // Replicate the INET isMerged success path (skip INET-only *(a1+1050)/*(a1+1024) flags)
    *(u32*)((u8*)a1 + 0xF4) &= ~3u;   // *(a1+244) &= ~3  — clear disconnect mode bits
    *((u8*)a1 + 0x421) = 0;            // *(a1+1057) = 0   — clear joint retry flag
    *(u32*)((u8*)a1 + 0xF0) = 3;      // *(a1+240) = 3    — source = post-merge marker
    changeState((u8*)a1 + 0x38, 13);  // DefaultMatchingSeq StateMachine → cMatching (13)
    FsLogger::LogFormatDefaultDirect("[MatchJointLan] stateAutoJointSession LAN — 0x1000 latched, moved to state 13\n");
}

// ---------------------------------------------------------------------------
// reqMoveJointHook — PREVIOUSLY hooked Cnet::MatchmakeExeBase::reqMoveJoint
//
// NO LONGER PATCHED in 552.slpatch (patch line 0036CE68 removed).
// The original req(8) flow from stateMatching drives DefaultMatchingSeq into
// state 16 naturally. This hook was using req(0)/autoMatch which was wrong.
//
// Kept here for reference only — not called by anything.
// ---------------------------------------------------------------------------
void reqMoveJointHook(void* thisPtr) {
    if (isInetMode()) {
        // INET mode: should never happen in this LAN mod.
        // Cannot call original (address is patched). Log and bail.
        FsLogger::LogFormatDefaultDirect("[MatchJointLan] reqMoveJoint called in INET mode — unexpected!\n");
        return;
    }

    // --- LAN MODE ---
    FsLogger::LogFormatDefaultDirect("[MatchJointLan] reqMoveJoint LAN — using autoMatch for 8 players\n");

    // Get the DefaultMatchingSeq and MatchmakeCondition
    // We use the same pattern as reqAutoMatch but with MoveJoint's max participants
    u64 vtable = getVtable(thisPtr);

    // Call getDefaultMatchingSeq: vtable call or direct
    // From the decompilation: DefaultMatchingSeq = getDefaultMatchingSeq(thisPtr)
    // We use the function pointer resolved at init
    void* defaultMatchSeq = getDefaultMatchingSeq(thisPtr);
    void* matchCond = getMatchmakeCondition(defaultMatchSeq);

    // Configure for 8 players (joint max)
    setDefaultMatchConfig(defaultMatchSeq, 1);
    int jointMax = *(int*)((u8*)thisPtr + 0x84); // this->jointMaxParticipants
    if (jointMax < 8) jointMax = 8;
    setCreateMaxParticipants(matchCond, jointMax);

    // Configure match conditions via vtable (same as original)
    // vtable[5](thisPtr, 4, 0) = configureMatchConditions(type=4=MoveJoint, 0)
    typedef void (*ConfigureFunc)(void*, int, int);
    ConfigureFunc configureConds = (ConfigureFunc)(*(u64*)(vtable + 40));
    configureConds(thisPtr, 4, 0);

    // Simple settings
    simpleSetting(matchCond);

    // LAN crypto key (same as reqCreateMatch LAN path)
    u8 randomBuf[0x20];
    seadRandomInit(randomBuf, 0xCEB9D8D9);
    makeRandomCryptoKey(matchCond, cryptoKeyData, 64, randomBuf);

    // Post-configure via vtable
    // vtable[6](thisPtr, 4) = postConfigureMatchConditions(type=4)
    typedef void (*PostConfigureFunc)(void*, int);
    PostConfigureFunc postConfigure = (PostConfigureFunc)(*(u64*)(vtable + 48));
    postConfigure(thisPtr, 4);

    // Use autoMatch (req(0)) instead of MoveJoint (req(10))
    // autoMatch will create a session if none found, or join if one exists
    defaultMatchingSeqReq(defaultMatchSeq, 0);

    sLanJointActive = true;
    sLanJointFrameCounter = 0;

    FsLogger::LogFormatDefaultDirect("[MatchJointLan] reqMoveJoint LAN — autoMatch(0) dispatched for %d players\n", jointMax);
}

// ---------------------------------------------------------------------------
// isMovingJointHook — PREVIOUSLY hooked Cnet::Utl::isMovingJoint
//
// NO LONGER PATCHED in 552.slpatch (patch line 00399B24 removed).
// Returning false here prevented calc() from ever setting the 0x1000 flag,
// which blocked MatchmakeExeBase state 12→13→14 transitions entirely.
// The natural behavior (false while DefaultMatchingSeq is not in state 16/21,
// and then transitioning naturally) is correct.
//
// Kept here for reference only — not called by anything.
// ---------------------------------------------------------------------------
bool isMovingJointHook() {
    return false;
}

// ---------------------------------------------------------------------------
// stateMoveJointHook — Replaces Cnet::MatchmakeExeBase::stateMoveJoint
//
// FULL REPLACEMENT (slpatch `B` at 0x36B82C). Cannot call original.
//
// Reimplements the original state machine flow for LAN, skipping all
// INET-specific joint session operations:
//   1. Skip isMovingJoint (always false on LAN; natural flow handles it now)
//   2. checkCloneConnect() → if false, wait
//   3. If first time (frameCounter == 0): start PlayerInfoSharingExe,
//      call onAfterJointConnect vtable, setMySeqState(1)
//   4. Increment frame counter
//   5. After 20 frames AND isSessionMaster: arm timer (bytes 36+24 set so that
//      checkRecoverySyncCloneAfterMoveJoint's LAN branch returns true)
//   6. After 120 frames: force advance to state 15 regardless (LAN fallback —
//      guarantees all players advance even if the timer path is not triggered)
//   6b. checkRecoverySyncCloneAfterMoveJoint → if true early, move to state 15
//   7. checkDisconnectSameSessionPlayer → if true, move to state 26
//   8. After 3600 frames: error 0x30D4B
// ---------------------------------------------------------------------------
void stateMoveJointHook(void* thisPtr) {
    if (isInetMode()) {
        // INET mode: should never happen in this LAN mod.
        FsLogger::LogFormatDefaultDirect("[MatchJointLan] stateMoveJoint called in INET mode — unexpected!\n");
        return;
    }

    // --- LAN MODE ---
    // Step 1: Skip isMovingJoint (we know it's not merging on LAN)
    // Step 2: Check clone connections
    if (!checkCloneConnectOriginal(thisPtr)) {
        // Not all clones connected yet — reset counter and wait
        setMoveJointFrameCounter(thisPtr, 0);
        return;
    }

    // Step 3: First-time setup
    int frameCounter = getMoveJointFrameCounter(thisPtr);
    if (frameCounter == 0) {
        // Start PlayerInfoSharingExe
        u8 argBuf[8];
        playerInfoSharingExeArgCtor(argBuf);
        void* piSharingExe = getPlayerInfoSharingExe(thisPtr);
        playerInfoSharingExeStart(piSharingExe, argBuf);

        // Call onAfterJointConnect vtable[2] (offset 0x10)
        u64 vtable = getVtable(thisPtr);
        typedef void (*OnAfterJointFunc)(void*);
        OnAfterJointFunc onAfterJoint = (OnAfterJointFunc)(*(u64*)(vtable + 0x10));
        onAfterJoint(thisPtr);

        // Set seq state to 1 (connected)
        setMySeqState(1);
    }

    // Step 4: Increment frame counter
    setMoveJointFrameCounter(thisPtr, frameCounter + 1);

    // Step 5: After 20 frames, session master arms the timer guard
    if (frameCounter >= 19) {
        if (isSessionMaster()) {
            void* timer = getMatchmakeTimer(thisPtr);
            u8 timerByte = *((u8*)timer + 0x26); // *(timer + 38) = guard flag
            if (!timerByte) {
                setResetTimeByAfterJoint(timer);   // sets timer byte 16 only
                *((u8*)timer + 0x25) = 1;          // *(timer + 37) = 1
                *((u8*)timer + 0x26) = 1;          // *(timer + 38) = 1 (guard)
                // LAN FIX: also set bytes 36 and 24 so that
                // checkRecoverySyncCloneAfterMoveJoint's LAN branch can fire.
                // On LAN, ValidSameSessionPlayerNum < 1, so the function checks:
                //   if (*(timer+36)) return *(timer+24) > 0;
                // setResetTimeByAfterJoint only sets byte 16 — never 36 or 24.
                *((u8*)timer + 36) = 1;  // enable the timer path
                *((u8*)timer + 24) = 1;  // return value: timer[24] > 0 → true
            }
            // On LAN, skip updateJointSessionState — no joint session to manage
        }

        // Step 6: LAN fallback — after 120 frames (2s at 60fps), force advance
        // checkRecoverySyncCloneAfterMoveJoint may now work (bytes 36/24 set above),
        // but the 120-frame path guarantees progress for non-session-master consoles
        // and any edge case where the timer bytes aren't set in time.
        if (frameCounter >= 120) {
            void* piSharingExe = getPlayerInfoSharingExe(thisPtr);
            reqSendFlagFixStandby(piSharingExe, false);

            int flags = getFlags(thisPtr);
            flags &= ~0x800; // clear the "in joint" flag
            setFlags(thisPtr, flags);

            setMySeqState(2); // seq state = ready
            changeState(getStateMachine(thisPtr), 15); // state 15 = cJointMatching

            sLanJointActive = false;
            FsLogger::LogFormatDefaultDirect("[MatchJointLan] stateMoveJoint LAN — joint complete (120-frame forced), moving to state 15\n");
            return;
        }

        // Step 6b: also try the normal path — may fire early if timer bytes are set
        if (checkRecoverySyncCloneAfterMoveJointOriginal(thisPtr)) {
            void* piSharingExe = getPlayerInfoSharingExe(thisPtr);
            reqSendFlagFixStandby(piSharingExe, false);

            int flags = getFlags(thisPtr);
            flags &= ~0x800;
            setFlags(thisPtr, flags);

            setMySeqState(2);
            changeState(getStateMachine(thisPtr), 15);

            sLanJointActive = false;
            FsLogger::LogFormatDefaultDirect("[MatchJointLan] stateMoveJoint LAN — joint complete (clone sync), moving to state 15\n");
            return;
        } else {
            // Check for disconnect
            if (checkDisconnectSameSessionPlayer(thisPtr)) {
                changeState(getStateMachine(thisPtr), 26); // state 26 = disconnect recovery
                sLanJointActive = false;
                return;
            }
        }
    }

    // Step 8: Timeout check (3600 frames = 60 seconds at 60fps)
    if (getMoveJointFrameCounter(thisPtr) >= 3600) {
        FsLogger::LogFormatDefaultDirect("[MatchJointLan] stateMoveJoint LAN — TIMEOUT (3600 frames)\n");
        setNetworkErrorForAppSession(0x30D4B);
        sLanJointActive = false;
    }
}

// ---------------------------------------------------------------------------
// updateJointSessionStateHook — Replaces updateJointSessionState
//
// FULL REPLACEMENT (slpatch `B` at 0x36D258). Cannot call original.
// On LAN: no-op — there is no joint session to manage.
// ---------------------------------------------------------------------------
void updateJointSessionStateHook(void* thisPtr) {
    // LAN: nothing to do — joint session management is INET-only
}

// ---------------------------------------------------------------------------
// checkCloseJointSessionConditionHook — Replaces checkCloseJointSessionCondition
//
// FULL REPLACEMENT (slpatch `B` at 0x36D358). Cannot call original.
// On LAN: return true immediately — no joint session to close.
// ---------------------------------------------------------------------------
bool checkCloseJointSessionConditionHook(void* thisPtr) {
    // LAN: always true — skip joint session closing flow
    return true;
}

// ---------------------------------------------------------------------------
// onUpdateJointSessionStateHook — Replaces VersusFestMatchmakeExe override
//
// FULL REPLACEMENT (slpatch `B` at 0x373F7C). Cannot call original.
// On LAN: no-op — no joint session attributes to modify.
// ---------------------------------------------------------------------------
void onUpdateJointSessionStateHook(void* thisPtr, int state) {
    // LAN: nothing to do
}

// ---------------------------------------------------------------------------
// autoMatchLanInitHook — BL hook on PiaMatchmakeCondition::simpleSetting
//                         inside Cnet::MatchmakeExeBase::reqAutoMatch
//
// PATCH TYPE: BL (not B) at 5.5.2 offset 0x36C960 — we REPLACE the call
// site, not the function.  We receive X0 = matchCond and must call the real
// simpleSetting ourselves before returning.
//
// Problem: reqAutoMatch on LAN fails with a network error because it never
// runs the session-init block that reqCreateMatch runs (offsets 0x36CB80–
// 0x36CBCC in 5.5.1).  That block:
//   1. Constructs a PacketLocalSessionData (74 bytes) on the stack
//   2. Calls setApplicationData(matchCond, &pkt, 74) — attaches the blob
//   3. Calls sead::Random::init(&rand, 0xCEB9D8D9) — seed the PRNG
//   4. Calls makeRandomCryptoKey(matchCond, cryptoKeyData, 64, &rand)
//
// Fix: Run steps 1–4 here before forwarding to simpleSetting, but only on
// LAN (isInetMode() == false).
// ---------------------------------------------------------------------------

// DO NOT TOUCH I SWEAR TO GOD
void autoMatchLanInitHook(void* matchCond) {
    if (!isInetMode()) {
        // --- LAN: inject the reqCreateMatch session-init block ---

        // Step 1: Construct PacketLocalSessionData on the stack (74 bytes)
        // The ctor zero-initialises the struct; we leave the "private flag"
        // byte (offset 0x28) at 0 — autoMatch is always public.
        alignas(8) u8 pktBuf[80]; // 74 needed; pad to 80 for alignment safety
        packetLocalSessionDataCtor(pktBuf);
        // pktBuf[0x28] left as 0 (no password / not private)

        // Step 2: Attach the blob to the matchmake condition
        setApplicationData(matchCond, pktBuf, (u16)74);

        // Step 3 & 4: Seed the PRNG and generate a random session crypto key
        // sead::Random state = 4 x u32 = 16 bytes
        alignas(8) u32 randState[4];
        seadRandomInit(randState, (u32)0xCEB9D8D9);
        makeRandomCryptoKey(matchCond, cryptoKeyData, (u32)64, randState);

        FsLogger::LogFormatDefaultDirect("[MatchJointLan] autoMatchLanInitHook: LAN session init done\n");
    }

    // Always call the real PiaMatchmakeCondition::simpleSetting
    simpleSetting(matchCond);
}
