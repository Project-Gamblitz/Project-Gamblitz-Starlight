#include "flexlion/BulletSuperShot.hpp"
#include "Game/Player/Player.h"
#include "Game/PlayerMgr.h"

extern "C" {
    void _ZN3Cmn5ActorC2Ev(void *);
    void _ZN6xlink222EnumPropertyDefinitionC2EPKcb(void *, const char *, bool);
    void _ZN6xlink222EnumPropertyDefinition12setEntryBuf_EiPNS0_5EntryE(void *, int, void *);
    void _ZN6xlink222EnumPropertyDefinition5entryEiPKc(void *, int, const char *);
    void _ZN2Lp3Sys5XLink12killAllSoundEv(void *xlink);
}
#define CmnActorCtor _ZN3Cmn5ActorC2Ev
#define EnumPropDefCtor _ZN6xlink222EnumPropertyDefinitionC2EPKcb
#define EnumPropDefSetEntryBuf _ZN6xlink222EnumPropertyDefinition12setEntryBuf_EiPNS0_5EntryE
#define EnumPropDefEntry _ZN6xlink222EnumPropertyDefinition5entryEiPKc
#define XLinkKillAllSound _ZN2Lp3Sys5XLink12killAllSoundEv

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
// Entries: DamageE, DamageP, Field, NoDamage, Shield
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
// Entries: NoPaint, Paint
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
// Entries matching the elink resource
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
// XLink root matrix patching (same as BSA)
// ============================================================

static void patchUserInstanceRootMtx(u8 *userInst, sead::Matrix34<float> *newMtx) {
    if (!userInst) return;
    sead::Matrix34<float> **rootMtxPtr = (sead::Matrix34<float> **)(userInst + 64);
    sead::Matrix34<float> *oldMtx = *rootMtxPtr;
    if (oldMtx == newMtx) return;

    u8 flag = *(userInst + 208) & 1;
    u8 *connStruct = *(u8 **)(userInst + 32 + flag * 8);
    if (connStruct && *(connStruct + 32)) {
        int count = *(int *)connStruct;
        u8 *connArray = *(u8 **)(connStruct + 8);
        for (int i = 0; i < count; i++) {
            u8 *conn = connArray + 24 * i;
            if (*(sead::Matrix34<float> **)(conn + 8) == oldMtx) {
                *(sead::Matrix34<float> **)(conn + 8) = newMtx;
                *(conn + 16) = 0;
            }
        }
    }
    *rootMtxPtr = newMtx;
    *(userInst + 72) = 0;
}

void BulletSuperShot::setXLinkRootMtx() {
    Lp::Sys::XLink *xlink = getXLink();
    if (!xlink) return;
    u8 *xlinkBytes = (u8 *)xlink;
    patchUserInstanceRootMtx(*(u8 **)(xlinkBytes + 8), &mXLinkMtx);
    patchUserInstanceRootMtx(*(u8 **)(xlinkBytes + 16), &mXLinkMtx);
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

    mStateMachine.registStateName(cState_Burst, sead::SafeStringBase<char>("cState_Burst"));
    mStateMachine.mStateBuffer[cState_Burst] = Lp::Utl::StateMachine::Delegate<BulletSuperShot>(
        this, &BulletSuperShot::stateEnterBurst, &BulletSuperShot::stateBurst, NULL);

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
    mHasBurst = false;
    mSystemActive = false;
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
    mHasBurst = false;
    setTeam(sender->mTeam);

    // Set xlink matrix to current position
    mXLinkMtx = sead::Matrix34<float>::ident;
    mXLinkMtx.matrix[0][3] = mPos.mX;
    mXLinkMtx.matrix[1][3] = mPos.mY;
    mXLinkMtx.matrix[2][3] = mPos.mZ;

    if (mSystemActive) {
        // Actor is already active — just restart flight state.
        // This avoids the sleep→reactivate cycle which corrupts the vtable pointer.
        setXLinkRootMtx();
        Lp::Sys::XLink *xlink = getXLink();
        if (xlink) {
            xlink->setLocalPropertyValue(0, 1.0f);
            xlink->setLocalPropertyValue(1, 1.0f);
            xlink->setLocalPropertyValue(2, 5.0f);
        }
        mStateMachine.changeState(cState_Flight);
    } else {
        // First launch — actor hasn't been activated yet.
        // reserveActivate queues for the activation processor, which calls actorOnActivate.
        asLpActor()->reserveActivate(true);
    }
}

void BulletSuperShot::doBurst() {
    if (mHasBurst) return;
    mHasBurst = true;
    mActive = false;

    // Update xlink matrix to burst position
    mXLinkMtx.matrix[0][3] = mPos.mX;
    mXLinkMtx.matrix[1][3] = mPos.mY;
    mXLinkMtx.matrix[2][3] = mPos.mZ;

    // Update xlink properties for burst
    Lp::Sys::XLink *xlink = getXLink();
    if (xlink) {
        xlink->setLocalPropertyValue(0, 1.0f); // BulletDamageType = DamageP
        xlink->setLocalPropertyValue(1, 1.0f); // BulletPaintType = Paint
        xlink->setLocalPropertyValue(2, 4.0f); // GndMaterial = Field
    }

    // Paint burst splash (same approach as BulletBombBase::burst_ → splashBullet_ForBurst)
    Cmn::Def::Team team = *(Cmn::Def::Team *)(_actorBase + 0x328);
    sead::Vector3<float> downDir;
    downDir.mX = 0.0f;
    downDir.mY = -1.0f;
    downDir.mZ = 0.0f;
    sead::Vector2<float> paintSize;
    paintSize.mX = BURST_PAINT_RADIUS * 2.0f;
    paintSize.mY = BURST_PAINT_RADIUS * 2.0f;
    Game::PaintUtl::requestColAndPaint(
        mPos, paintSize, downDir,
        (Game::PaintTexType)0, team,
        mPos, false, 3, 0.0f, false);

    // Apply damage to nearby players
    Game::PlayerMgr *pmgr = Game::PlayerMgr::sInstance;
    if (pmgr && mSender) {
        for (int i = 0; i < pmgr->mTotalPlayers; i++) {
            Game::Player *target = pmgr->getPerformerAt(i);
            if (target == NULL) continue;
            if (target->mTeam == mSender->mTeam) continue;
            float dx = mPos.mX - target->mPosition.mX;
            float dy = mPos.mY - target->mPosition.mY;
            float dz = mPos.mZ - target->mPosition.mZ;
            float distSq = dx*dx + dy*dy + dz*dz;
            if (distSq < BURST_DAMAGE_RADIUS * BURST_DAMAGE_RADIUS) {
                sead::Vector3<float> hitDir;
                float dist = sqrtf(distSq);
                if (dist > 0.0f) {
                    float inv = 1.0f / dist;
                    hitDir.mX = -dx * inv;
                    hitDir.mY = -dy * inv;
                    hitDir.mZ = -dz * inv;
                } else {
                    hitDir.mX = 0.0f;
                    hitDir.mY = -1.0f;
                    hitDir.mZ = 0.0f;
                }
                Game::DamageReason reason;
                reason.mWeaponId = 0xFFFF;
                reason.mClassType = 1; // bomb type
                target->receiveDamage_Net(
                    (int)mSender->mIndex, DAMAGE,
                    hitDir, reason, false, true, true);
            }
        }
    }

    // Reset frame counter for burst timer
    mFrame = 0;

    // Transition to burst state (triggers xlink burst effects)
    mStateMachine.changeState(cState_Burst);
}

void BulletSuperShot::doSleep() {
    mSender = NULL;
    mActive = false;
    mHasBurst = false;
    mFrame = 0;
    mPos = sead::Vector3<float>::zero;
    mVel = sead::Vector3<float>::zero;

    // Kill looping xlink effects (Bullet/BulletCore are loops)
    Lp::Sys::XLink *xlink = getXLink();
    if (xlink) {
        XLinkKillAllSound(xlink);
    }

    // Transition to idle — do NOT call reserveSleepAll_.
    // The sleep→reactivate cycle corrupts the vtable pointer on the second activation.
    // Instead, the actor stays permanently active and idles until the next launch().
    mStateMachine.changeState(cState_Idle);
}

// ============================================================
// State machine callbacks
// ============================================================

void BulletSuperShot::stateEnterFlight() {
    // Effects are emitted on the first stateFlight() frame, NOT here.
    // Emitting during vtOnActivate (which calls changeState → stateEnter)
    // is unsafe on reactivation because the xlink isn't fully re-initialized.
}

void BulletSuperShot::stateFlight() {
    // Emit flight effects on first frame (deferred from stateEnterFlight
    // so xlink is fully initialized after actorOnActivate completes)
    if (mFrame == 0) {
        Lp::Sys::XLink *xlink = getXLink();
        if (xlink) {
            xlink2::Handle h;
            xlink->searchAndEmitWrap("Bullet", false, &h);
            xlink->searchAndEmitWrap("BulletCore", false, &h);
        }
    }

    // Apply gravity
    mVel.mY -= GRAVITY;

    // Calculate movement direction and distance
    float moveLen = sqrtf(mVel.mX * mVel.mX + mVel.mY * mVel.mY + mVel.mZ * mVel.mZ);
    sead::Vector3<float> moveDir;
    if (moveLen > 0.0f) {
        float inv = 1.0f / moveLen;
        moveDir.mX = mVel.mX * inv;
        moveDir.mY = mVel.mY * inv;
        moveDir.mZ = mVel.mZ * inv;
    } else {
        moveDir.mX = 0.0f;
        moveDir.mY = -1.0f;
        moveDir.mZ = 0.0f;
    }

    // Simple ground check (TODO: replace with proper KDGndCol once actor is collision-registered)
    sead::Vector3<float> nextPos;
    nextPos.mX = mPos.mX + mVel.mX;
    nextPos.mY = mPos.mY + mVel.mY;
    nextPos.mZ = mPos.mZ + mVel.mZ;
    if (nextPos.mY < mStartPos.mY - 500.0f) {
        doBurst();
        return;
    }

    // No collision, update position
    mPos.mX += mVel.mX;
    mPos.mY += mVel.mY;
    mPos.mZ += mVel.mZ;
    mFrame++;

    // Update xlink matrix to follow bullet
    mXLinkMtx.matrix[0][3] = mPos.mX;
    mXLinkMtx.matrix[1][3] = mPos.mY;
    mXLinkMtx.matrix[2][3] = mPos.mZ;

    // Maximum flight time
    if (mFrame >= MAX_FLIGHT_FRAMES) {
        doBurst();
    }
}

void BulletSuperShot::stateEnterBurst() {
    // Effects are emitted on the first stateBurst() frame (same reason as flight)
}

void BulletSuperShot::stateBurst() {
    // Emit burst effects on first frame
    if (mFrame == 0) {
        Lp::Sys::XLink *xlink = getXLink();
        if (xlink) {
            xlink2::Handle h;
            xlink->searchAndEmitWrap("FloorSplash", false, &h);
            xlink->searchAndEmitWrap("HitSplash", false, &h);
        }
    }

    // Wait a few frames for burst effects to play, then sleep
    mFrame++;
    if (mFrame >= 30) {
        doSleep();
    }
}

void BulletSuperShot::stateEnterIdle() {
    // Actor stays active but does nothing until next launch().
    // Xlink effects are already killed in doSleep() before we get here.
}

void BulletSuperShot::stateIdle() {
    // Do nothing — waiting for launch()
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

    // Set initial xlink property values
    // elink index 0 = BulletDamageType (DamageP=1)
    // elink index 1 = BulletPaintType  (Paint=1)
    // elink index 2 = GndMaterial      (NoGnd=5)
    Lp::Sys::XLink *xlink = self->getXLink();
    if (xlink) {
        xlink->setLocalPropertyValue(0, 1.0f);
        xlink->setLocalPropertyValue(1, 1.0f);
        xlink->setLocalPropertyValue(2, 5.0f);
    }

    self->setXLinkRootMtx();
    self->mStateMachine.changeState(cState_Flight);
}

void BulletSuperShot::vtOnSleep(BulletSuperShot *self) {
    // Safety net: if the actor system sleeps us (e.g. parent player sleeps),
    // reset state. The base actorOnSleep wrapper handles xlink cleanup.
    self->mSystemActive = false;
    self->mSender = NULL;
    self->mActive = false;
    self->mHasBurst = false;
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
    return false; // No slink resource exists for BulletSuperShot
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
    return 3; // BulletDamageType, BulletPaintType, GndMaterial
}

} // namespace Game

// Global init function (called from outside the namespace)
void initBSSVtable(Game::BulletSuperShot *bss) {
    Game::initBSSVtable_internal(bss);
}
