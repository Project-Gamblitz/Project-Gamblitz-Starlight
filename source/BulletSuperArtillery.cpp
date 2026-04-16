#include "flexlion/BulletSuperArtillery.hpp"
#include "flexlion/InkstrikeMgr.hpp"
#include "Game/BulletBombBase.h"
#include "Game/Player/Player.h"
#include "Game/Player/PlayerDamage.h"
#include "Game/MainMgr.h"
#include "Game/PaintUtl.h"
#include "Game/DamageReason.h"
#include "Game/SighterTarget.h"
#include "nn/fs.h"
#include "flexlion/FsLogger.hpp"

extern "C" {
    void _ZN3Cmn5ActorC2Ev(void *);
    void _ZN6xlink222EnumPropertyDefinitionC2EPKcb(void *, const char *, bool);
    void _ZN6xlink222EnumPropertyDefinition12setEntryBuf_EiPNS0_5EntryE(void *, int, void *);
    void _ZN6xlink222EnumPropertyDefinition5entryEiPKc(void *, int, const char *);
    void _ZN2Lp3Sys5XLink12killAllSoundEv(void *xlink);
    // VTV symbols for manual F32PropertyDefinition construction
    extern u8 _ZTVN6xlink221F32PropertyDefinitionE[];
    extern u8 _ZTVN4sead22BufferedSafeStringBaseIcEE[];
	extern bool gSpecialWeaponPaint;
}
#define CmnActorCtor _ZN3Cmn5ActorC2Ev
#define EnumPropDefCtor _ZN6xlink222EnumPropertyDefinitionC2EPKcb
#define EnumPropDefSetEntryBuf _ZN6xlink222EnumPropertyDefinition12setEntryBuf_EiPNS0_5EntryE
#define EnumPropDefEntry _ZN6xlink222EnumPropertyDefinition5entryEiPKc
#define XLinkKillAllSound _ZN2Lp3Sys5XLink12killAllSoundEv

// Flight parameters
const int BSA_FLIGHT_TIME = 120;
const float BSA_FLIGHT_HEIGHT = 300.0f;
const float tornadoTankZOffset = -3.0f;

// Burst parameters
const float BSA_BURST_RADIUS_START = 0.0f;   // Initial paint/hitbox radius
const float BSA_BURST_RADIUS_MAX   = 300.0f;  // Maximum radius 
const float BSA_BURST_RADIUS_GROW  = 0.5f;    // Radius growth per frame
const float BSA_BURST_TEX_ROTATION = 30.0f; // texture rotation per paint
const float BSA_BURST_ROT_FRAMES   = 2.0f; // how many frames per rotation step (can be different)
const int  	BSA_BURST_FRAMES	   = 2;		// how many frames each time paint is applied
const int   BSA_BURST_DAMAGE       = 25;       // 2.5 HP per frame (internal units: 25 = 2.5% of 1000 max HP)
const float BSA_BURST_DMG_START	   = 120.0f;  // 12.0 HP at burst start
const float BSA_BURST_DMG_END      = 30.0f;   // 3.0 HP at burst end
const int   BSA_BURST_DURATION     = 100;      // Frames before paint burst ends
const int   BSA_BURST_DMG_DURATION = 90;      // Frames damage duration

namespace Flexlion {

// ============================================================
// Vtable
// ============================================================

static bool sInitVtable = false;
Cmn::ActorVtable sBSAVtable;

void initBSAVtable(BulletSuperArtillery *bsa) {
    Cmn::ActorVtable **vtable = (Cmn::ActorVtable **)bsa;
    if (!sInitVtable) {
        memcpy(&sBSAVtable, *vtable, sizeof(Cmn::ActorVtable));
        sBSAVtable.getClassName = (u64)((const char *(*)(BulletSuperArtillery *))&BulletSuperArtillery::vtGetClassName);
        sBSAVtable.firstCalc = (u64)((void (*)(BulletSuperArtillery *))&BulletSuperArtillery::vtFirstCalc);
		sBSAVtable.secondCalc = (u64)((void (*)(BulletSuperArtillery *))&BulletSuperArtillery::vtSecondCalc);
        sBSAVtable.fourthCalc = (u64)((void (*)(BulletSuperArtillery *))&BulletSuperArtillery::vtFourthCalc);
        sBSAVtable.onActivate = (u64)((void (*)(BulletSuperArtillery *, bool))&BulletSuperArtillery::vtOnActivate);
        sBSAVtable.onSleep = (u64)((void (*)(BulletSuperArtillery *))&BulletSuperArtillery::vtOnSleep);
        sBSAVtable.setXlinkLocalPropertyDefinition = (u64)((int (*)(BulletSuperArtillery *, int))&BulletSuperArtillery::vtSetXLinkLocalPropertyDef);
        sBSAVtable.countXlinkLocalProperty = (u64)((int (*)(BulletSuperArtillery *))&BulletSuperArtillery::vtCountXLinkLocalProperty);
        sBSAVtable.getXLinkMtx = (u64)((sead::Matrix34<float> *(*)(BulletSuperArtillery *))&BulletSuperArtillery::vtGetXLinkMtx);
        sBSAVtable.getXLinkScale = (u64)((sead::Vector3<float> *(*)(BulletSuperArtillery *))&BulletSuperArtillery::vtGetXLinkScale);
        sInitVtable = true;
    }
    *vtable = &sBSAVtable;
}

// ============================================================
// XLink "User" enum property (Player / Enemy)
// ============================================================

struct EnumEntry {
    u64 name_ptr;
    u32 value;
    u32 _pad;
};

static _BYTE sUserPropBuf[0x78];
static EnumEntry sUserEntries[2];
static bool sUserPropInit = false;

// F32PropertyDefinition for "BulletDistance" (manually constructed, no ctor symbol available)
// Layout: +0 vtable, +8 sead string vtable, +16 buf ptr, +24 capacity, +28 name
static _BYTE sDistPropBuf[0x60] __attribute__((aligned(8)));
static bool sDistPropInit = false;

void resetBSAStatics() {
    sInitVtable = false;
    sUserPropInit = false;
    sDistPropInit = false;
}

static void initUserProperty() {
    if (sUserPropInit) return;
    EnumPropDefCtor(sUserPropBuf, "User", false);
    memset(sUserEntries, 0, sizeof(sUserEntries));
    sUserEntries[0].value = (u32)-1;
    sUserEntries[1].value = (u32)-1;
    EnumPropDefSetEntryBuf(sUserPropBuf, 2, sUserEntries);
    EnumPropDefEntry(sUserPropBuf, 0, "Player");
    EnumPropDefEntry(sUserPropBuf, 1, "Enemy");
    sUserPropInit = true;
}

static void initDistProperty() {
    if (sDistPropInit) return;
    memset(sDistPropBuf, 0, sizeof(sDistPropBuf));
    // F32PropertyDefinition vtable = _ZTV + 0x10
    *(u64 *)(sDistPropBuf + 0) = (u64)(_ZTVN6xlink221F32PropertyDefinitionE + 0x10);
    // Embedded sead::BufferedSafeStringBase<char> vtable = _ZTV + 0x10
    *(u64 *)(sDistPropBuf + 8) = (u64)(_ZTVN4sead22BufferedSafeStringBaseIcEE + 0x10);
    // Buffer pointer → name chars at +28
    *(u64 *)(sDistPropBuf + 16) = (u64)(sDistPropBuf + 28);
    // Buffer capacity
    *(u32 *)(sDistPropBuf + 24) = 64;
    // Name
    memcpy(sDistPropBuf + 28, "BulletDistance", 15);
    sDistPropInit = true;
}

// ============================================================
// Constructor
// ============================================================

BulletSuperArtillery::BulletSuperArtillery() {
    // Initialize the Cmn::Actor base at offset 0
    CmnActorCtor(this);

    // Patch primary vtable to our custom one
    initBSAVtable(this);

    // Initialize state machine (3 states matching elink user definition)
    mStateMachine.initialize(cState_Count, 0, NULL);
    setStateMachineInnerPtr(&mStateMachine);

    mStateMachine.registStateName(cState_Pronounce, sead::SafeStringBase<char>("cState_Pronounce"));
    mStateMachine.mStateBuffer[cState_Pronounce] = Lp::Utl::StateMachine::Delegate<BulletSuperArtillery>(
        this, &BulletSuperArtillery::stateEnterPronounce, &BulletSuperArtillery::statePronounce, NULL);

    mStateMachine.registStateName(cState_Wait, sead::SafeStringBase<char>("cState_Wait"));
    mStateMachine.mStateBuffer[cState_Wait] = Lp::Utl::StateMachine::Delegate<BulletSuperArtillery>(
        this, &BulletSuperArtillery::stateEnterWait, &BulletSuperArtillery::stateWait, NULL);

    mStateMachine.registStateName(cState_Burst, sead::SafeStringBase<char>("cState_Burst"));
    mStateMachine.mStateBuffer[cState_Burst] = Lp::Utl::StateMachine::Delegate<BulletSuperArtillery>(
        this, &BulletSuperArtillery::stateEnterBurst, &BulletSuperArtillery::stateBurst, NULL);

    mStateMachine.registStateName(cState_Aim, sead::SafeStringBase<char>("cState_Aim"));
    mStateMachine.mStateBuffer[cState_Aim] = Lp::Utl::StateMachine::Delegate<BulletSuperArtillery>(
        this, &BulletSuperArtillery::stateEnterAim, &BulletSuperArtillery::stateAim, NULL);

    // Zero custom fields
    mSender = NULL;
    mTornadoModel = NULL;
    mFrom = sead::Vector3<float>::zero;
    mTo = sead::Vector3<float>::zero;
    mPos = sead::Vector3<float>::zero;
    mRot = sead::Vector3<float>::zero;
    mStartFrm = 0;
    mFlightActive = false;
    mHasBurst = false;
    mXLinkMtx = sead::Matrix34<float>::ident;
    mBurstRadius = 0.0f;
    mBurstFrm = 0;
	mMatchEnding = false;
}

// ============================================================
// Factory
// ============================================================

BulletSuperArtillery *BulletSuperArtillery::create(Lp::Sys::Actor *parent, gsys::Model *model, Cmn::Def::Team team) {
    Lp::Sys::ActorSystem *system = Lp::Sys::ActorSystem::sInstance;
    system->criticalSession->lock();

    BulletSuperArtillery *bsa = new BulletSuperArtillery();
    bsa->asLpActor()->mHeap = sead::HeapMgr::sInstance->getCurrentHeap();
    bsa->setActorFullModel(model);
    bsa->setTeam(team);
    bsa->mTornadoModel = model;
    bsa->asLpActor()->actorSysOnCreate(parent);

    system->criticalSession->unlock();
    return bsa;
}

// ============================================================
// Public API
// ============================================================

void BulletSuperArtillery::prepare(Game::Player *sender) {
    mSender = sender;
    mFrom = sead::Vector3<float>::zero;
    mTo = sead::Vector3<float>::zero;
    mPos = sead::Vector3<float>::zero;
    mRot = sead::Vector3<float>::zero;
    mStartFrm = 0;
    mFlightActive = false;
    mHasBurst = false;
    mBurstRadius = 0.0f;
    mBurstFrm = 0;

    // Set xlink matrix to player position so OnActivate sound plays there
    mXLinkMtx = {{
        1.0f, 0.0f, 0.0f, sender->mPosition.mX,
        0.0f, 1.0f, 0.0f, sender->mPosition.mY,
        0.0f, 0.0f, 1.0f, sender->mPosition.mZ
    }};
	
    // ALSO set the model's world matrix — XLink reads from this on first frame
    if (mTornadoModel) {
        mTornadoModel->mtx = mXLinkMtx;
        mTornadoModel->mUpdateScale |= 1;
        mTornadoModel->updateAnimationWorldMatrix_(3);
    }
	
    asLpActor()->reserveActivate(true);
}

void BulletSuperArtillery::launch(sead::Vector3<float> src, sead::Vector3<float> dst, int paintgamefrm, bool forcedByMatchEnd) {
    mFrom = src;
    mTo = dst;
    mStartFrm = paintgamefrm;
    mPos = src;
    mRot.mX = 0.0f;
    mRot.mZ = 0.0f;
    mRot.mY = atan2f(src.mX - dst.mX, src.mZ - dst.mZ) + MATH_PI;
    if (mRot.mY > MATH_PI * 2.0f) mRot.mY -= MATH_PI * 2.0f;
    mFlightActive = true;
    mMatchEnding = forcedByMatchEnd;

    // Update xlink root matrix to target position
//    mXLinkMtx = {{
//        1.0f, 0.0f, 0.0f, mTo.mX,
//        0.0f, 1.0f, 0.0f, mTo.mY,
//        0.0f, 0.0f, 1.0f, mTo.mZ
//    }};

    mStateMachine.changeState(cState_Pronounce);
}

void BulletSuperArtillery::cancel() {
    doSleep();
}

bool BulletSuperArtillery::isActive() const {
    return mFlightActive || mHasBurst;
}

void BulletSuperArtillery::reset() {
    mSender = NULL;
    mFrom = sead::Vector3<float>::zero;
    mTo = sead::Vector3<float>::zero;
    mPos = sead::Vector3<float>::zero;
    mRot = sead::Vector3<float>::zero;
    mStartFrm = 0;
    mFlightActive = false;
    mHasBurst = false;
    mBurstRadius = 0.0f;
    mBurstFrm = 0;
	mMatchEnding = false;
}

void BulletSuperArtillery::eatBombs(float radiusSq, float hitHalfHeight) {
    if (!mHasBurst) return;

    sead::Vector3<float> tornadoPos = mTo;
    int tornadoTeam = (int)*(Cmn::Def::Team*)(this->_actorBase + 0x328);

    auto iterNode = Game::BulletBombBase::getClassIterNodeStatic();
    for (Lp::Sys::Actor *actor = iterNode->derivedFrontActiveActor();
         actor != NULL; )
    {
        Lp::Sys::Actor *next = iterNode->derivedNextActiveActor(actor);
        Cmn::Actor *bomb = (Cmn::Actor *)actor;
        if ((int)bomb->mTeam != tornadoTeam) {
            u64 vtable = *(u64 *)bomb;
            typedef float* (*GetPosFunc)(void*);
            float *pos = ((GetPosFunc)(*(u64 *)(vtable + 760)))(bomb);
            if (pos) {
                float dx = pos[0] - tornadoPos.mX;
				float dz = pos[2] - tornadoPos.mZ;
				float dy = pos[1] - tornadoPos.mY;
				
				if (dx*dx + dz*dz < radiusSq && dy > -hitHalfHeight && dy < hitHalfHeight) {
                    typedef void (*InformFunc)(void*, int, int*, sead::Vector3<float>*, int, int, int, int, int);
                    int outResult = 0;
                    ((InformFunc)(*(u64 *)(vtable + 744)))(bomb, 0, &outResult, &tornadoPos, 6, tornadoTeam, 0, 0, 0);
                }
            }
        }
        actor = next;
    }
}

void BulletSuperArtillery::eatActorClass(Lp::Sys::ActorClassIterNodeBase *iterNode, float radiusSq, float hitHalfHeight, int reactionType, int vtableOffset) {
    if (!mHasBurst) return;

    sead::Vector3<float> tornadoPos = mTo;
    int tornadoTeam = (int)*(Cmn::Def::Team*)(this->_actorBase + 0x328);

    for (Lp::Sys::Actor *actor = iterNode->derivedFrontActiveActor();
         actor != NULL; )
    {
        Lp::Sys::Actor *next = iterNode->derivedNextActiveActor(actor);
        Cmn::Actor *obj = (Cmn::Actor *)actor;

        if ((int)obj->mTeam == tornadoTeam) {
            actor = next;
            continue;
        }

        u64 vtable = *(u64 *)obj;
        if (!vtable) { actor = next; continue; }
		
        // call getPos
        typedef float* (*GetPosFunc)(void*);
        u64 getPosPtr = *(u64 *)(vtable + 760);
        if (!getPosPtr) { actor = next; continue; }
        float *pos = ((GetPosFunc)getPosPtr)(obj);
        if (!pos) { actor = next; continue; }

        float dx = pos[0] - tornadoPos.mX;
        float dz = pos[2] - tornadoPos.mZ;
        float dy = pos[1] - tornadoPos.mY;

        if (dx*dx + dz*dz < radiusSq && dy > -hitHalfHeight && dy < hitHalfHeight) {
            u64 informPtr = *(u64 *)(vtable + vtableOffset);
            if (!informPtr) { actor = next; continue; }
            
            typedef void (*InformFunc)(void*, int, int*, sead::Vector3<float>*, int, int, int, int, int);
            int outResult = 0;
            ((InformFunc)informPtr)(
                obj, 0, &outResult, &tornadoPos,
                reactionType, tornadoTeam, 0, 0, 0);
        }

        actor = next;
    }
}

static float readSpongeMaxFill(Cmn::Actor *obj) {
    u64 paramBlock = *(u64 *)((u8 *)obj + 0x4D0);
    if (!paramBlock) return 5.0f;
    
    // Try cached value first
    if (*(u8 *)(paramBlock + 39)) {
        return *(float *)(paramBlock + 40);
    }
    
    // Virtual call fallback: *(*(*(paramBlock+16) + 864) + 216)(*(paramBlock+16) + 864)
    u64 provider = *(u64 *)(paramBlock + 16);
    if (!provider) return 5.0f;
    
    u64 thisPtr = provider + 864;
    u64 vtable = *(u64 *)thisPtr;
    if (!vtable) return 5.0f;
    
    typedef float* (*ParamGetFn)(u64);
    ParamGetFn fn = (ParamGetFn)(*(u64 *)(vtable + 216));
    float *result = fn(thisPtr);
    if (result) return *result;
    return 5.0f;
}

// Subtract-HP objects (Sprinkler, JumpFlag)
static void damageObjects_SubHP(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg, int hpOffset)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        if ((int)obj->mTeam == senderTeamInt) continue;
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        if (odx*odx + odz*odz < hitRadiusSq && ody > -hitHalfHeight && ody < hitHalfHeight) {
            int *hp = (int *)((u8 *)obj + hpOffset);
            int newHp = *hp - dmg;
            if (newHp < 0) newHp = 0;
            *hp = newHp;
        }
    }
}

// Add-accumulated-damage objects (Shield, Gachihoko, UmbrellaCanopy)
static void damageObjects_AddDmg(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg, int dmgOffset)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        if ((int)obj->mTeam == senderTeamInt) continue;
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        if (odx*odx + odz*odz < hitRadiusSq && ody > -hitHalfHeight && ody < hitHalfHeight) {
            int *accum = (int *)((u8 *)obj + dmgOffset);
            *accum += dmg;
        }
    }
}
// Gachihoko (Rainmaker shield) — team-directional damage
static void damageGachihokoInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg, int attackerIdx)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        if (odx*odx + odz*odz < hitRadiusSq && ody > -hitHalfHeight && ody < hitHalfHeight) {
            int directionalDmg = (senderTeamInt == 1) ? -dmg : dmg;
            int *accum = (int *)((u8 *)obj + 0x5DC);
            int newAccum = *accum + directionalDmg;
            if (newAccum > 99999) newAccum = 99999;
            if (newAccum < -99999) newAccum = -99999;
            *accum = newAccum;

            // Store attacker
            *(int *)((u8 *)obj + 0x5E0) = attackerIdx;
			
			// Hit animation flag
            *(u32 *)((u8 *)obj + 0x4F8) |= 2;
			
//            // Per-player hit timer for visual flash
//            if (attackerIdx >= 0 && attackerIdx < 10) {
//                *(int *)((u8 *)obj + 0x295C + attackerIdx * 4) = 10;
//            }

            // Burst only when HP threshold exceeded
            int absAccum = newAccum < 0 ? -newAccum : newAccum;
            if (absAccum >= 10000) {
                *(u8 *)((u8 *)obj + 0x5E4) = 1;
                *(int *)((u8 *)obj + 0x5E8) = attackerIdx;
            }
        }
    }
}

static void damageBubblesInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg, int attackerIdx, int gameFrame)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        int bubbleTeam = (int)obj->mTeam;

        // Use vtable getPos (offset 760) — bubble stores pos at +0x404, not +0x39C
        u64 vtable = *(u64 *)obj;
        typedef float* (*GetPosFunc)(void*);
        float *pos = ((GetPosFunc)(*(u64 *)(vtable + 760)))(obj);
        if (!pos) continue;

        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;

        if (odx*odx + odz*odz < hitRadiusSq && ody > -hitHalfHeight && ody < hitHalfHeight) {
            // Write to reserve damage table at +0xBF0
            // Format: [gameFrame(4), damage(4)] per slot, 80 slots
            u8 *base = (u8 *)obj;
            for (int slot = 0; slot < 80; slot++) {
                int *slotFrame = (int *)(base + 0xBF0 + slot * 8);
                int *slotDmg   = (int *)(base + 0xBF4 + slot * 8);
                if (*slotDmg == 0) {
                    // Empty slot — write current frame + damage
                    *slotFrame = gameFrame;
                    *slotDmg = (bubbleTeam == senderTeamInt) ? dmg : -dmg;
                    break;
                }
            }
        }
    }
}
// Commented as we decided to leave Stamp being able to go through
//void BulletSuperArtillery::eatStampThrow(Lp::Sys::ActorClassIterNodeBase *iterNode, float radiusSq, float hitHalfHeight) {
//    if (!mHasBurst) return;
//
//    sead::Vector3<float> tornadoPos = mTo;
//    int tornadoTeam = (int)*(Cmn::Def::Team*)(this->_actorBase + 0x328);
//
//    for (Lp::Sys::Actor *actor = iterNode->derivedFrontActiveActor();
//         actor != NULL; actor = iterNode->derivedNextActiveActor(actor))
//    {
//        Cmn::Actor *obj = (Cmn::Actor *)actor;
//        int stampTeam = (int)obj->mTeam;
//        int currentState = *(int *)((u8 *)obj + 0x9E8);
//        
//        // FsLogger::LogFormatDefaultDirect("[BSA] Stamp: team=%d tornadoTeam=%d state=%d\n", 
//        //    stampTeam, tornadoTeam, currentState);
//
//        if (stampTeam == tornadoTeam) continue;
//        if (currentState != 6) continue;
//
//        u64 vtable = *(u64 *)obj;
//        typedef float* (*GetPosFunc)(void*);
//        float *pos = ((GetPosFunc)(*(u64 *)(vtable + 760)))(obj);
//        if (!pos) continue;
//
//        float dx = pos[0] - tornadoPos.mX;
//		float dz = pos[2] - tornadoPos.mZ;
//		float dy = pos[1] - tornadoPos.mY;
//
//        // FsLogger::LogFormatDefaultDirect("[BSA] Stamp pos=(%.1f,%.1f,%.1f) dist=%.1f radius=%.1f\n", pos[0], pos[1], pos[2], sqrtf(dx*dx + dz*dz), sqrtf(radiusSq));
//
//        if (dx*dx + dz*dz < radiusSq && dy > -hitHalfHeight && dy < hitHalfHeight) {
//			if (currentState == 6) {
//            // FsLogger::LogFormatDefaultDirect("[BSA] Stamp HIT! Setting flag\n");
//			
//			// // Set mode to non-flying (prevents stateThrow from doing wall collision) - crashes if uncommented
//			// *(u16 *)((u8 *)obj + 0xA2E) = 0;  // mode = 0 (not flying)
//			
//			// Set hit flag  
//			u8 *flags = (u8 *)obj + 0xA2D;
//            *flags |= 0x10;
//			}
//            
//        }
//    }
//}

static void damageBlowoutsInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg, int attackerIdx)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;

        if (odx*odx + odz*odz < hitRadiusSq && ody > -hitHalfHeight && ody < hitHalfHeight) {
            float dirDmg = (senderTeamInt == 1) ? -(float)dmg : (float)dmg;
            // Approximate: assume threshold ~1000, so increment = (dmg/1000)*10
            float *fill = (float *)((u8 *)obj + 0x588);
            *fill += (dirDmg / 1000.0f) * 10.0f;
            *(u8 *)((u8 *)obj + 0x658) = 1;  // hit flag
            *(int *)((u8 *)obj + 0x950) = attackerIdx;
        }
    }
}

static void damageSpongesInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg, int burstFrm)
{
    if (burstFrm % 20 != 0) return;
    
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        int spongeTeam = *(int *)((u8 *)obj + 0x5F0);

        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;

        if (odx*odx + odz*odz < hitRadiusSq && ody > -hitHalfHeight && ody < hitHalfHeight) {
            float *fill = (float *)((u8 *)obj + 0x610);

            // Read max fill from params block
            float maxFill = readSpongeMaxFill(obj);

            if (spongeTeam == senderTeamInt) {
                *fill += (float)dmg / 500.0f;
                if (*fill > maxFill) *fill = maxFill;
            } else {
                *fill -= (float)dmg / 500.0f;
                if (*fill < 1.0f) *fill = 1.0f;
            }
            *(u32 *)((u8 *)obj + 0x4F8) |= 2;
        }
    }
}
// ============================================================
// Vtable overrides
// ============================================================

const char *BulletSuperArtillery::vtGetClassName(BulletSuperArtillery *self) {
    return "BulletSuperArtillery";
}

void BulletSuperArtillery::vtFirstCalc(BulletSuperArtillery *self) {
    self->mStateMachine.executeState();
}

void BulletSuperArtillery::vtSecondCalc(BulletSuperArtillery *self) {
    if (!self->mHasBurst || !self->mSender) return;

    float t = (float)self->mBurstFrm / (float)BSA_BURST_DURATION;
    if (t > 1.0f) t = 1.0f;
    const float hitRadiusStart = 30.0f;
    const float hitRadiusEnd   = 140.0f;
    const float hitHalfHeight  = 5000.0f;
    float hitRadius = hitRadiusStart + (hitRadiusEnd - hitRadiusStart) * t;
    float hitRadiusSq = hitRadius * hitRadius;

    int dmg = (int)(BSA_BURST_DMG_START + (BSA_BURST_DMG_END - BSA_BURST_DMG_START) * t);
    if (dmg < (int)BSA_BURST_DMG_END) dmg = (int)BSA_BURST_DMG_END;

    Game::PlayerMgr *pmgr = Game::PlayerMgr::sInstance;
    int senderTeamInt = (int)*(Cmn::Def::Team*)(self->_actorBase + 0x328);

    Game::DamageReason reason;
    reason.mWeaponId = TORNADO_SPECIAL_ID;
    reason.mClassType = 2;
    reason.mHitCountToKill = 0;

    for (int i = 0; i < 10; i++) {
        Game::Player *p = pmgr->getPerformerAt(i);
        if (!p || p == self->mSender || (int)p->mTeam == senderTeamInt) continue;

        float dx = p->mPosition.mX - self->mTo.mX;
        float dz = p->mPosition.mZ - self->mTo.mZ;
        float horizontalDistSq = dx * dx + dz * dz;
        float dy = p->mPosition.mY - self->mTo.mY;

        if (horizontalDistSq < hitRadiusSq && dy > -hitHalfHeight && dy < hitHalfHeight) {
            sead::Vector3<float> hitDir = {0.0f, 0.0f, 0.0f};
            if (p->isInSuperArmor() || p->isInBarrier()) {
                float hDist = sqrtf(horizontalDistSq);
                if (hDist > 0.1f) {
                    hitDir.mX = (dx / hDist) * 2.0f;
                    hitDir.mZ = (dz / hDist) * 2.0f;
                }
            }
            p->receiveDamage_Net(self->mSender->mIndex, (Cmn::Def::DMG)dmg, hitDir, reason, true, false, false);
        }
    }

    // SighterTargets
    auto stIterNode = Game::SighterTarget::getClassIterNodeStatic();
    for (Game::SighterTarget *st = (Game::SighterTarget *)stIterNode->derivedFrontActiveActor();
         st != NULL;
         st = (Game::SighterTarget *)stIterNode->derivedNextActiveActor(st))
    {
        if ((int)st->mTeam == senderTeamInt) continue;
        u32 stState = *(u32 *)((u8 *)st + 0x590);
        if (stState == 3) continue;
        u64 playerDamage = *(u64 *)((u8 *)st + 0x5E8);
        if (!playerDamage) continue;

        float *stPos = (float *)((u8 *)st + 0x39C);
        float sdx = stPos[0] - self->mTo.mX;
        float sdz = stPos[2] - self->mTo.mZ;
        float sdy = stPos[1] - self->mTo.mY;

        if (sdx * sdx + sdz * sdz < hitRadiusSq && sdy > -hitHalfHeight && sdy < hitHalfHeight) {
            Game::PlayerDamage::informDamage((void *)playerDamage, self->mSender->mIndex, (Cmn::Def::DMG)dmg, reason, true);
            *(u32 *)((u8 *)st + 0x4F8) |= 2;
        }
    }
	// Sprinkler — subtract HP at +0x5B4
	damageObjects_SubHP(Game::Sprinkler::getClassIterNodeStatic(),
		senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg, 0x5B4);
	
	// JumpFlag (Beakon) — subtract HP at +0x5C0
	damageObjects_SubHP(Game::JumpFlag::getClassIterNodeStatic(),
		senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg, 0x5C0);
	
	// Shield (Splash Wall) — add accumulated damage at +0x628
	damageObjects_AddDmg(Game::Shield::getClassIterNodeStatic(),
		senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg * 2.5, 0x628);
		
	// Gachihoko (Rainmaker) — team-directional with threshold burst
	damageGachihokoInCylinder(Game::Gachihoko::getClassIterNodeStatic(),
		senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg * 2.5, self->mSender->mIndex);
	
	// SuperBubble (Bubble Blower) - team-directional
	damageBubblesInCylinder(Game::BulletSpSuperBubble::getClassIterNodeStatic(),
		senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg,
		self->mSender->mIndex, Game::MainMgr::sInstance->mPaintGameFrame);
	
	// Brella canopy — hitBarrier (reaction 11), game handles damage internally
	self->eatActorClass(Game::BulletUmbrellaCanopyBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, 11);
	
// commented out because it crashes vvvvv - delete me when you fix it
	//// Blowouts (rotating targets) — float damage
	//damageBlowoutsInCylinder(Game::Blowouts::getClassIterNodeStatic(),
	//	senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg, self->mSender->mIndex);
	
	// SpongeVersus — float fill, team-directional
	damageSpongesInCylinder(Game::SpongeVersus::getClassIterNodeStatic(),
		senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg * 5, self->mBurstFrm);
	
	// Bomb projectiles
    self->eatBombs(hitRadiusSq, hitHalfHeight);
	
	// Seeker
	self->eatActorClass(Game::BulletBombChase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, 6);
	
	// Thrown Splash Wall
	self->eatActorClass(Game::BulletShield::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, 11);
	
//	// Ultra Stamp — set "hit wall" flag, game handles burst on next firstCalc - crashes / ignores ultrastamp
//	self->eatStampThrow(Game::BulletSpSuperStamp::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight);
	
	// deletes all weapon bullets acting as a wall
	self->eatActorClass(Game::BulletShooterBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletBlasterBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletSplashBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletChargerBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletRollerCore::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletRollerSplash::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletSpinnerBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletTwinsBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletSlosherBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletSlosherSplash::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletUmbrellaShotBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
}

void BulletSuperArtillery::vtFourthCalc(BulletSuperArtillery *self) {
    if (self->mFlightActive) {
        self->updateModelMatrix();
    } else if (self->mHasBurst) {
        // paint/timing handled in stateBurst (firstCalc)
        // damage handled in vtSecondCalc
    } else {
        // Only render tank bone tornado if InkstrikeMgr state is cAim or cShootPrepare
        InkstrikeMgr *mgr = InkstrikeMgr::sInstance;
        if (mgr != NULL && self->mSender != NULL) {
            int id = self->mSender->mIndex;
            if (mgr->playerState[id] == TornadoState::cAim || mgr->playerState[id] == TornadoState::cShootPrepare) {
                self->calcTankBone();
            }
        }
    }
}

void BulletSuperArtillery::vtOnActivate(BulletSuperArtillery *self, bool) {
    // Re-set XLink position from sender (in case deferred activation missed the prepare() write)
    if (self->mSender) {
        self->mXLinkMtx = {{
            1.0f, 0.0f, 0.0f, self->mSender->mPosition.mX,
            0.0f, 1.0f, 0.0f, self->mSender->mPosition.mY,
            0.0f, 0.0f, 1.0f, self->mSender->mPosition.mZ
        }};
    }
    // Set XLink local property values
    Lp::Sys::XLink *xlink = self->getXLink();
    if (xlink) {
        // Property 0: "User" (0.0 = Player)
        xlink->setLocalPropertyValue(0, 0.0f);
        // Property 1: "BulletDistance" (set later in launch())
        xlink->setLocalPropertyValue(1, 0.0f);
    }

    // Start in cState_Aim (idle aiming, no xlink effects)
    self->mStateMachine.changeState(cState_Aim);
}

void BulletSuperArtillery::vtOnSleep(BulletSuperArtillery *self) {
    self->reset();
}

int BulletSuperArtillery::vtSetXLinkLocalPropertyDef(BulletSuperArtillery *self, int idx) {
    initUserProperty();
    initDistProperty();
    Lp::Sys::XLinkIUser *xlinkUser = (Lp::Sys::XLinkIUser *)(self->_actorBase + 0x2E8);
    xlinkUser->pushLocalPropertyDefinition((xlink2::PropertyDefinition *)sUserPropBuf);
    xlinkUser->pushLocalPropertyDefinition((xlink2::PropertyDefinition *)sDistPropBuf);
    return idx + 2;
}

int BulletSuperArtillery::vtCountXLinkLocalProperty(BulletSuperArtillery *self) {
    return 2; // "User" + "BulletDistance"
}

// XLink reads position from the matrix returned by getXLinkMtx vtable call.
// By overriding getXLinkMtx we avoid patching XLink internal structures.
static sead::Vector3<float> sBSAUnitScale = {1.0f, 1.0f, 1.0f};

sead::Matrix34<float> *BulletSuperArtillery::vtGetXLinkMtx(BulletSuperArtillery *self) {
    return &self->mXLinkMtx;
}

sead::Vector3<float> *BulletSuperArtillery::vtGetXLinkScale(BulletSuperArtillery *self) {
    return &sBSAUnitScale;
}

// ============================================================
// State machine callbacks
// ============================================================

void BulletSuperArtillery::stateEnterPronounce() {
}

void BulletSuperArtillery::statePronounce() {
    calcFlight();
    int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mStartFrm;
    int flightTime = BSA_FLIGHT_TIME;
	if (elapsed >= 5) {
		// Update xlink root matrix to target position
		mXLinkMtx = {{
			1.0f, 0.0f, 0.0f, mTo.mX,
			0.0f, 1.0f, 0.0f, mTo.mY,
			0.0f, 0.0f, 1.0f, mTo.mZ
		}};
	}
    if (elapsed >= flightTime) {
        mStateMachine.changeState(cState_Burst);
    }
}

void BulletSuperArtillery::stateEnterWait() {
}

void BulletSuperArtillery::stateWait() {
    // During cState_Wait, model is rendered at tank bone via calcTankBone in fourthCalc
    // State transitions to cState_Pronounce when launch() is called
	// Update xlink root matrix to player position for rumble
	mXLinkMtx = {{
		1.0f, 0.0f, 0.0f, mSender->mPosition.mX,
		0.0f, 1.0f, 0.0f, mSender->mPosition.mY,
		0.0f, 0.0f, 1.0f, mSender->mPosition.mZ
	}};
}

void BulletSuperArtillery::stateEnterBurst() {
    doBurst();
}

void BulletSuperArtillery::stateBurst() {
    calcBurstFollow();
}

void BulletSuperArtillery::stateEnterAim() {
}

void BulletSuperArtillery::stateAim() {
}

// ============================================================
// Helpers
// ============================================================

void BulletSuperArtillery::calcTankBone() {
    if (mSender == NULL || mTornadoModel == NULL) return;
    if (!mSender->isAlive()) return;

    Cmn::PlayerCustomPart *tank = mSender->getTank();
    if (tank == NULL) tank = mSender->mPlayerCustomMgr->getMantle();
    if (tank == NULL) return;

    sead::Matrix34<float> tankBoneMtx;
    tank->getRootBoneMtx(&tankBoneMtx);

    // Normalize rotation columns to strip bone scale, keep only position + rotation
    for (int col = 0; col < 3; col++) {
        float len = sqrtf(
            tankBoneMtx.matrix[0][col] * tankBoneMtx.matrix[0][col] +
            tankBoneMtx.matrix[1][col] * tankBoneMtx.matrix[1][col] +
            tankBoneMtx.matrix[2][col] * tankBoneMtx.matrix[2][col]
        );
        if (len > 0.0f) {
            tankBoneMtx.matrix[0][col] /= len;
            tankBoneMtx.matrix[1][col] /= len;
            tankBoneMtx.matrix[2][col] /= len;
        }
    }

    // Apply Z axis offset along the bone's local Z direction
    tankBoneMtx.matrix[0][3] += tankBoneMtx.matrix[0][2] * tornadoTankZOffset;
    tankBoneMtx.matrix[1][3] += tankBoneMtx.matrix[1][2] * tornadoTankZOffset;
    tankBoneMtx.matrix[2][3] += tankBoneMtx.matrix[2][2] * tornadoTankZOffset;

    // Scramble effect during cShootPrepare (InkstrikeMgr state)
    InkstrikeMgr *mgr = InkstrikeMgr::sInstance;
    if (mgr != NULL) {
        int id = mSender->mIndex;
        if (mgr->playerState[id] == TornadoState::cShootPrepare) {
            int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mgr->mShootPrepareFrm[id];
            float intensity = (float)elapsed / 60.0f; // ramps up 0→1 over startflightdelay
            float scramble = sinf(elapsed * 2.5f) * intensity * 1.5f;
            tankBoneMtx.matrix[0][3] += tankBoneMtx.matrix[0][0] * scramble;
            tankBoneMtx.matrix[1][3] += tankBoneMtx.matrix[1][0] * scramble;
            tankBoneMtx.matrix[2][3] += tankBoneMtx.matrix[2][0] * scramble;
        }
    }

    mTornadoModel->mtx = tankBoneMtx;
    mTornadoModel->mUpdateScale |= 1;
    mTornadoModel->updateAnimationWorldMatrix_(3);
    mTornadoModel->requestDraw();
}

void BulletSuperArtillery::calcFlight() {
    int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mStartFrm;
    int flightTime = (mMatchEnding || (mSender != NULL && mSender->isInTrouble_Dying())) ? 150 : BSA_FLIGHT_TIME;
    float anim = float(elapsed) / float(flightTime);
    if (anim > 1.0f) anim = 1.0f;

    sead::Vector3<float> respos;
    respos.mX = mFrom.mX * (1.0f - anim) + mTo.mX * anim;
    respos.mY = mFrom.mY * (1.0f - anim) + mTo.mY * anim + sinf(anim * MATH_PI) * BSA_FLIGHT_HEIGHT;
    respos.mZ = mFrom.mZ * (1.0f - anim) + mTo.mZ * anim;

    // Pitch from velocity direction
    mRot.mX = atan2f(
        sqrtf(powf(abs(respos.mX - mPos.mX), 2) + powf(abs(respos.mZ - mPos.mZ), 2)),
        respos.mY - mPos.mY
    );

    mPos = respos;
}

void BulletSuperArtillery::doBurst() {
    if (mSender == NULL) return;

    // Set BulletDistance = distance from controlled player (listener proxy) to burst pos, capped at 128
    Lp::Sys::XLink *xlink = getXLink();
    if (xlink) {
        Game::Player *listener = Game::PlayerMgr::sInstance->getControlledPerformer();
        if (listener) {
            float dx = listener->mPosition.mX - mPos.mX;
            float dy = listener->mPosition.mY - mPos.mY;
            float dz = listener->mPosition.mZ - mPos.mZ;
            float dist = sqrtf(dx * dx + dy * dy + dz * dz);
            if (dist > 128.0f) dist = 128.0f;
            xlink->setLocalPropertyValue(1, dist);
        }
    }

    // Initialize burst state — BSA handles paint and damage itself
    mBurstRadius = BSA_BURST_RADIUS_START;
    mBurstFrm = 0;

    mFlightActive = false;
    mHasBurst = true;
}
static float s_BSA_BURST_RADIUS_START = BSA_BURST_RADIUS_START;
static float s_BSA_BURST_RADIUS_MAX   = BSA_BURST_RADIUS_MAX;
static float s_BSA_BURST_RADIUS_GROW  = BSA_BURST_RADIUS_GROW;
static float s_BSA_BURST_TEX_ROTATION = BSA_BURST_TEX_ROTATION;
static float s_BSA_BURST_ROT_FRAMES   = BSA_BURST_ROT_FRAMES;
static int   s_BSA_BURST_FRAMES       = BSA_BURST_FRAMES;
static int   s_BSA_BURST_DAMAGE       = BSA_BURST_DAMAGE;
static int   s_BSA_BURST_DURATION     = BSA_BURST_DURATION;

//static void loadBurstConfig() {
//    nn::fs::FileHandle file;
//    nn::Result rc = nn::fs::OpenFile(&file, "sd:/GamblitzDev/blitz_burst.txt", nn::fs::OpenMode_Read);
//    if(rc.IsSuccess()){
//        s64 fileSize = 0;
//        nn::fs::GetFileSize(&fileSize, file);
//        char buf[256] = {};
//        nn::fs::ReadFile(file, 0, buf, fileSize);
//        nn::fs::CloseFile(file);
//        sscanf(buf,
//            "BSA_BURST_RADIUS_START=%f\n"
//            "BSA_BURST_RADIUS_MAX=%f\n"
//            "BSA_BURST_RADIUS_GROW=%f\n"
//            "BSA_BURST_TEX_ROTATION=%f\n"
//            "BSA_BURST_ROT_FRAMES=%f\n"
//            "BSA_BURST_FRAMES=%d\n"
//            "BSA_BURST_DAMAGE=%d\n"
//            "BSA_BURST_DURATION=%d\n",
//            &s_BSA_BURST_RADIUS_START,
//            &s_BSA_BURST_RADIUS_MAX,
//            &s_BSA_BURST_RADIUS_GROW,
//            &s_BSA_BURST_TEX_ROTATION,
//            &s_BSA_BURST_ROT_FRAMES,
//            &s_BSA_BURST_FRAMES,
//            &s_BSA_BURST_DAMAGE,
//            &s_BSA_BURST_DURATION);
//    }
//}

void BulletSuperArtillery::calcBurstFollow() {
	int PlayerIndex = mSender->mIndex;
    mBurstFrm++;
    float increment = (s_BSA_BURST_RADIUS_MAX - s_BSA_BURST_RADIUS_START) / (float)s_BSA_BURST_DURATION;
    float currentRadius = s_BSA_BURST_RADIUS_START + (float)mBurstFrm * increment;
    if(currentRadius > s_BSA_BURST_RADIUS_MAX) currentRadius = s_BSA_BURST_RADIUS_MAX;

    // Paint every x frames
    if(mBurstFrm % s_BSA_BURST_FRAMES == 0){
        // Rotate texture each paint application
        // Each call rotates by x degrees — requestColAndPaint's vel parameter
        // can be used to rotate the texture by passing a rotated direction vector
		float rotAngle = (float)(mBurstFrm / s_BSA_BURST_ROT_FRAMES) * (s_BSA_BURST_TEX_ROTATION * MATH_PI / 180.0f);
        sead::Vector3<float> vel;
        vel.mX = sinf(rotAngle);
        vel.mY = 0.0f;
        vel.mZ = cosf(rotAngle);
		gSpecialWeaponPaint = true;

        sead::Vector2<float> paintSize = {currentRadius, currentRadius};
        Cmn::Def::Team team = *(Cmn::Def::Team*)(_actorBase + 0x328);
        Game::PaintUtl::requestColAndPaint(
            mTo, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosXXSHigh = mTo;
		paintPosXXSHigh.mY += 280.0f; // units up
        Game::PaintUtl::requestColAndPaint(
            paintPosXXSHigh, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosXSHigh = mTo;
		paintPosXSHigh.mY += 240.0f; // units up
        Game::PaintUtl::requestColAndPaint(
            paintPosXSHigh, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosSSHigh = mTo;
		paintPosSSHigh.mY += 200.0f; // units up
        Game::PaintUtl::requestColAndPaint(
            paintPosSSHigh, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosSHigh = mTo;
		paintPosSHigh.mY += 160.0f; // units up
        Game::PaintUtl::requestColAndPaint(
            paintPosSHigh, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosHigh = mTo;
		paintPosHigh.mY += 120.0f; // units up
        Game::PaintUtl::requestColAndPaint(
            paintPosHigh, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosMHigh = mTo;
		paintPosMHigh.mY += 80.0f; // units up
        Game::PaintUtl::requestColAndPaint(
            paintPosMHigh, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosLHigh = mTo;
		paintPosLHigh.mY += 40.0f; // units up
        Game::PaintUtl::requestColAndPaint(
            paintPosLHigh, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosLLow = mTo;
		paintPosLLow.mY -= 40.0f; // units down
        Game::PaintUtl::requestColAndPaint(
            paintPosLLow, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosMLow = mTo;
		paintPosMLow.mY -= 80.0f; // units down
        Game::PaintUtl::requestColAndPaint(
            paintPosMLow, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosLow = mTo;
		paintPosLow.mY -= 120.0f; // units down
        Game::PaintUtl::requestColAndPaint(
            paintPosLow, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosSLow = mTo;
		paintPosSLow.mY -= 160.0f; // units down
        Game::PaintUtl::requestColAndPaint(
            paintPosSLow, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosSSLow = mTo;
		paintPosSSLow.mY -= 200.0f; // units down
        Game::PaintUtl::requestColAndPaint(
            paintPosSSLow, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosXSLow = mTo;
		paintPosXSLow.mY -= 240.0f; // units down
        Game::PaintUtl::requestColAndPaint(
            paintPosXSLow, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);
			
		sead::Vector3<float> paintPosXXSLow = mTo;
		paintPosXXSLow.mY -= 280.0f; // units down
        Game::PaintUtl::requestColAndPaint(
            paintPosXXSLow, paintSize, vel,
            (Game::PaintTexType)11, team,
            sead::Vector3<float>::ey, false, PlayerIndex, 80.0f);

		gSpecialWeaponPaint = false;
    }

    if (mBurstFrm >= s_BSA_BURST_DURATION)
        doSleep();
}

void BulletSuperArtillery::updateModelMatrix() {
    if (!mFlightActive) return;
    if (mTornadoModel == NULL) return;

    mTornadoModel->mtx = {{
        cosf(mRot.mY) * cosf(mRot.mZ),
        sinf(mRot.mX) * sinf(mRot.mY) * cosf(mRot.mZ) - sinf(mRot.mZ) * cosf(mRot.mX),
        cosf(mRot.mX) * sinf(mRot.mY) * cosf(mRot.mZ) + sinf(mRot.mZ) * sinf(mRot.mX),
        mPos.mX,
        sinf(mRot.mZ) * cosf(mRot.mY),
        sinf(mRot.mX) * sinf(mRot.mY) * sinf(mRot.mZ) + cosf(mRot.mZ) * cosf(mRot.mX),
        cosf(mRot.mX) * sinf(mRot.mY) * sinf(mRot.mZ) - cosf(mRot.mZ) * sinf(mRot.mX),
        mPos.mY,
        -sinf(mRot.mY),
        sinf(mRot.mX) * cosf(mRot.mY),
        cosf(mRot.mX) * cosf(mRot.mY),
        mPos.mZ
    }};
    mTornadoModel->mUpdateScale |= 1;
    mTornadoModel->updateAnimationWorldMatrix_(3);
    mTornadoModel->requestDraw();
}

void BulletSuperArtillery::doSleep() {
    mHasBurst = false;
    mFlightActive = false;
    asLpActor()->reserveSleepAll_(Lp::Sys::Actor::ListNodeKind::None);
}

} // namespace Flexlion
