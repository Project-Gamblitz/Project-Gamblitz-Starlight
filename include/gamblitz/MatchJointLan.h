#pragma once

#include "types.h"
#include "flexlion/ProcessMemory.hpp"

// ============================================================================
// MatchJointLan — LAN Splatfest/League MatchJoint Implementation
// ============================================================================
//
// Problem: Splatfest and League modes use a "MatchJoint" system that merges
// two 4-player team sessions into one 8-player match. This is implemented via
// PIA's INET joint session mechanism (NexMatchJointSessionJob), which does not
// exist on LAN. As a result, the Joint never completes and the game errors out
// with code 0x30D4B (timeout in stateMoveJoint).
//
// Solution: On LAN we patch the following:
// 1. stateAutoJointSession (DefaultMatchingSeq state 16): on LAN the isMerged()
//    NEX check always returns false so the state loops forever. We replace it
//    with a hook that waits 1 tick (so calc() can latch the 0x1000 flag), then
//    immediately replicates the INET success path → state 13 (cMatching).
//    The latched 0x1000 drives MatchmakeExeBase 12→13→14 naturally.
// 2. stateMoveJoint (state 14): replaced to skip INET joint management.
//    After 20 frames the session master arms the MatchmakeTimer (bytes 36+24)
//    so checkRecoverySyncCloneAfterMoveJoint may fire early. After 120 frames
//    all players unconditionally advance to state 15.
// 3. updateJointSessionState: no-op (no joint session to manage on LAN).
// 4. checkCloseJointSessionCondition: always returns true on LAN.
// 5. onUpdateJointSessionState (Fest-specific): no-op on LAN.
// ============================================================================

namespace MatchJointLan {
    void init();
}

// Hook functions (called from .slpatch BL/B directives)
void stateAutoJointSessionHook(void* a1);   // DefaultMatchingSeq state 16 — critical LAN fix
void stateMoveJointHook(void* thisPtr);
void updateJointSessionStateHook(void* thisPtr);
bool checkCloseJointSessionConditionHook(void* thisPtr);
void onUpdateJointSessionStateHook(void* thisPtr, int state);

// BL hook: replaces the PiaMatchmakeCondition::simpleSetting call inside reqAutoMatch.
// On LAN, injects the reqCreateMatch LAN session-init block (PacketLocalSessionData +
// setApplicationData + sead::Random + makeRandomCryptoKey) before simpleSetting runs.
// Patch: BL at 5.5.2 offset 0x36C960 (inside reqAutoMatch).
void autoMatchLanInitHook(void* matchCond);

