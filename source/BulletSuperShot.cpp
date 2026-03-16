#include "flexlion/BulletSuperShot.hpp"
#include "Cmn/KDGndCol/Manager.h"
#include "Game/Player/Player.h"
#include "Game/PlayerMgr.h"

extern "C" {
    void _ZN3Cmn5ActorC2Ev(void *);
    void _ZN6xlink222EnumPropertyDefinitionC2EPKcb(void *, const char *, bool);
    void _ZN6xlink222EnumPropertyDefinition12setEntryBuf_EiPNS0_5EntryE(void *, int, void *);
    void _ZN6xlink222EnumPropertyDefinition5entryEiPKc(void *, int, const char *);
    void _ZN2Lp3Sys5XLink12killAllSoundEv(void *xlink);
    void _ZN2Lp3Sys5XLink13killAllEffectEv(void *xlink);
}
#define CmnActorCtor _ZN3Cmn5ActorC2Ev
#define EnumPropDefCtor _ZN6xlink222EnumPropertyDefinitionC2EPKcb
#define EnumPropDefSetEntryBuf _ZN6xlink222EnumPropertyDefinition12setEntryBuf_EiPNS0_5EntryE
#define EnumPropDefEntry _ZN6xlink222EnumPropertyDefinition5entryEiPKc
#define XLinkKillAllSound _ZN2Lp3Sys5XLink12killAllSoundEv
#define XLinkKillAllEffect _ZN2Lp3Sys5XLink13killAllEffectEv

namespace Game {

// ============================================================
// Vtable
// ============================================================

static bool sInitVtable = false;
static Cmn::ActorVtable sBSSVtable;

void initBSSVtable_internal(BulletSuperShot *bss) {
    Cmn::ActorVtable **vtable = (Cmn::ActorVtable **)bss;
    if (!sInitVtable) {
        memcpy(&sBSSVtable, *vtable, sizeof(Cmn::ActorVtable));
        sBSSVtable.getClassName = (u64)((const char *(*)(BulletSuperShot *))&BulletSuperShot::vtGetClassName);
        sBSSVtable.firstCalc = (u64)((void (*)(BulletSuperShot *))&BulletSuperShot::vtFirstCalc);
        sBSSVtable.onActivate = (u64)((void (*)(BulletSuperShot *, bool))&BulletSuperShot::vtOnActivate);
        sBSSVtable.onSleep = (u64)((void (*)(BulletSuperShot *))&BulletSuperShot::vtOnSleep);
        sBSSVtable.getXlinkName = (u64)((const char *(*)(BulletSuperShot *))&BulletSuperShot::vtGetXlinkName);
        sBSSVtable.getXLinkMtx = (u64)((sead::Matrix34<float> *(*)(BulletSuperShot *))&BulletSuperShot::vtGetXLinkMtx);
        sBSSVtable.getXLinkScale = (u64)((sead::Vector3<float> *(*)(BulletSuperShot *))&BulletSuperShot::vtGetXLinkScale);
        sBSSVtable.isCreateSlink = (u64)((bool (*)(BulletSuperShot *))&BulletSuperShot::vtIsCreateSlink);
        sBSSVtable.setXlinkLocalPropertyDefinition = (u64)((int (*)(BulletSuperShot *, int))&BulletSuperShot::vtSetXLinkLocalPropertyDef);
        sBSSVtable.countXlinkLocalProperty = (u64)((int (*)(BulletSuperShot *))&BulletSuperShot::vtCountXLinkLocalProperty);
        sInitVtable = true;
    }
    *vtable = &sBSSVtable;
}

// ============================================================
// XLink properties
// ============================================================

struct EnumEntry {
    u64 name_ptr;
    u32 value;
    u32 _pad;
};

// "BulletDamageType" enum property (elink index 0)
static _BYTE sDmgTypePropBuf[0x78];
static EnumEntry sDmgTypeEntries[5];
static bool sDmgTypePropInit = false;

static void initDmgTypeProperty() {
    if (sDmgTypePropInit) return;
    EnumPropDefCtor(sDmgTypePropBuf, "BulletDamageType", false);
    memset(sDmgTypeEntries, 0, sizeof(sDmgTypeEntries));
    for (int i = 0; i < 5; i++) sDmgTypeEntries[i].value = (u32)-1;
    EnumPropDefSetEntryBuf(sDmgTypePropBuf, 5, sDmgTypeEntries);
    EnumPropDefEntry(sDmgTypePropBuf, 0, "DamageE");
    EnumPropDefEntry(sDmgTypePropBuf, 1, "DamageP");
    EnumPropDefEntry(sDmgTypePropBuf, 2, "Field");
    EnumPropDefEntry(sDmgTypePropBuf, 3, "NoDamage");
    EnumPropDefEntry(sDmgTypePropBuf, 4, "Shield");
    sDmgTypePropInit = true;
}

// "BulletPaintType" enum property (elink index 1)
static _BYTE sPaintTypePropBuf[0x78];
static EnumEntry sPaintTypeEntries[2];
static bool sPaintTypePropInit = false;

static void initPaintTypeProperty() {
    if (sPaintTypePropInit) return;
    EnumPropDefCtor(sPaintTypePropBuf, "BulletPaintType", false);
    memset(sPaintTypeEntries, 0, sizeof(sPaintTypeEntries));
    sPaintTypeEntries[0].value = (u32)-1;
    sPaintTypeEntries[1].value = (u32)-1;
    EnumPropDefSetEntryBuf(sPaintTypePropBuf, 2, sPaintTypeEntries);
    EnumPropDefEntry(sPaintTypePropBuf, 0, "NoPaint");
    EnumPropDefEntry(sPaintTypePropBuf, 1, "Paint");
    sPaintTypePropInit = true;
}

// "GndMaterial" enum property (elink index 2)
static _BYTE sGndMatPropBuf[0x78];
static EnumEntry sGndMatEntries[8];
static bool sGndMatPropInit = false;

static void initGndMatProperty() {
    if (sGndMatPropInit) return;
    EnumPropDefCtor(sGndMatPropBuf, "GndMaterial", false);
    memset(sGndMatEntries, 0, sizeof(sGndMatEntries));
    for (int i = 0; i < 8; i++) sGndMatEntries[i].value = (u32)-1;
    EnumPropDefSetEntryBuf(sGndMatPropBuf, 8, sGndMatEntries);
    EnumPropDefEntry(sGndMatPropBuf, 0, "ExFallWater");
    EnumPropDefEntry(sGndMatPropBuf, 1, "ExGrate");
    EnumPropDefEntry(sGndMatPropBuf, 2, "ExObj");
    EnumPropDefEntry(sGndMatPropBuf, 3, "ExSponge");
    EnumPropDefEntry(sGndMatPropBuf, 4, "Field");
    EnumPropDefEntry(sGndMatPropBuf, 5, "NoGnd");
    EnumPropDefEntry(sGndMatPropBuf, 6, "PaintedEnemy");
    EnumPropDefEntry(sGndMatPropBuf, 7, "PaintedFriend");
    sGndMatPropInit = true;
}

// ============================================================
// Constructor
// ============================================================

BulletSuperShot::BulletSuperShot() {
    CmnActorCtor(this);
    initBSSVtable_internal(this);

    mStateMachine.initialize(cState_Count, 0, NULL);
    setStateMachineInnerPtr(&mStateMachine);

    mStateMachine.registStateName(cState_Flight, sead::SafeStringBase<char>("cState_Flight"));
    mStateMachine.mStateBuffer[cState_Flight] = Lp::Utl::StateMachine::Delegate<BulletSuperShot>(
        this, &BulletSuperShot::stateEnterFlight, &BulletSuperShot::stateFlight, NULL);

    mStateMachine.registStateName(cState_Idle, sead::SafeStringBase<char>("cState_Idle"));
    mStateMachine.mStateBuffer[cState_Idle] = Lp::Utl::StateMachine::Delegate<BulletSuperShot>(
        this, &BulletSuperShot::stateEnterIdle, &BulletSuperShot::stateIdle, NULL);

    mSender = NULL;
    mPos = sead::Vector3<float>::zero;
    mVel = sead::Vector3<float>::zero;
    mStartPos = sead::Vector3<float>::zero;
    mXLinkMtx = sead::Matrix34<float>::ident;
    mFrame = 0;
    mActive = false;
    mSystemActive = false;
    mBulletEvent = 0;
    mBulletEventId = 0;
}

// ============================================================
// Factory
// ============================================================

BulletSuperShot *BulletSuperShot::create(Lp::Sys::Actor *parent, Cmn::Def::Team team) {
    Lp::Sys::ActorSystem *system = Lp::Sys::ActorSystem::sInstance;
    system->criticalSession->lock();

    BulletSuperShot *bss = new BulletSuperShot();
    bss->asLpActor()->mHeap = sead::HeapMgr::sInstance->getCurrentHeap();
    bss->setTeam(team);
    bss->asLpActor()->actorSysOnCreate(parent);

    system->criticalSession->unlock();
    return bss;
}

// ============================================================
// Public API
// ============================================================

void BulletSuperShot::launch(Game::Player *sender, sead::Vector3<float> const &pos, sead::Vector3<float> const &vel) {
    mSender = sender;
    mPos = pos;
    mVel = vel;
    mStartPos = pos;
    mFrame = 0;
    mActive = true;
    setTeam(sender->mTeam);

    mXLinkMtx = sead::Matrix34<float>::ident;
    mXLinkMtx.matrix[0][3] = mPos.mX;
    mXLinkMtx.matrix[1][3] = mPos.mY;
    mXLinkMtx.matrix[2][3] = mPos.mZ;

    // Actor should already be active (pre-activated when player enters special).
    // Just switch to flight state.
    if (mSystemActive) {
        Lp::Sys::XLink *xlink = getXLink();
        if (xlink) {
            xlink->setLocalPropertyValue(0, 1.0f); // BulletDamageType = DamageP
            xlink->setLocalPropertyValue(1, 1.0f); // BulletPaintType = Paint
            xlink->setLocalPropertyValue(2, 5.0f); // GndMaterial = NoGnd
        }
        mStateMachine.changeState(cState_Flight);
    } else {
        // Fallback: activate now if not pre-activated
        asLpActor()->reserveActivate(true);
    }
}

void BulletSuperShot::doSleep() {
    mSender = NULL;
    mActive = false;
    mFrame = 0;
    mPos = sead::Vector3<float>::zero;
    mVel = sead::Vector3<float>::zero;

    // Synchronously kill all xlink effects/sounds BEFORE the deferred sleep.
    // reserveSleepAll_ is deferred — if the scene heap is freed before it processes,
    // the xlink worker thread would access freed memory (our matrix, vtable, etc).
    // Killing effects synchronously ensures the worker has nothing to process.
    Lp::Sys::XLink *xlink = getXLink();
    if (xlink) {
        XLinkKillAllEffect(xlink);
        XLinkKillAllSound(xlink);
    }

    asLpActor()->reserveSleepAll_(Lp::Sys::Actor::ListNodeKind::None);
}

// ============================================================
// State machine callbacks
// ============================================================

void BulletSuperShot::stateEnterFlight() {
}

void BulletSuperShot::stateFlight() {
    // Emit flight effects on first frame (deferred so xlink is fully initialized)
    if (mFrame == 0) {
        Lp::Sys::XLink *xlink = getXLink();
        if (xlink) {
            xlink2::Handle h;
            xlink->searchAndEmitWrap("Bullet", false, &h);
            mBulletEvent = *(u64 *)&h;
            mBulletEventId = *(u32 *)((u8 *)&h + 8);
            xlink->searchAndEmitWrap("BulletCore", false, &h);
        }
    }

    // Apply gravity
    mVel.mY -= GRAVITY;

    // Next position
    sead::Vector3<float> nextPos;
    nextPos.mX = mPos.mX + mVel.mX;
    nextPos.mY = mPos.mY + mVel.mY;
    nextPos.mZ = mPos.mZ + mVel.mZ;

    // Ground collision via downward raycast
    bool hitGround = false;
    if (mVel.mY < 0.0f) {
        Cmn::KDGndCol::CheckIF colCheck((Cmn::Actor *)this);
        sead::Vector3<float> sweepStart = nextPos;
        sweepStart.mY += 20.0f;
        sead::Vector3<float> downDir;
        downDir.mX = 0.0f;
        downDir.mY = -1.0f;
        downDir.mZ = 0.0f;
        colCheck.checkMoveSphere(
            sweepStart, downDir, 40.0f, 6.0f,
            0xFFFFFEFF, 0xFFFFFFFF,
            Cmn::KDGndCol::Manager::cWallNrmY_L, 1.0f);
        if (colCheck.mResultFlags != 0) {
            hitGround = true;
        }
        // Fallback: sleep if bullet fell far below start position
        if (!hitGround && nextPos.mY < mStartPos.mY - 500.0f) {
            hitGround = true;
        }
    }

    if (hitGround) {
        mPos = nextPos;
        mXLinkMtx.matrix[0][3] = mPos.mX;
        mXLinkMtx.matrix[1][3] = mPos.mY;
        mXLinkMtx.matrix[2][3] = mPos.mZ;

        Lp::Sys::XLink *xlink = getXLink();
        if (xlink) {
            xlink2::Handle h;
            xlink->searchAndEmitWrap("FloorSplash", false, &h);
            xlink->searchAndEmitWrap("HitSplash", false, &h);
        }

        doSleep();
        return;
    }

    // Update position
    mPos = nextPos;
    mFrame++;

    // Update xlink matrix to follow bullet
    mXLinkMtx.matrix[0][3] = mPos.mX;
    mXLinkMtx.matrix[1][3] = mPos.mY;
    mXLinkMtx.matrix[2][3] = mPos.mZ;

    // Maximum flight time
    if (mFrame >= MAX_FLIGHT_FRAMES) {
        doSleep();
    }
}

void BulletSuperShot::stateEnterIdle() {
}

void BulletSuperShot::stateIdle() {
}

// ============================================================
// Vtable overrides
// ============================================================

const char *BulletSuperShot::vtGetClassName(BulletSuperShot *self) {
    return "BulletSuperShot";
}

void BulletSuperShot::vtFirstCalc(BulletSuperShot *self) {
    self->mStateMachine.executeState();
}

void BulletSuperShot::vtOnActivate(BulletSuperShot *self, bool) {
    self->mSystemActive = true;

    Lp::Sys::XLink *xlink = self->getXLink();
    if (xlink) {
        xlink->setLocalPropertyValue(0, 1.0f); // BulletDamageType = DamageP
        xlink->setLocalPropertyValue(1, 1.0f); // BulletPaintType = Paint
        xlink->setLocalPropertyValue(2, 5.0f); // GndMaterial = NoGnd
    }

    // Start in Idle — launch() will switch to Flight when the player fires
    self->mStateMachine.changeState(cState_Idle);
}

void BulletSuperShot::vtOnSleep(BulletSuperShot *self) {
    self->mSystemActive = false;
    self->mSender = NULL;
    self->mActive = false;
    self->mFrame = 0;
}

sead::Matrix34<float> *BulletSuperShot::vtGetXLinkMtx(BulletSuperShot *self) {
    return &self->mXLinkMtx;
}

static sead::Vector3<float> sUnitScale = {1.0f, 1.0f, 1.0f};

sead::Vector3<float> *BulletSuperShot::vtGetXLinkScale(BulletSuperShot *self) {
    return &sUnitScale;
}

bool BulletSuperShot::vtIsCreateSlink(BulletSuperShot *self) {
    return false;
}

const char *BulletSuperShot::vtGetXlinkName(BulletSuperShot *self) {
    return "BulletSuperShot";
}

int BulletSuperShot::vtSetXLinkLocalPropertyDef(BulletSuperShot *self, int baseCount) {
    initDmgTypeProperty();
    initPaintTypeProperty();
    initGndMatProperty();
    Lp::Sys::XLinkIUser *xlinkUser = (Lp::Sys::XLinkIUser *)(self->_actorBase + 0x2E8);
    xlinkUser->pushLocalPropertyDefinition((xlink2::PropertyDefinition *)sDmgTypePropBuf);
    xlinkUser->pushLocalPropertyDefinition((xlink2::PropertyDefinition *)sPaintTypePropBuf);
    xlinkUser->pushLocalPropertyDefinition((xlink2::PropertyDefinition *)sGndMatPropBuf);
    return baseCount + 3;
}

int BulletSuperShot::vtCountXLinkLocalProperty(BulletSuperShot *self) {
    return 3;
}

} // namespace Game

void initBSSVtable(Game::BulletSuperShot *bss) {
    Game::initBSSVtable_internal(bss);
}
