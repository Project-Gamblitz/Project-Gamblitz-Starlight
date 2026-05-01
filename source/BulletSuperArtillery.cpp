#include "flexlion/BulletSuperArtillery.hpp"
#include "flexlion/InkstrikeMgr.hpp"
#include "Game/BulletBombBase.h"
#include "Game/Player/Player.h"
#include "Game/RespawnPoint.h"
#include "Game/Player/PlayerDamage.h"
#include "Game/MainMgr.h"
#include "Game/PaintUtl.h"
#include "Game/DamageReason.h"
#include "Game/SighterTarget.h"
#include "Cmn/KDGndCol/Manager.h"
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
    extern u8 _ZTVN4sead14SafeStringBaseIcEE[];
	extern bool gSpecialWeaponPaint;
	extern int gSpecialWeaponPlayerIdx;
    // Damagable::onDamage(damagable, attackerIdx, team, dmg)
    int _ZN4Game3Cmp9Damagable8onDamageEiN3Cmn3Def4TeamENS3_3DMGE(
        void *damagable, int attackerIdx, int team, int dmg);
    
    // InkRailVersus::emit_(self, team, dmg, soundFlag) — soundFlag=false plays sounds
    void _ZN4Game13InkRailVersus5emit_EN3Cmn3Def4TeamEib(
        void *self, int team, int dmg, bool soundFlag);
    
    // GrindRailVersus::emit_(self, team, currentFrame)
    void _ZN4Game15GrindRailVersus5emit_EN3Cmn3Def4TeamEj(
        void *self, int team, unsigned int currentFrame);
    void _ZN2Lp3Utl12StateMachine11changeStateEj(void *stateMachine, unsigned int newState);
    void _ZN4Game9Gachihoko13informDamage_EiN3Cmn3Def3DMGENS2_4TeamE(
        void *gachihoko, int attackerIdx, unsigned int damage, int team);
    void _ZN4Game12SpongeVersus13informDamage_EN3Cmn3Def3DMGENS2_4TeamEb(
        void *sponge, unsigned int damage, int team, bool forceAlreadyActivated);
	int* _ZN4Game15ScrewLiftOnline14processDamage_EiN3Cmn3Def4TeamENS2_3DMGEibjRKNS_12DamageReasonE(
        void *screw,           // a1
        int attackerIdx,       // a2
        int team,              // a3
        int dmg,               // a4
        int direction,         // a5   (+1 = clockwise, -1 = counter)
        bool overMaxFlag,      // a6   (true if dmg was clamped to 99999)
        unsigned int gameFrame,// a7
        void *damageReason);   // a8   (u16 weaponId + u8 classTypePacked + ...)
    float _ZN4Game8Blowouts6damageEN3Cmn3Def3DMGENS2_4TeamEi(
        void *blowout, int dmg, int team, int attackerIdx);
    void _ZN4Game14BlowoutsOnline6damageEPv(void *furler, void *damageInfo);
	void _ZN4Game14IidaBombOnline6damageEPv(void *bomb, void *damageInfo);
	void _ZN4Game15SwitchWeakPoint6damageEPv(void *weakPoint, void *damageInfo);
	void _ZN4Game18DendenSwitchVersus9onDamage_EN3Cmn3Def3DMGENS2_4TeamEj(
        void *denden, int damage, int team, unsigned int gameFrame);
    void _ZN4Game11SwitchShock9onDamage_EN3Cmn3Def3DMGE(
        void *shock, int damage);
    void _ZN4Game12GeyserVersus6damageEPv(void *geyser, void *damageInfo);
	void _ZN4Game13AirBallOnline6damageEPv(void *airball, void *damageInfo);
	void _ZN4Game19AttractTargetVersus13startAttract_EPNS_6PlayerE(
        void *attractTarget, Game::Player *player);
    void _ZN4Game19RollingBarrelOnline10informHit_EPNS0_8HitInfoE(
        void *barrel, Game::RollingBarrelOnline::HitInfo *h);

    // Cmn::KDGndCol::HitInfo::getPaintBlock_PaintInfo(int blockIdx, u8* outPaintIdx,
    //   sead::Vector3<float>* probePos, sead::Vector3<float>* outLocalPos,
    //   PaintType* outPaintType, Block** outPaintInfo, Block** unused) const
    // Called per paint block returned by checkSphere; outPaintIdx receives the
    // block's ObjPaintIndex byte (0xFE = field, otherwise specific object slot).
    void _ZNK3Cmn8KDGndCol7HitInfo23getPaintBlock_PaintInfoEiPhPN4sead7Vector3IfEES6_PNS1_9PaintTypeEPPNS0_5BlockESB_(
        void *self, int blockIdx, unsigned char *outPaintIdx,
        sead::Vector3<float> *probePos, sead::Vector3<float> *outLocalPos,
        unsigned int *outPaintType, void **outPaintInfo, void *unused);

    // Per-mode paint-block-count table. Indexed as [2 * curModeIdx], where
    // curModeIdx is read from HitInfoImpl + 0x17C after a checkSphere succeeds.
    extern unsigned int _ZN3Cmn8KDGndCol11HitInfoImpl19msaanHitBlock_PaintE[];
	}
#define CmnActorCtor _ZN3Cmn5ActorC2Ev
#define EnumPropDefCtor _ZN6xlink222EnumPropertyDefinitionC2EPKcb
#define EnumPropDefSetEntryBuf _ZN6xlink222EnumPropertyDefinition12setEntryBuf_EiPNS0_5EntryE
#define EnumPropDefEntry _ZN6xlink222EnumPropertyDefinition5entryEiPKc
#define XLinkKillAllSound _ZN2Lp3Sys5XLink12killAllSoundEv
#define DamagableOnDamage   _ZN4Game3Cmp9Damagable8onDamageEiN3Cmn3Def4TeamENS3_3DMGE
#define InkRailEmit         _ZN4Game13InkRailVersus5emit_EN3Cmn3Def4TeamEib
#define GrindRailEmit      _ZN4Game15GrindRailVersus5emit_EN3Cmn3Def4TeamEj
#define StateMachineChangeState _ZN2Lp3Utl12StateMachine11changeStateEj
#define GachihokoInformDamage _ZN4Game9Gachihoko13informDamage_EiN3Cmn3Def3DMGENS2_4TeamE
#define SpongeVersusInformDamage _ZN4Game12SpongeVersus13informDamage_EN3Cmn3Def3DMGENS2_4TeamEb
#define ScrewLiftProcessDamage _ZN4Game15ScrewLiftOnline14processDamage_EiN3Cmn3Def4TeamENS2_3DMGEibjRKNS_12DamageReasonE
#define BlowoutsDamage _ZN4Game8Blowouts6damageEN3Cmn3Def3DMGENS2_4TeamEi
#define BlowoutsOnlineDamage _ZN4Game14BlowoutsOnline6damageEPv
#define IidaBombOnlineDamage _ZN4Game14IidaBombOnline6damageEPv
#define SwitchWeakPointDamage    _ZN4Game15SwitchWeakPoint6damageEPv
#define DendenSwitchVersusOnDamage  _ZN4Game18DendenSwitchVersus9onDamage_EN3Cmn3Def3DMGENS2_4TeamEj
#define SwitchShockOnDamage         _ZN4Game11SwitchShock9onDamage_EN3Cmn3Def3DMGE
#define GeyserVersusDamage _ZN4Game12GeyserVersus6damageEPv
#define AirBallOnlineDamage _ZN4Game13AirBallOnline6damageEPv
#define AttractTargetStartAttract _ZN4Game19AttractTargetVersus13startAttract_EPNS_6PlayerE
#define RollingBarrelInformHit _ZN4Game19RollingBarrelOnline10informHit_EPNS0_8HitInfoE
#define HitInfoGetPaintBlock_PaintInfo _ZNK3Cmn8KDGndCol7HitInfo23getPaintBlock_PaintInfoEiPhPN4sead7Vector3IfEES6_PNS1_9PaintTypeEPPNS0_5BlockESB_
#define HitInfoImplMsaanHitBlock_Paint _ZN3Cmn8KDGndCol11HitInfoImpl19msaanHitBlock_PaintE

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
const float BSA_BURST_HEIGHT       = 280.0f; // cylinder half-height (paint reaches +/- this from mTo.y)
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
	mDidAttractGrab = false;
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
	mFlightTime = BSA_FLIGHT_TIME;

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
    mFlightTime = (forcedByMatchEnd || (mSender != NULL && !mSender->isAlive())) ? 150 : BSA_FLIGHT_TIME;

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
	mFlightTime = BSA_FLIGHT_TIME;
	mDidAttractGrab = false;
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
            
            // Skip Booyah Bomb (BulletSpSuperBall) — should phase through
            typedef const char* (*GetClassNameFunc)(void*);
            const char *className = ((GetClassNameFunc)(*(u64 *)(vtable + 0xB8)))(bomb);
            if (className && strcmp(className, "BulletSpSuperBall") == 0) {
                actor = next;
                continue;
            }
            
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

static void hitInkRailsInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, int senderIdx,
    float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        if (odx*odx + odz*odz >= hitRadiusSq) continue;
        if (ody <= -hitHalfHeight || ody >= hitHalfHeight) continue;

        int state = *(int *)((u8 *)obj + 0x598);
        if (state == 0) {
			// Read rail's max HP param the same way emit_ does (rail+0x4D0 → +16 → +576 → vtable+216)
			int maxHP = 9999;  // safe fallback
			u64 paramRoot = *(u64 *)((u8 *)obj + 0x4D0);
			if (paramRoot) {
				u64 paramBlock = *(u64 *)(paramRoot + 16);
				if (paramBlock) {
					u64 maxHpProvider = paramBlock + 576;
					u64 vtable = *(u64 *)maxHpProvider;
					if (vtable) {
						typedef int* (*GetIntFn)(u64);
						GetIntFn fn = (GetIntFn)(*(u64 *)(vtable + 216));
						int *result = fn(maxHpProvider);
						if (result) maxHP = *result;
					}
				}
			}
			InkRailEmit(obj, senderTeamInt, maxHP, false);
		} else if (state == 1) {
            // Active — heal or drain via game's own logic
            void *damagable = *(void **)((u8 *)obj + 0x768);
            if (damagable) {
                DamagableOnDamage(damagable, senderIdx, senderTeamInt, dmg);
            }
        }
    }
}

static void hitGrindRailsInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, int senderIdx,
    float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        if (odx*odx + odz*odz >= hitRadiusSq) continue;
        if (ody <= -hitHalfHeight || ody >= hitHalfHeight) continue;

        int state = *(int *)((u8 *)obj + 0x598);
        if (state == 0) {
            // Inactive — activate. emit_ sets team, color, HP, drawer state.
            // changeState(1) needed afterward to trigger stateEnterConnect.
            unsigned int gameFrame = (unsigned int)Game::MainMgr::sInstance->mPaintGameFrame;
            GrindRailEmit(obj, senderTeamInt, gameFrame);
            StateMachineChangeState((void *)((u8 *)obj + 0x588), 1u);
        } else if (state == 1) {
            // Active — heal or drain via game's own logic
            void *damagable = *(void **)((u8 *)obj + 0x6F0);
            if (damagable) {
                DamagableOnDamage(damagable, senderIdx, senderTeamInt, dmg);
            }
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
            // Call the game's own informDamage_ — this handles:
            //  1. Team-directional damage (team=1 negates dmg)
            //  2. Accumulation + clamping at ±99999
            //  3. Break-barrier flag at threshold
            //  4. Hit pulse animation (writes amplitude to +0x688, resets phase at +0x68C)
            //     with cooldown gating so pulses don't stack too fast
            GachihokoInformDamage(obj, attackerIdx, (unsigned int)dmg, senderTeamInt);
        }
    }
}

// for Propellers
struct MinimalDamageReason {
    u16 mWeaponId;       // +0x00 — reads via *a8
    u8  mClassTypeByte;  // +0x02 — bits used: low 6 bits sign-extended (<<26 >>26)
    u8  _pad3;           // +0x03
    u32 _pad4;           // +0x04
    // Total 8 bytes should be plenty; function only reads +0 and +2
};

static void damageScrewLiftsInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, int attackerIdx,
    float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg, int gameFrame)
{
    // Build a DamageReason that won't trigger special cooldown paths.
    // mClassType > 2 skips the weapon-kind lookup entirely.
    MinimalDamageReason reason;
    reason.mWeaponId = 0;
    reason.mClassTypeByte = 0x3F;  // -1 after sign extension of low 6 bits -> classType != 1..2
    reason._pad3 = 0;
    reason._pad4 = 0;
    
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        
        if (odx*odx + odz*odz < hitRadiusSq && ody > -hitHalfHeight && ody < hitHalfHeight) {
            // Compute spin direction from hit position relative to screw axis.
            // Screw axis is stored at: *(screw + 0x688) + 0x10 (mat row).
            // But for simplicity we just force +1 direction — the platform will spin
            // one way consistently from Inkstrike. If you want direction-based spin,
            // dot (center - screwPos) with that axis vector.
            int direction = 1;
            
            ScrewLiftProcessDamage(
                obj,
                attackerIdx,
                senderTeamInt,
                dmg,
                direction,
                false,         // overMaxFlag = false unless dmg >= 99999
                (unsigned)gameFrame,
                &reason
            );
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
    sead::Vector3<float> &center, int dmg, int attackerIdx,
    const char *className)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        // Inkfurlers extend outward from the wall-mounted post. The post position
        // at +0x39C is near the wall — often missed by Inkstrike on the floor.
        // The game computes the unroll-hitbox bbox each frame into +0x5F8..+0x60C:
        //   +0x5F8..+0x600 = bbox near corner (post-side)
        //   +0x604..+0x60C = bbox far corner (unroll-tip-side)
        // In state 0 (rest) both corners are ≈ the post position.
        // In states 1/2/3 the far corner is the active hitbox center.
        // We test both corners — if either is in range, we hit.
        u8 *base = (u8 *)obj;
        float *nearPos = (float *)(base + 0x5F8);
        float *farPos  = (float *)(base + 0x604);
        
        bool inRange = false;
        for (int i = 0; i < 2; i++) {
            float *p = (i == 0) ? nearPos : farPos;
            float odx = p[0] - center.mX;
            float odz = p[2] - center.mZ;
            float ody = p[1] - center.mY;
            if (odx*odx + odz*odz >= hitRadiusSq) continue;
            if (ody <= -hitHalfHeight || ody >= hitHalfHeight) continue;
            inRange = true;
            break;
        }
        if (!inRange) continue;
        
        // NOTE: no friendly-skip — friendly damage extends unrolled duration
        u8 dmgInfo[56] = {0};
        *(u32 *)(dmgInfo + 8)  = (u32)attackerIdx;
        *(u32 *)(dmgInfo + 12) = (u32)dmg;
        *(u32 *)(dmgInfo + 48) = (u32)senderTeamInt;
        
        BlowoutsOnlineDamage(obj, dmgInfo);
    }
}

static void damageAirBallsInCylinder(
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
        if (odx*odx + odz*odz >= hitRadiusSq) continue;
        if (ody <= -hitHalfHeight || ody >= hitHalfHeight) continue;

        u8 dmgInfo[56] = {0};
        *(u32 *)(dmgInfo + 8)  = (u32)attackerIdx;
        *(u32 *)(dmgInfo + 12) = (u32)dmg;
        // dmgInfo[+16] = 0 (zero-init) — matches fresh actor's gate field
        *(u32 *)(dmgInfo + 48) = (u32)senderTeamInt;

        AirBallOnlineDamage(obj, dmgInfo);
    }
}

static void attractGrapplersInCylinder(
    bool *didGrabFlag,   // per-BSA latch
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, Game::Player *sender)
{
    if (*didGrabFlag) return;   // already grabbed this BSA lifetime; skip entirely

    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        if (odx*odx + odz*odz >= hitRadiusSq) continue;
        if (ody <= -hitHalfHeight || ody >= hitHalfHeight) continue;

        AttractTargetStartAttract(obj, sender);
        *didGrabFlag = true;     // latch it — no more grabs this BSA
        return;
    }
}

static void hitBarrelsInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int attackerIdx, int dmg)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        if (odx*odx + odz*odz >= hitRadiusSq) continue;
        if (ody <= -hitHalfHeight || ody >= hitHalfHeight) continue;

        // Only hit barrels in waiting state (state 0)
        if (*(int *)((u8 *)obj + 0x5A0) != 0) continue;

        // Launch direction: barrel -> tornado landing, on ground plane
        float dx = pos[0] - center.mX;
        float dz = pos[2] - center.mZ;
        float len2 = dx*dx + dz*dz;
        if (len2 < 0.0001f) continue;   // barrel right under tornado: skip
        float inv = 1.0f / sqrtf(len2);

        // Read the barrel's instance typeId — required for informHit_ to accept
        void *paramPtr = *(void **)((u8 *)obj + 0x7B0);
        if (paramPtr == NULL) continue;   // defensive
        int typeId = *(int *)((u8 *)paramPtr + 52);

        Game::RollingBarrelOnline::HitInfo h;
        h.typeId    = typeId;
        h.playerIdx = attackerIdx;
        h.damage    = dmg;           
        h.pad12     = 0;
        h.flag14    = 3;               // matches what the game writes
        h.flag15    = 0;
        h.dirX      = dx * inv;
        h.dirY      = 0.0f;
        h.dirZ      = dz * inv;
        h.pad28     = 0;

        RollingBarrelInformHit(obj, &h);
    }
}

static void damageDendenSwitchesInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg, unsigned int gameFrame)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        if (odx*odx + odz*odz >= hitRadiusSq) continue;
        if (ody <= -hitHalfHeight || ody >= hitHalfHeight) continue;

        DendenSwitchVersusOnDamage(obj, dmg, senderTeamInt, gameFrame);
    }
}

static void damageSwitchShocksInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        if (odx*odx + odz*odz >= hitRadiusSq) continue;
        if (ody <= -hitHalfHeight || ody >= hitHalfHeight) continue;

        SwitchShockOnDamage(obj, dmg);
    }
}

static void damageSpongesInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg, int burstFrm)
{
    if (burstFrm % 20 != 0) return;  // same rate-limit as before
    
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        
        if (odx*odx + odz*odz < hitRadiusSq && ody > -hitHalfHeight && ody < hitHalfHeight) {
            // Call the game's own informDamage_ — handles fill math, team color/LED change,
            // "Damage"/"Thick" animations, "Max" sound, hit flags automatically
            SpongeVersusInformDamage(obj, (unsigned int)dmg, senderTeamInt, false);
        }
    }
}

static void damageIidaBombsInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg, int attackerIdx)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        // IidaBombOnline inherits Cmn::Actor — position at +0x39C
        int bombTeam = *(int *)((u8 *)obj + 0x328);
        if (bombTeam == senderTeamInt) continue;
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        if (odx*odx + odz*odz >= hitRadiusSq) continue;
        if (ody <= -hitHalfHeight || ody >= hitHalfHeight) continue;
        
        // Skip bombs that are still in Wait/Marking/Fly (not yet damageable)
        // State is at +0x5A8 — Set state = 3 per SP (Wait=0, Marking=1, Fly=2, Set=3).
        u8 dmgInfo[56] = {0};
        *(u32 *)(dmgInfo + 8)  = (u32)attackerIdx;
        *(u32 *)(dmgInfo + 12) = (u32)dmg;
        *(u32 *)(dmgInfo + 48) = (u32)senderTeamInt;
        
        IidaBombOnlineDamage(obj, dmgInfo);
    }
}

static void damageWeakPointsInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt, float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center, int dmg, int attackerIdx)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        // Team gate: only the opposite team can damage.
        int wpTeam = *(int *)((u8 *)obj + 0x328);
        if (wpTeam == senderTeamInt) continue;

        // Position check
        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        if (odx*odx + odz*odz >= hitRadiusSq) continue;
        if (ody <= -hitHalfHeight || ody >= hitHalfHeight) continue;

        u8 dmgInfo[56] = {0};
        *(u32 *)(dmgInfo + 8)  = (u32)attackerIdx;
        *(u32 *)(dmgInfo + 12) = (u32)dmg;
        *(u32 *)(dmgInfo + 48) = (u32)senderTeamInt;

        SwitchWeakPointDamage(obj, dmgInfo);
    }
}

static void damageGeysersInCylinder(
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
        if (odx*odx + odz*odz >= hitRadiusSq) continue;
        if (ody <= -hitHalfHeight || ody >= hitHalfHeight) continue;

        u8 dmgInfo[56] = {0};
        *(u32 *)(dmgInfo + 8)  = (u32)attackerIdx;
        *(u32 *)(dmgInfo + 12) = (u32)dmg;
        *(u32 *)(dmgInfo + 48) = (u32)senderTeamInt;

        GeyserVersusDamage(obj, dmgInfo);
    }
}
static void playBarrierHitEffect(Cmn::Actor* barrier,
                                 const sead::Vector3<float>& hitPos)
{
    // Step 1: fetch xlink (same as game: barrier + 0x320)
    Lp::Sys::XLink* xlink = *reinterpret_cast<Lp::Sys::XLink**>(
        reinterpret_cast<u8*>(barrier) + 0x320);
    if (!xlink) return;

    // Step 2: play "Guard" (block sound) and "Debris" (glass shatter sound)
    xlink2::Handle sndGuardH{};
    xlink->searchAndPlayWrap("Guard", false, &sndGuardH);

    xlink2::Handle sndDebrisH{};
    xlink->searchAndPlayWrap("Debris", false, &sndDebrisH);

    // Step 3: write barrier's own world position into both sound handles so
    // sounds spatialize at the barrier (not at the BSA landing point)
    const float* barrierPos = reinterpret_cast<const float*>(
        reinterpret_cast<u8*>(barrier) + 0x39C);

    for (auto* h : { &sndGuardH, &sndDebrisH }) {
        if (h->mEvent != nullptr) {
            xlink2::Event* ev = h->mEvent;
            if (*reinterpret_cast<u32*>(reinterpret_cast<u8*>(ev) + 32) == h->mEventId) {
                u8* evBytes = reinterpret_cast<u8*>(ev);
                *reinterpret_cast<float*>(evBytes + 188) = barrierPos[0];
                *reinterpret_cast<float*>(evBytes + 192) = barrierPos[1];
                *reinterpret_cast<float*>(evBytes + 196) = barrierPos[2];
                *reinterpret_cast<u16*>(evBytes + 152) |= 0x100;
                *reinterpret_cast<u16*>(evBytes + 154) |= 0x100;
            }
        }
    }
}

static void hitBarriersInCylinder(
    Lp::Sys::ActorClassIterNodeBase *iterNode,
    int senderTeamInt,
    float hitRadiusSq, float hitHalfHeight,
    sead::Vector3<float> &center)
{
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        int barrierTeam = *(int *)((u8 *)obj + 0x328);
        if (barrierTeam == senderTeamInt) continue;

        float *pos = (float *)((u8 *)obj + 0x39C);
        float odx = pos[0] - center.mX;
        float odz = pos[2] - center.mZ;
        float ody = pos[1] - center.mY;
        if (odx*odx + odz*odz >= hitRadiusSq) continue;
        if (ody <= -hitHalfHeight || ody >= hitHalfHeight) continue;

        playBarrierHitEffect(obj, center);
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
		senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg * 1000, 0x5B4);
	
	// JumpFlag (Beakon) — subtract HP at +0x5C0
	damageObjects_SubHP(Game::JumpFlag::getClassIterNodeStatic(),
		senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg * 1000, 0x5C0);
	
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
	
	// BlowoutsOnline
	damageBlowoutsInCylinder(Game::BlowoutsOnline::getClassIterNodeStatic(),
		senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg, self->mSender->mIndex,
		"Online");
	
	// Marina bombs
	damageIidaBombsInCylinder(Game::IidaBombOnline::getClassIterNodeStatic(),
		senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg, self->mSender->mIndex);
	
	// Enm_BossWeakPoint_VS (MP tentacle) — opposite-team only
    damageWeakPointsInCylinder(Game::SwitchWeakPoint::getClassIterNodeStatic(),
        senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg, self->mSender->mIndex);
	
	// SpongeVersus — float fill, team-directional
	damageSpongesInCylinder(Game::SpongeVersus::getClassIterNodeStatic(),
		senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg * 10, self->mBurstFrm);
		
	hitInkRailsInCylinder(Game::InkRailVersus::getClassIterNodeStatic(),
		senderTeamInt, self->mSender->mIndex,
		hitRadiusSq, hitHalfHeight, self->mTo, dmg);

	hitGrindRailsInCylinder(Game::GrindRailVersus::getClassIterNodeStatic(),
		senderTeamInt, self->mSender->mIndex,
		hitRadiusSq, hitHalfHeight, self->mTo, dmg);
	
	// Propellers
	damageScrewLiftsInCylinder(Game::ScrewLiftOnline::getClassIterNodeStatic(),
		senderTeamInt, self->mSender->mIndex, hitRadiusSq, hitHalfHeight,
		self->mTo, dmg, Game::MainMgr::sInstance->mPaintGameFrame);
	
	// DendenSwitchVersus — ink switch, anyone can hit
    damageDendenSwitchesInCylinder(Game::DendenSwitchVersus::getClassIterNodeStatic(),
        senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg * 3,
        Game::MainMgr::sInstance->mPaintGameFrame);

    // SwitchShock — other ink switch variant
    damageSwitchShocksInCylinder(Game::SwitchShock::getClassIterNodeStatic(),
        hitRadiusSq, hitHalfHeight, self->mTo, dmg * 3);
		
	// GeyserVersus - Damages in waiting state only
    damageGeysersInCylinder(Game::GeyserVersus::getClassIterNodeStatic(),
        senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg, self->mSender->mIndex);

	// AirBallOnline — ink grapplers
    damageAirBallsInCylinder(Game::AirBallOnline::getClassIterNodeStatic(),
        senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo, dmg * 10,
        self->mSender->mIndex);
	
	// RollingBarrelOnline - Roloniums
    hitBarrelsInCylinder(
        Game::RollingBarrelOnline::getClassIterNodeStatic(),
        hitRadiusSq, hitHalfHeight, self->mTo,
        self->mSender->mIndex, dmg);
		
	// AttractTargetVersus — MP ink grapplers. Sender grabs all in radius.
    attractGrapplersInCylinder(&self->mDidAttractGrab,
		Game::AttractTargetVersus::getClassIterNodeStatic(),
		hitRadiusSq, hitHalfHeight, self->mTo, self->mSender);
	
	// Bomb projectiles
    self->eatBombs(hitRadiusSq, hitHalfHeight);
	
	// Seeker
	self->eatActorClass(Game::BulletBombChase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, 6);
	
	// Thrown Splash Wall
	self->eatActorClass(Game::BulletShield::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, 11);
	
//	// Ultra Stamp — set "hit wall" flag, game handles burst on next firstCalc - crashes / ignores ultrastamp
//	self->eatStampThrow(Game::BulletSpSuperStamp::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight);

	// Respawn barrier - for sounds and effects
	hitBarriersInCylinder(
    Game::RespawnPoint::getClassIterNodeStatic(),
		senderTeamInt, hitRadiusSq, hitHalfHeight, self->mTo);
	
	// deletes all weapon bullets acting as a wall
	self->eatActorClass(Game::BulletShooterBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletBlasterBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletSplashBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletChargerBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletRollerCore::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletRollerSplash::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletSpinnerBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletTwinsBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletSlosherBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, 6);
	self->eatActorClass(Game::BulletSlosherSplash::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletUmbrellaShotBase::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletSpJetpackLauncher::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
	self->eatActorClass(Game::BulletSpSuperMissile::getClassIterNodeStatic(), hitRadiusSq, hitHalfHeight, -1);
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
	if (elapsed >= 5) {
		// Update xlink root matrix to target position
		mXLinkMtx = {{
			1.0f, 0.0f, 0.0f, mTo.mX,
			0.0f, 1.0f, 0.0f, mTo.mY,
			0.0f, 0.0f, 1.0f, mTo.mZ
		}};
	}
    if (elapsed >= mFlightTime) {
        mStateMachine.changeState(cState_Burst);
    }
}

void BulletSuperArtillery::stateEnterWait() {
	// Update xlink root matrix to tank position for rumble
    if (mTornadoModel != NULL) {
        mXLinkMtx = mTornadoModel->mtx;
    }
}

void BulletSuperArtillery::stateWait() {
    // During cState_Wait, model is rendered at tank bone via calcTankBone in fourthCalc
    // State transitions to cState_Pronounce when launch() is called
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
    float anim = float(elapsed) / float(mFlightTime);
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
        float rotAngle = (float)(mBurstFrm / s_BSA_BURST_ROT_FRAMES) * (s_BSA_BURST_TEX_ROTATION * MATH_PI / 180.0f);
        gSpecialWeaponPaint = true;
        gSpecialWeaponPlayerIdx = PlayerIndex;

        Cmn::Def::Team team = *(Cmn::Def::Team*)(_actorBase + 0x328);

        // Volumetric cylinder paint via requestIndependentHeightRangePaint.
        // paintIdx = 0xFE → field block (world-frame, no per-object transform).
        // The hook on PaintDrawCommandMgr::requestPaint (552.slpatch FC26AC)
        // intercepts the dispatch and overwrites the RequestPaintArg's
        // playerIdx/cmd fields so the bursting player gets credit without
        // refilling their special gauge.
        sead::Vector2<float> heightRange;
        heightRange.mX = -0.5f - BSA_BURST_HEIGHT;
        heightRange.mY = BSA_BURST_HEIGHT;

        sead::Vector2<float> extent;
        extent.mX = currentRadius;
        extent.mY = currentRadius;

        sead::Vector2<float> paintDir;
        paintDir.mX = sinf(rotAngle);
        paintDir.mY = cosf(rotAngle);

        u32 paintIdxData = 0xFE;
        Game::ObjPaintIndex paintIdx;
        paintIdx.mIndex = &paintIdxData;

        // Pass 1024 as the frame value. This matches exactly what the engine
        // returns from Game::MainMgr::getPaintGameFrame() during normal
        // gameplay — internally, a flag on CloneObjMgr+0x400 makes that
        // getter return the sentinel constant 1024 instead of the real
        // mPaintGameFrame counter. ALL vanilla paint (every shooter, every
        // other special, every cylinder/area paint) uses 1024 as its frame,
        // which yields cmd+0x20 (RainbowInkFrame) = (playerIdx + 10*1024)/16
        // ≈ 640 — the engine's "baseline paint priority".
        //
        // We previously passed mPaintGameFrame directly (bypassing the
        // sentinel), so our paint was either at RIF=0 (frame=0: too low to
        // override anything, and below the paint baseline) or at RIF >
        // baseline (frame=current: above the baseline, which trips the
        // "elevated paint = lock" gate at scene-time >15s).
        //
        // Passing 1024 puts us exactly on the engine's intended baseline:
        // same priority as vanilla → override works on ties (newer command
        // wins, that's us), AND we never exceed the baseline → no lock.
        unsigned int frame = 1024;

        Game::PaintUtl::requestIndependentHeightRangePaint(
            frame, heightRange, mTo, sead::Vector3<float>::ey,
            extent, paintDir, team, paintIdx,
            (Game::PaintTexType)11, (Cmn::KDGndCol::HitInfo::PaintType)1);

        // Object paint — mimic requestColAndPaint's per-block transform.
        // Sphere-check to enumerate paint blocks the burst could touch, then
        // for each unique paintIdx, read the block's paintInfo (basis matrix
        // at +0x0C..+0x2C, origin at +0x30..+0x38) and compute the world→
        // block-local transform exactly the way requestColAndPaint does
        // internally:
        //
        //   delta     = mTo - blockOrigin
        //   localPos  = basis · delta
        //
        // We pass localPos as the cylinder's position. The vtable34Hook
        // (552.slpatch FC2500) skips the per-object pos transform inside
        // requestIndependentHeightRangePaint so our pre-computed local
        // value isn't re-transformed. This gives correct paint placement
        // per block and naturally excludes far objects (their localPos
        // ends up far outside their UV bounds → renderer paints nothing).
        Cmn::KDGndCol::CheckIF colCheck(NULL);
        sead::Vector3<float> probeWorld = mTo;
        probeWorld.mY += 1.0f;
        float sphereRad = (BSA_BURST_HEIGHT > currentRadius) ? BSA_BURST_HEIGHT : currentRadius;
        // Wide kindFloor mask (same as Utils::calcGroundPos's raycast): include
        // every floor kind, only clear bits 8 (skip main field) and 9 (skip
        // objects) so both are iterated. Catches stage-specific block kinds
        // like Mahi-Mahi's water-platform surfaces that the narrower
        // requestColAndPaint mode (0x4040A) doesn't include.
        unsigned int sweepMode = 0xFFFFFCFF;
        bool hit = colCheck.checkSphere(probeWorld, sphereRad, sweepMode, 0xFFFFFFFF,
                                        Cmn::KDGndCol::Manager::cWallNrmY_L);
        if (hit && colCheck.mHitInfoImpl != NULL) {
            bool seen[256] = {false};
            seen[0xFE] = true;  // field already dispatched above
            seen[0xFF] = true;  // sentinel — never paint

            // ObjPaintMgr's array of ObjPaint pointers — indexed by paintIdx.
            // Layout:
            //   PaintMgr::sInstance + 0xE9B8 = embedded ObjPaintMgr
            //   ObjPaintMgr + 0x18           = paint-object count (u32)
            //   ObjPaintMgr + 0x20           = void** array (one entry per paintIdx)
            // We read this once per paint frame and use it to look up an
            // ObjPaint instance by paintIdx so we can inspect its vtable.
            char* paintMgr = (char*)Game::PaintMgr::sInstance;
            char* objPaintMgr = paintMgr + 0xE9B8;
            void** objPaintArray = *(void***)(objPaintMgr + 0x20);
            unsigned int objPaintCount = *(unsigned int*)(objPaintMgr + 0x18);

            unsigned int curModeIdx = *(unsigned int*)((unsigned char*)colCheck.mHitInfoImpl + 0x17C);
            unsigned int numBlocks  = HitInfoImplMsaanHitBlock_Paint[2 * curModeIdx];

            for (unsigned int i = 0; i < numBlocks; ++i) {
                unsigned char paintIdxByte = 0xFE;
                sead::Vector3<float> probeIO  = probeWorld;
                sead::Vector3<float> localOut = {0.0f, 0.0f, 0.0f};
                unsigned int paintTypeOut = 0;
                void *paintInfoOut = NULL;

                HitInfoGetPaintBlock_PaintInfo(
                    colCheck.mHitInfoImpl, (int)i, &paintIdxByte,
                    &probeIO, &localOut, &paintTypeOut, &paintInfoOut, NULL);

                if (seen[paintIdxByte]) continue;
                if (paintInfoOut == NULL) continue;
                seen[paintIdxByte] = true;

                // Floor-tied detection: ObjPaintFloor and ObjPaintYPlusFloor
                // override draw() to tail-call FloorPainter::drawToFloor when
                // faceType==4 (floor face). That writes paint to the FIELD's
                // RT instead of (or in addition to) the object's own RT, so
                // dispatching cylinder paint to these objects on top of our
                // explicit field dispatch above produces visible double-paint
                // — what the user described as "3-4 Inkstrikes landed at the
                // same time". Skat-park / Tuzura have several such objects.
                //
                // We can't link against ObjPaintFloor's vtable in 5.5.2 (no
                // symbols) so we detect it at runtime by signature:
                //   vtable[13]() = `draw(this, drawCtx, paintCmd, paintType, drawCache)`
                // Both ObjPaintFloor::draw and ObjPaintYPlusFloor::draw start
                // with the same 8-byte prologue:
                //   LDR W8, [X2, #0x54]   = 0xB9405448  (load cmd+0x54 = faceType)
                //   CMP W8, #4            = 0x7100111F
                // No other ObjPaint subclass starts with this — they all have
                // proper stack-frame prologues. So this signature uniquely
                // identifies the floor-drawing-tied subclasses.
                if (paintIdxByte < objPaintCount && objPaintArray != NULL) {
                    void* objPaint = objPaintArray[paintIdxByte];
                    if (objPaint != NULL) {
                        void** vtable = *(void***)objPaint;
                        if (vtable != NULL) {
                            void* drawFn = vtable[13];  // 0x68/8 = 13 (ObjPaintBase::draw slot)
                            if (drawFn != NULL) {
                                unsigned int* instr = (unsigned int*)drawFn;
                                if (instr[0] == 0xB9405448u &&
                                    instr[1] == 0x7100111Fu) {
                                    // Floor-tied object — paint goes to field RT
                                    // anyway via FloorPainter::drawToFloor. Skip
                                    // our dispatch to avoid double-paint.
                                    continue;
                                }
                            }
                        }
                    }
                }

                // Read block transform from paintInfo (same offsets requestColAndPaint uses)
                float* pi = (float*)paintInfoOut;
                float b00 = pi[3],  b01 = pi[4],  b02 = pi[5];   // basis row 0  (off 0x0C/0x10/0x14)
                float b10 = pi[6],  b11 = pi[7],  b12 = pi[8];   // basis row 1  (off 0x18/0x1C/0x20)
                float b20 = pi[9],  b21 = pi[10], b22 = pi[11];  // basis row 2  (off 0x24/0x28/0x2C)
                float ox  = pi[12], oy  = pi[13], oz  = pi[14];  // block origin (off 0x30/0x34/0x38)

                float dx = mTo.mX - ox;
                float dy = mTo.mY - oy;
                float dz = mTo.mZ - oz;
                sead::Vector3<float> localPos;
                localPos.mX = dx * b00 + dy * b01 + dz * b02;
                localPos.mY = dx * b10 + dy * b11 + dz * b12;
                localPos.mZ = dx * b20 + dy * b21 + dz * b22;

                u32 objIdxData = paintIdxByte;
                Game::ObjPaintIndex objPaintIdx;
                objPaintIdx.mIndex = &objIdxData;
                Game::PaintUtl::requestIndependentHeightRangePaint(
                    frame, heightRange, localPos, sead::Vector3<float>::ey,
                    extent, paintDir, team, objPaintIdx,
                    (Game::PaintTexType)11,
                    (Cmn::KDGndCol::HitInfo::PaintType)1);
            }
        }

        gSpecialWeaponPaint = false;
    }

    if (mBurstFrm >= s_BSA_BURST_DURATION) {
        doSleep();
	}
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
	mDidAttractGrab = false;
    asLpActor()->reserveSleepAll_(Lp::Sys::Actor::ListNodeKind::None);
}

} // namespace Flexlion
