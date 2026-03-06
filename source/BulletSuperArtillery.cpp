#include "flexlion/BulletSuperArtillery.hpp"
#include "flexlion/InkstrikeMgr.hpp"
#include "Game/BulletMgr.h"
#include "Game/BulletSpSuperBall.h"
#include "Game/Player/Player.h"
#include "Game/Player/PlayerSuperBall.h"
#include "Game/Player/PlayerInkAction.h"
#include "Game/MainMgr.h"

extern "C" {
    void _ZN3Cmn5ActorC2Ev(void *);
    void _ZN6xlink222EnumPropertyDefinitionC2EPKcb(void *, const char *, bool);
    void _ZN6xlink222EnumPropertyDefinition12setEntryBuf_EiPNS0_5EntryE(void *, int, void *);
    void _ZN6xlink222EnumPropertyDefinition5entryEiPKc(void *, int, const char *);
}
#define CmnActorCtor _ZN3Cmn5ActorC2Ev
#define EnumPropDefCtor _ZN6xlink222EnumPropertyDefinitionC2EPKcb
#define EnumPropDefSetEntryBuf _ZN6xlink222EnumPropertyDefinition12setEntryBuf_EiPNS0_5EntryE
#define EnumPropDefEntry _ZN6xlink222EnumPropertyDefinition5entryEiPKc

// Flight parameters
const int BSA_PRONOUNCE_DURATION = 15;
const int BSA_FLIGHT_TIME = 150;
const float BSA_FLIGHT_HEIGHT = 300.0f;

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

void resetBSAStatics() {
    sInitVtable = false;
    sUserPropInit = false;
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

void BulletSuperArtillery::launch(Game::Player *sender, sead::Vector3<float> src, sead::Vector3<float> dst, int paintgamefrm) {
    mSender = sender;
    mFrom = src;
    mTo = dst;
    mStartFrm = paintgamefrm;
    mPos = src;
    mRot.mX = 0.0f;
    mRot.mZ = 0.0f;
    mRot.mY = atan2f(src.mX - dst.mX, src.mZ - dst.mZ) + MATH_PI;
    if (mRot.mY > MATH_PI * 2.0f) mRot.mY -= MATH_PI * 2.0f;
    mFlightActive = true;
    mHasBurst = false;
    mSuperball = NULL;

    asLpActor()->reserveActivate(true);
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
    if (!self->mFlightActive && !self->mHasBurst) return;
    self->mStateMachine.executeState();
}

void BulletSuperArtillery::vtFourthCalc(BulletSuperArtillery *self) {
    self->updateModelMatrix();
}

void BulletSuperArtillery::vtOnActivate(BulletSuperArtillery *self, bool) {
    // Build mXLinkMtx as identity with translation = target landing position.
    // The xlink2 system reads from this matrix for boneless effects (Laser, LaserIcon).
    // Bone-attached effects (JetSmoke=root, JetSplash=screw) use bone matrices and are unaffected.
    self->mXLinkMtx = {{
        1.0f, 0.0f, 0.0f, self->mTo.mX,
        0.0f, 1.0f, 0.0f, self->mTo.mY,
        0.0f, 0.0f, 1.0f, self->mTo.mZ
    }};

    // Update the UserInstance root matrix pointer so calcMtx uses our mXLinkMtx
    // for effects without a bone assignment (inline setRootMtx logic)
    self->setXLinkRootMtx();

    // Set XLink "User" property value (0 = Player)
    Lp::Sys::XLink *xlink = self->getXLink();
    if (xlink) {
        xlink->setLocalPropertyValue(0, 0.0f);
    }
    // Start state machine at initial state
    if (self->mFlightActive) {
        self->mStateMachine.changeState(cState_Pronounce);
    }
}

void BulletSuperArtillery::vtOnSleep(BulletSuperArtillery *self) {
    self->reset();
}

int BulletSuperArtillery::vtSetXLinkLocalPropertyDef(BulletSuperArtillery *self, int idx) {
    // Cmn::Actor base returns idx unchanged (no-op for actors without components)
    initUserProperty();
    Lp::Sys::XLinkIUser *xlinkUser = (Lp::Sys::XLinkIUser *)(self->_actorBase + 0x2E8);
    xlinkUser->pushLocalPropertyDefinition((xlink2::PropertyDefinition *)sUserPropBuf);
    return idx + 1;
}

int BulletSuperArtillery::vtCountXLinkLocalProperty(BulletSuperArtillery *self) {
    return 1; // "User"
}

// Update the UserInstance's root matrix pointer (+64) to point at our mXLinkMtx.
// Only changes the default pointer — does NOT patch individual ModelAssetConnections,
// since the game never calls setRootMtx (no xrefs) and patching connections breaks
// effect rendering for JetSmoke/Bomb.
void BulletSuperArtillery::setXLinkRootMtx() {
    Lp::Sys::XLink *xlink = getXLink();
    if (!xlink) return;

    u8 *userInst = *(u8 **)((u8 *)xlink + 8);
    if (!userInst) return;

    *(sead::Matrix34<float> **)(userInst + 64) = &mXLinkMtx;
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
    if (elapsed >= BSA_PRONOUNCE_DURATION) {
        mStateMachine.changeState(cState_Wait);
    }
}

void BulletSuperArtillery::stateEnterWait() {
    // XLink triggers cState_Wait effects:
    // JetSmoke (trail, starts at frame 15 of state)
}

void BulletSuperArtillery::stateWait() {
    calcFlight();
    int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mStartFrm;
    if (elapsed >= BSA_FLIGHT_TIME) {
        mStateMachine.changeState(cState_Burst);
    }
}

void BulletSuperArtillery::stateEnterBurst() {
    // XLink triggers cState_Burst effects: Bomb (explosion)
    doBurst();
}

void BulletSuperArtillery::stateBurst() {
    calcBurstFollow();
}

// ============================================================
// Helpers
// ============================================================

void BulletSuperArtillery::calcFlight() {
    int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mStartFrm;
    float anim = float(elapsed) / float(BSA_FLIGHT_TIME);
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
    if (!mFlightActive && !mHasBurst) return;
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
