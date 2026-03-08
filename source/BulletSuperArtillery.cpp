#include "flexlion/BulletSuperArtillery.hpp"
#include "flexlion/InkstrikeMgr.hpp"
#include "Game/BulletMgr.h"
#include "Game/BulletSpSuperBall.h"
#include "Game/Player/Player.h"
#include "Game/Player/PlayerSuperBall.h"
#include "Game/Player/PlayerInkAction.h"
#include "Game/MainMgr.h"
#include "Cmn/PlayerWeapon.h"

extern "C" {
    void _ZN3Cmn5ActorC2Ev(void *);
    void _ZN6xlink222EnumPropertyDefinitionC2EPKcb(void *, const char *, bool);
    void _ZN6xlink222EnumPropertyDefinition12setEntryBuf_EiPNS0_5EntryE(void *, int, void *);
    void _ZN6xlink222EnumPropertyDefinition5entryEiPKc(void *, int, const char *);
    void _ZN2Lp3Sys5XLink12killAllSoundEv(void *xlink);
    // VTV symbols for manual F32PropertyDefinition construction
    extern u8 _ZTVN6xlink221F32PropertyDefinitionE[];
    extern u8 _ZTVN4sead22BufferedSafeStringBaseIcEE[];
}
#define CmnActorCtor _ZN3Cmn5ActorC2Ev
#define EnumPropDefCtor _ZN6xlink222EnumPropertyDefinitionC2EPKcb
#define EnumPropDefSetEntryBuf _ZN6xlink222EnumPropertyDefinition12setEntryBuf_EiPNS0_5EntryE
#define EnumPropDefEntry _ZN6xlink222EnumPropertyDefinition5entryEiPKc
#define XLinkKillAllSound _ZN2Lp3Sys5XLink12killAllSoundEv

// Flight parameters
const int BSA_FLIGHT_TIME = 120;
const float BSA_FLIGHT_HEIGHT = 400.0f;
const float tornadoTankZOffset = -3.0f;

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
        sBSAVtable.fourthCalc = (u64)((void (*)(BulletSuperArtillery *))&BulletSuperArtillery::vtFourthCalc);
        sBSAVtable.onActivate = (u64)((void (*)(BulletSuperArtillery *, bool))&BulletSuperArtillery::vtOnActivate);
        sBSAVtable.onSleep = (u64)((void (*)(BulletSuperArtillery *))&BulletSuperArtillery::vtOnSleep);
        sBSAVtable.setXlinkLocalPropertyDefinition = (u64)((int (*)(BulletSuperArtillery *, int))&BulletSuperArtillery::vtSetXLinkLocalPropertyDef);
        sBSAVtable.countXlinkLocalProperty = (u64)((int (*)(BulletSuperArtillery *))&BulletSuperArtillery::vtCountXLinkLocalProperty);
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
    mSuperball = NULL;
    mFrom = sead::Vector3<float>::zero;
    mTo = sead::Vector3<float>::zero;
    mPos = sead::Vector3<float>::zero;
    mRot = sead::Vector3<float>::zero;
    mStartFrm = 0;
    mFlightActive = false;
    mHasBurst = false;
    mXLinkMtx = sead::Matrix34<float>::ident;
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
    mSuperball = NULL;

    asLpActor()->reserveActivate(true);
}

void BulletSuperArtillery::launch(sead::Vector3<float> src, sead::Vector3<float> dst, int paintgamefrm) {
    mFrom = src;
    mTo = dst;
    mStartFrm = paintgamefrm;
    mPos = src;
    mRot.mX = 0.0f;
    mRot.mZ = 0.0f;
    mRot.mY = atan2f(src.mX - dst.mX, src.mZ - dst.mZ) + MATH_PI;
    if (mRot.mY > MATH_PI * 2.0f) mRot.mY -= MATH_PI * 2.0f;
    mFlightActive = true;

    // Update xlink root matrix to target position
    mXLinkMtx = {{
        1.0f, 0.0f, 0.0f, mTo.mX,
        0.0f, 1.0f, 0.0f, mTo.mY,
        0.0f, 0.0f, 1.0f, mTo.mZ
    }};
    setXLinkRootMtx();

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
    mSuperball = NULL;
    mFrom = sead::Vector3<float>::zero;
    mTo = sead::Vector3<float>::zero;
    mPos = sead::Vector3<float>::zero;
    mRot = sead::Vector3<float>::zero;
    mStartFrm = 0;
    mFlightActive = false;
    mHasBurst = false;
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

void BulletSuperArtillery::vtFourthCalc(BulletSuperArtillery *self) {
    if (self->mFlightActive) {
        self->updateModelMatrix();
    } else if (!self->mHasBurst) {
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
    // Set XLink local property values
    Lp::Sys::XLink *xlink = self->getXLink();
    if (xlink) {
        // Property 0: "User" (0.0 = Player)
        xlink->setLocalPropertyValue(0, 0.0f);
        // Property 1: "BulletDistance" (set later in launch())
        xlink->setLocalPropertyValue(1, 0.0f);
    }

    // Update the UserInstance root matrix pointer so calcMtx uses our mXLinkMtx
    self->setXLinkRootMtx();

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

// Inline reimplementation of xlink2::UserInstance::setRootMtx.
// Updates the UserInstance's default matrix pointer (+64) and patches all
// ModelAssetConnections that were using the old pointer, so boneless effects
// (Laser, LaserIcon) read from our mXLinkMtx while bone effects are untouched.
void BulletSuperArtillery::setXLinkRootMtx() {
    Lp::Sys::XLink *xlink = getXLink();
    if (!xlink) return;

    // Lp::Sys::XLink+8 = xlink2::UserInstanceELink*
    u8 *userInst = *(u8 **)((u8 *)xlink + 8);
    if (!userInst) return;

    sead::Matrix34<float> **rootMtxPtr = (sead::Matrix34<float> **)(userInst + 64);
    sead::Matrix34<float> *oldMtx = *rootMtxPtr;
    sead::Matrix34<float> *newMtx = &mXLinkMtx;
    if (oldMtx == newMtx) return;

    // Connection array struct at UserInstance + 32 or +40 (depending on flag bit 0 at +208)
    u8 flag = *(userInst + 208) & 1;
    u8 *connStruct = *(u8 **)(userInst + 32 + flag * 8);
    if (connStruct && *(connStruct + 32)) {
        int count = *(int *)connStruct;
        u8 *connArray = *(u8 **)(connStruct + 8);
        for (int i = 0; i < count; i++) {
            u8 *conn = connArray + 24 * i;
            // Only update connections that pointed to the old default matrix
            if (*(sead::Matrix34<float> **)(conn + 8) == oldMtx) {
                *(sead::Matrix34<float> **)(conn + 8) = newMtx;
                *(conn + 16) = 0; // row-major flag
            }
        }
    }

    *rootMtxPtr = newMtx;
    *(userInst + 72) = 0; // row-major flag
}

// ============================================================
// State machine callbacks
// ============================================================

void BulletSuperArtillery::stateEnterPronounce() {
}

void BulletSuperArtillery::statePronounce() {
    calcFlight();
    int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mStartFrm;
    int flightTime = (mSender != NULL && mSender->isInTrouble_Dying()) ? 150 : BSA_FLIGHT_TIME;
    if (elapsed >= flightTime) {
        mStateMachine.changeState(cState_Burst);
    }
}

void BulletSuperArtillery::stateEnterWait() {
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
    int flightTime = (mSender != NULL && mSender->isInTrouble_Dying()) ? 150 : BSA_FLIGHT_TIME;
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

    Game::PlayerSuperBall *playerSuperBall = mSender->mPlayerSuperBall;
    Game::PlayerInkAction *inkAction = mSender->mPlayerInkAction;
    Game::BulletMgr *bulletMgr = Game::MainMgr::sInstance->mBulletMgr;

    if (playerSuperBall && inkAction && bulletMgr) {
        if (playerSuperBall->mBullet == NULL) {
            playerSuperBall->mBullet = (Game::BulletSpSuperBall *)bulletMgr->activateOneCancelUnnecessary(
                0x76, mSender->mIsRemote == 0, mPos, sead::Vector3<float>::zero, -1);
            mSuperball = playerSuperBall->mBullet;
        }
        if (playerSuperBall->mBullet) {
            playerSuperBall->mBullet->Initialize(mSender->mIndex, &mPos);
            InkstrikeMgr::sInstance->isShot = 1;
            inkAction->shotSuperBall();
            InkstrikeMgr::sInstance->isShot = 0;
            playerSuperBall->reset();
            playerSuperBall->mBullet = NULL;
        }
    }

    mFlightActive = false;
    mHasBurst = true;
}

void BulletSuperArtillery::calcBurstFollow() {
    if (mSuperball == NULL) {
        doSleep();
        return;
    }

    // Check if the superball actor is still alive
    auto ballIter = Game::BulletSpSuperBall::getClassIterNodeStatic();
    bool found = false;
    for (auto *bullet = (Game::BulletSpSuperBall *)ballIter->derivedFrontActiveActor();
         bullet != NULL;
         bullet = (Game::BulletSpSuperBall *)ballIter->derivedNextActiveActor(bullet)) {
        if (mSuperball == bullet) {
            found = true;
            break;
        }
    }

    if (!found) {
        doSleep();
        return;
    }

    // Scale the superball core matrix for tornado visual effect
    if (mSuperball->mIsHitGnd && mSuperball->mCore != NULL) {
        sead::Matrix34<float> *matrix = &mSuperball->mCore->mMatrix;
        *matrix = {{
            0.7f, 0.0f, 0.0f, matrix->matrix[0][3],
            0.0f, 3.0f, 0.0f, matrix->matrix[1][3],
            0.0f, 0.0f, 0.7f, matrix->matrix[2][3]
        }};
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
    asLpActor()->reserveSleepAll_(Lp::Sys::Actor::ListNodeKind::None);
}

} // namespace Flexlion
