#include "Game/EnemyBall.h"
#include "Game/BulletMgr.h"
#include "Game/Utl.h"


// Cool static factoryy yayyyyy
static Game::Utl::ActorFactory<Game::EnemyBall> sEnemyBallFactory(Game::Utl::ActorFactoryType::EnemyMgr, sead::SafeStringBase<char>("EnemyBall"));


extern "C" {
    void _ZN2Lp3Sys6Params13pushBackChildEPNS0_9ParamNodeE(void *, void *);
    void _ZN4Game3Cmp28SearchablePlayerParamsFamilyC2ERKN2Lp3Sys9ParamNode3ArgEPNS3_6ParamsE(void *, const void *, void *);
    void _ZN2Lp3Sys6ParamsC2ERKNS0_9ParamNode3ArgEPS1_(void *, const void *, void *);
    void _ZN3Cmn13ComponentBase13ComponentBaseEPNS_15ComponentHolderE(void *, void *);
    void _ZN4Game6Player12informImpactERKN4sead7Vector3IfEENS0_9HitMotionEff(void *, const void *, int, float, float);
}
static u64 sLpSysParamFloatVtable = 0x29E8258;
static u64 sLpSysParamIntVtable = 0x29E9BB0;
static u64 sLpSysParamBoolVtable = 0x29E8060;
#define pushBackChild _ZN2Lp3Sys6Params13pushBackChildEPNS0_9ParamNodeE
#define SPParamsFamilyCtor _ZN4Game3Cmp28SearchablePlayerParamsFamilyC2ERKN2Lp3Sys9ParamNode3ArgEPNS3_6ParamsE
#define LpSysParamsCtor _ZN2Lp3Sys6ParamsC2ERKNS0_9ParamNode3ArgEPS1_
#define ComponentBaseCtor _ZN3Cmn13ComponentBase13ComponentBaseEPNS_15ComponentHolderE
#define PlayerInformImpact _ZN4Game6Player12informImpactERKN4sead7Vector3IfEENS0_9HitMotionEff

static u64 sTrackPaintableVtable = 0x2A5F478;
static u64 sTrackPaintableInnerVtable = 0x2A5F568;

static void initFloatParam(char *param, const char *name, float val, void *parent) {
    *(u64 *)(param + 0x00) = ProcessMemory::MainAddr(sLpSysParamFloatVtable);
    *(u64 *)(param + 0x08) = 0;
    *(u64 *)(param + 0x10) = (u64)name;
    *(u64 *)(param + 0x18) = 0;
    *(u64 *)(param + 0x20) = 0;
    *(u32 *)(param + 0x28) = 0;
    pushBackChild(parent, param);
    *(float *)(param + 0x2C) = val;
    *(u32 *)(param + 0x28) &= ~0x400000u;
}

static void initIntParam(char *param, const char *name, int val, void *parent) {
    *(u64 *)(param + 0x00) = ProcessMemory::MainAddr(sLpSysParamIntVtable);
    *(u64 *)(param + 0x08) = 0;
    *(u64 *)(param + 0x10) = (u64)name;
    *(u64 *)(param + 0x18) = 0;
    *(u64 *)(param + 0x20) = 0;
    *(u32 *)(param + 0x28) = 0;
    pushBackChild(parent, param);
    *(int *)(param + 0x2C) = val;
    *(u32 *)(param + 0x28) &= ~0x400000u;
}

static void initBoolParam(char *param, const char *name, bool val, void *parent) {
    *(u64 *)(param + 0x00) = ProcessMemory::MainAddr(sLpSysParamBoolVtable);
    *(u64 *)(param + 0x08) = 0;
    *(u64 *)(param + 0x10) = (u64)name;
    *(u64 *)(param + 0x18) = 0;
    *(u64 *)(param + 0x20) = 0;
    *(u32 *)(param + 0x28) = 0;
    pushBackChild(parent, param);
    *(u8 *)(param + 0x2C) = val ? 1 : 0;
    *(u32 *)(param + 0x28) &= ~0x400000u;
}

static bool initVtable = false;
static bool initParamsVtable = false;
static bool initFamilyVtable = false;
Game::EnemyBaseVtable cstmVtable;
Game::EnemyBallParamsVtable cstmParamsVtable;
u64 cstmFamilyVtable[41];

void initEnmBallVtable(Game::EnemyBall *enm) {
    Game::EnemyBaseVtable **vtable = enm->getEnmVtable();
    if (!initVtable) {
        memcpy(&cstmVtable, *vtable, sizeof(Game::EnemyBaseVtable));
        cstmVtable.getClassName = (u64)((const char *(*)(Game::EnemyBall *))&Game::EnemyBall::getClassName);
        cstmVtable.setXlinkLocalPropertyDefinition = (u64)((void (*)(Game::EnemyBall *, int))&Game::EnemyBall::setXLinkLocalPropertyDefinition_);
        cstmVtable.countXlinkLocalProperty = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::countXLinkLocalProperty_);
        cstmVtable.executeStateMachine = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::executeStateMachine_);
        cstmVtable.changeFirstState = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::changeFirstState_);
        cstmVtable.load_ = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::load_);
        cstmVtable.reset_ = (u64)((void (*)(Game::EnemyBall *, Cmn::Def::ResetType))&Game::EnemyBall::reset_);
        cstmVtable.firstCalc_ = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::firstCalc_);
        cstmVtable.createDesignerParams = (u64)((Game::EnemyBallParamsDesigner *(*)(Game::EnemyBall *))&Game::EnemyBall::createDesignerParams);
        cstmVtable.isCreateDesignerParamsImpl_ = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::isCreateDesignerParamsImpl_);
        cstmVtable.createMergedParams_ = (u64)((Game::EnemyBallParams *(*)(Game::EnemyBall *))&Game::EnemyBall::createMergedParams_);
        cstmVtable.createEnemyParams_ = (u64)((Game::EnemyBallParamsFamily *(*)(Game::EnemyBall *))&Game::EnemyBall::createEnemyParams_);
        cstmVtable.getPlayerDamage = (u64)((float (*)(Game::EnemyBall *))&Game::EnemyBall::getPlayerDamage);
        cstmVtable.interactCol = (u64)((void (*)(Game::EnemyBall *, Game::CollisionEventArg const&))&Game::EnemyBall::interactCol);
        initVtable = true;
    }
    *vtable = &cstmVtable;
}

namespace Game {

EnemyBallParamsFamily::EnemyBallParamsFamily() : EnemyParamsFamily() {
    initFloatParam((char *)&mSpeed,                "mSpeed",                1.0f,    this);
    initFloatParam((char *)&mSpeedPL,              "mSpeedPL",              0.06f,   this);
    initFloatParam((char *)&mAcc,                  "mAcc",                  0.1f,    this);
    initFloatParam((char *)&mRotRate,              "mRotRate",              0.1f,    this);
    initFloatParam((char *)&mRotSpeed,             "mRotSpeed",             0.01f,   this);
    initFloatParam((char *)&mRotSpeedPL,           "mRotSpeedPL",           0.001f,  this);
    initFloatParam((char *)&mTrackPaintRadius,     "mTrackPaintRadius",     10.0f,   this);
    initFloatParam((char *)&mImpactToPlayer,       "mImpactToPlayer",       3.0f,    this);
    initFloatParam((char *)&mImpactToPlayerX,      "mImpactToPlayerX",      3.0f,    this);
    initFloatParam((char *)&mImpactBulletX,        "mImpactBulletX",        3.0f,    this);
    initIntParam  ((char *)&mDamagePlayer,         "mDamagePlayer",         400,     this);
    initFloatParam((char *)&mMonitorHalfSize,      "mMonitorHalfSize",      10.0f,   this);
    initFloatParam((char *)&mTrackPaintableOffsetZ,"mTrackPaintableOffsetZ",-10.0f,  this);
    initBoolParam ((char *)&mMuteki,               "mMuteki",               false,   this);
    initBoolParam ((char *)&mMutekiBulletIgnore,   "mMutekiBulletIgnore",   false,   this);
    initIntParam  ((char *)&mChanceSinkFrame,      "mChanceSinkFrame",      20,      this);
    initIntParam  ((char *)&mMutekiRefreshFrame,   "mMutekiRefreshFrame",   20,      this);
    initBoolParam ((char *)&mIsMutekiChanceChanging,"mIsMutekiChanceChanging", false,this);
    initBoolParam ((char *)&mIsMutekiNormalChanging,"mIsMutekiNormalChanging", false,this);
    initFloatParam((char *)&mChanceStopRollAngle,  "mChanceStopRollAngle", -40.0f,   this);
    initIntParam  ((char *)&mReturnWaitFrame,      "mReturnWaitFrame",      4,       this);
    initBoolParam ((char *)&mStopMoveOnReturn,     "mStopMoveOnReturn",     true,    this);
    initFloatParam((char *)&mTrackPaintableRadius,  "mTrackPaintableRadius", 32.0f,  this);
    initFloatParam((char *)&mBulletHitPaintRadius,  "mBulletHitPaintRadius", 20.0f,  this);

    struct {
        sead::SafeStringBase<char> name;
        sead::SafeStringBase<char> label;
        sead::SafeStringBase<char> meta;
        int flags;
    } arg;
    arg.name = sead::SafeStringBase<char>("mSearchablePlayerParams");
    arg.label = sead::SafeStringBase<char>("");
    arg.meta = sead::SafeStringBase<char>("");
    arg.flags = 0;
    SPParamsFamilyCtor((char *)&mSearchablePlayerParamsFamily, &arg, this);
}

static void initSubParams(char *subBase, const char *name, void *parent) {
    struct {
        sead::SafeStringBase<char> sName;
        sead::SafeStringBase<char> sLabel;
        sead::SafeStringBase<char> sMeta;
        int flags;
    } arg;
    arg.sName = sead::SafeStringBase<char>(name);
    arg.sLabel = sead::SafeStringBase<char>("");
    arg.sMeta = sead::SafeStringBase<char>("");
    arg.flags = 0;
    LpSysParamsCtor(subBase, &arg, parent);
}

EnemyBallParamsDesigner::EnemyBallParamsDesigner() : EnemyParamsDesigner() {
    initSubParams((char *)&mBulletKnockbackableDesigner, "mBulletKnockbackableParams", this);
    initFloatParam((char *)&mBulletKnockbackableDesigner.mImpactBulletZ, "mImpactBulletZ", 2.0f,  &mBulletKnockbackableDesigner);
    initFloatParam((char *)&mBulletKnockbackableDesigner.mSinkRate,      "mSinkRate",      1.0f,  &mBulletKnockbackableDesigner);
    initFloatParam((char *)&mBulletKnockbackableDesigner.mImpactVelGs,   "mImpactVelGs",   0.7f,  &mBulletKnockbackableDesigner);

    initSubParams((char *)&mSinkableDesigner, "mSinkableParams", this);
    initFloatParam((char *)&mSinkableDesigner.mSinkAcc,      "mSinkAcc",      0.05f,  &mSinkableDesigner);
    initFloatParam((char *)&mSinkableDesigner.mSinkSpeed,    "mSinkSpeed",    1.0f,   &mSinkableDesigner);
    initFloatParam((char *)&mSinkableDesigner.mOfstMax,      "mOfstMax",     -4.0f,   &mSinkableDesigner);
    initFloatParam((char *)&mSinkableDesigner.mSpringK,      "mSpringK",      0.05f,  &mSinkableDesigner);
    initFloatParam((char *)&mSinkableDesigner.mSpringDamp,   "mSpringDamp",   0.95f,  &mSinkableDesigner);
    initBoolParam((char *)&mSinkableDesigner.mSpringEnable,  "mSpringEnable", true,   &mSinkableDesigner);

    initSubParams((char *)&mBoneDirectableDesigner, "mEyeDirectableParams", this);
    initFloatParam((char *)&mBoneDirectableDesigner.mAngleLimitP, "mAngleLimitP", 30.0f, &mBoneDirectableDesigner);
    initFloatParam((char *)&mBoneDirectableDesigner.mAngleLimitY, "mAngleLimitY", 30.0f, &mBoneDirectableDesigner);
    initFloatParam((char *)&mBoneDirectableDesigner.mLerpRate,    "mLerpRate",    0.1f,  &mBoneDirectableDesigner);
}

EnemyBallParams::EnemyBallParams() : EnemyParams() {
    if (!initParamsVtable) {
        memcpy(&cstmParamsVtable, *(u64 **)this, sizeof(EnemyBallParamsVtable));
        cstmParamsVtable.readMuObj = (u64)(void (*)(EnemyBallParams *, Cmn::MuObj const*, sead::SafeStringBase<char> const&))&EnemyBallParams::readMuObj;
        cstmParamsVtable.readFamilyParams = (u64)(void (*)(EnemyBallParams *, Game::EnemyBallParamsFamily const*))&EnemyBallParams::readFamilyParams;
        cstmParamsVtable.readDesignerParams = (u64)(void (*)(EnemyBallParams *, Game::EnemyBallParamsDesigner const*))&EnemyBallParams::readDesignerParams;
        initParamsVtable = true;
    }
    *(u64 **)this = (u64 *)&cstmParamsVtable;
}

EnemyBallParams::~EnemyBallParams() {
}

void EnemyBallParams::readMuObj(Cmn::MuObj const*muObj, sead::SafeStringBase<char> const&name) {
    EnemyParams::readMuObj(muObj, name);
}

void EnemyBallParams::readFamilyParams(Game::EnemyBallParamsFamily const*familyParams) {
    EnemyParams::readFamilyParams(familyParams);
    mSearchablePlayerParams.readFamilyParams(&familyParams->mSearchablePlayerParamsFamily);
}

void EnemyBallParams::readDesignerParams(Game::EnemyBallParamsDesigner const*designerParams) {
    EnemyParams::readDesignerParams((Lp::Sys::Params const*)designerParams);
    mBulletKnockbackableParams.readDesignerParams(&designerParams->mBulletKnockbackableDesigner);
    mSinkableParams.mDesignerParams = &designerParams->mSinkableDesigner;
    mBoneDirectableParams.mDesignerParams = &designerParams->mBoneDirectableDesigner;
}

EnemyBall::EnemyBall() {
    initEnmBallVtable(this);

    mStateMachine.initialize(5, 0, NULL);
    this->setInnerStateMachinePtr(&mStateMachine);

    mStateMachine.registStateName(0, sead::SafeStringBase<char>("State::cWait"));
    mStateMachine.mStateBuffer[0] = Lp::Utl::StateMachine::Delegate<Game::EnemyBall>(this, &EnemyBall::stateEnterWait_, &EnemyBall::stateWait_, NULL);

    mStateMachine.registStateName(1, sead::SafeStringBase<char>("State::cNotice"));
    mStateMachine.mStateBuffer[1] = Lp::Utl::StateMachine::Delegate<Game::EnemyBall>(this, &EnemyBall::stateEnterNotice_, &EnemyBall::stateNotice_, NULL);

    mStateMachine.registStateName(2, sead::SafeStringBase<char>("State::cMove"));
    mStateMachine.mStateBuffer[2] = Lp::Utl::StateMachine::Delegate<Game::EnemyBall>(this, &EnemyBall::stateEnterMove_, &EnemyBall::stateMove_, NULL);

    mStateMachine.registStateName(3, sead::SafeStringBase<char>("State::cFall"));
    mStateMachine.mStateBuffer[3] = Lp::Utl::StateMachine::Delegate<Game::EnemyBall>(this, &EnemyBall::stateEnterMove_, &EnemyBall::stateMove_, NULL);

    mStateMachine.registStateName(4, sead::SafeStringBase<char>("State::cDie"));
    mStateMachine.mStateBuffer[4] = Lp::Utl::StateMachine::Delegate<Game::EnemyBall>(this, &EnemyBall::stateEnterDie_, &EnemyBall::stateDie_, NULL);

    mMovable = NULL;
    mFrontDirRotatable = NULL;
    mRollable = NULL;
    mBulletKnockbackable = NULL;
    mChanceAnimSetController = NULL;
    mTrackPaintable = NULL;
    mSearchablePlayer = NULL;
    mBoneDirectableToSearch = NULL;

    mBumpSfxTimer = 0;
    mHitWallCounter = 0;
    mMutekiState = 0;
    mHitKeepOutTimer = 0;
    mPlayerHitTimer = 0;
    mDrownState = 0;
    mIsReturning = 0;
    mWasHitWall = 0;
    
}

EnemyBall::~EnemyBall() {
}

void EnemyBall::destroy(Game::EnemyBase *enm) {
    delete enm;
}

const char *EnemyBall::getClassName() const {
    return "EnemyBall";
}

int EnemyBall::countXLinkLocalProperty_() const {
    return Cmn::Actor::countXLinkLocalProperty_() + 2;
}

void EnemyBall::executeStateMachine_() {
    mStateMachine.executeState();
}

void EnemyBall::changeFirstState_() {
    mStateMachine.changeState(0);
}

EnemyBallParamsDesigner *EnemyBall::createDesignerParams() {
    return new EnemyBallParamsDesigner();
}

bool EnemyBall::isCreateDesignerParamsImpl_() const {
    return true;
}

EnemyBallParams *EnemyBall::createMergedParams_() {
    return new EnemyBallParams();
}

EnemyBallParamsFamily *EnemyBall::createEnemyParams_() {
    return new EnemyBallParamsFamily();
}

void EnemyBall::load_() {
    float *gndColRadiusPtr = (float*)((char*)getFamilyParams() + 0x3D0 + 0x38 + 0x2C);

    mMovable = new Cmp::Movable(mComponentHolder);
    mFrontDirRotatable = new Cmp::FrontDirRotatable(mComponentHolder, Cmp::FrontDirRotatable::Interp::Enabled);
    mRollable = new Cmp::Rollable(mComponentHolder, getKdAttT(), *gndColRadiusPtr, mModelCB);
    mBulletKnockbackable = new Cmp::BulletKnockbackable(mComponentHolder, &getMergedParams()->mBulletKnockbackableParams);

    {
        char *tp = (char *)operator new(0x78);
        memset(tp, 0, 0x78);
        ComponentBaseCtor(tp, mComponentHolder);
        *(u64 *)(tp + 0x00) = ProcessMemory::MainAddr(sTrackPaintableVtable);
        *(u64 *)(tp + 0x10) = ProcessMemory::MainAddr(sTrackPaintableInnerVtable);
        *(u32 *)(tp + 0x2C) = 0xBF800000;
        *(u32 *)(tp + 0x34) = 0;
        *(s32 *)(tp + 0x38) = -1;
        float trackPaintRadius = getFamilyParams()->mTrackPaintableRadius.mValue;
        Cmp::TrackPaintable *trackPaintable = (Cmp::TrackPaintable *)tp;
        trackPaintable->init_(Cmn::Def::Team::Bravo, trackPaintRadius);
        mTrackPaintable = trackPaintable;
    }

    float *footPaintCheckThresholdPtr = (float*)((char*)getFamilyParams() + 0x300 + 0x2C);
    createFootPaintCheckSet_(&getMergedParams()->mSinkableParams, footPaintCheckThresholdPtr);

    mPixelMonitorDiskHolder->mMonitorHalfSize = &getFamilyParams()->mMonitorHalfSize.mValue;
    mPixelMonitorDiskHolder->mKdPos = &getKdAttT()->mPos;

    mChanceAnimSetController = new Cmp::AnimSetController(mComponentHolder, 1, NULL);
    mChanceAnimSetController->setModel(mActorFullModel);
    mChanceAnimSetController->setMatSlot(5);

    mSearchablePlayer = new Cmp::SearchablePlayer(mComponentHolder, &getMergedParams()->mSearchablePlayerParams, (Game::Actor *)this);

    mBoneDirectableToSearch = new Cmp::BoneDirectableToSearch(
        mComponentHolder, &getMergedParams()->mBoneDirectableParams, 2, mModelCB, mSearchablePlayer);

    sead::SafeStringBase<char> eyeL("eye_L");
    mBoneDirectableToSearch->mBoneIndices[0] = mActorFullModel->searchBone(eyeL);
    sead::SafeStringBase<char> eyeR("eye_R");
    mBoneDirectableToSearch->mBoneIndices[1] = mActorFullModel->searchBone(eyeR);
}

void EnemyBall::reset_(Cmn::Def::ResetType resetType) {
    Cmn::KDUtl::AttT *att = getKdAttT();
    Cmn::KDUtl::AttT *initAtt = (Cmn::KDUtl::AttT *)((char*)this + 0x348);
    memcpy(att, initAtt, sizeof(Cmn::KDUtl::AttT));

    mMovable->reset();

    *(u32*)((char*)mEnemyPhysics + 48) |= 0x48u;

    float gndColRadius = *(float*)((char*)getFamilyParams() + 0x3D0 + 0x38 + 0x2C);
    att->mPos.mY += gndColRadius;

    mPrevPos = att->mPos;

    sead::Vector3<float> *physFrontDir = &mEnemyPhysics->mFrontDir;
    *physFrontDir = *(sead::Vector3<float>*)((char*)initAtt + 0x18);

    sead::SafeStringBase<char> rollBone("roll");
    mRollable->mRollBoneIdx = mActorFullModel->searchBone(rollBone);

    mSinkable->reset();
    mBulletKnockbackable->reset();

    mMutekiState = 1;
    mChanceAnimSetController->reset();
    mChanceAnimSetController->start("Chance");
    float numFrame = mChanceAnimSetController->getCurrentAnimNumFrame();
    mChanceAnimSetController->setCurrentFrame(numFrame);
    mChanceAnimSetController->setUpdateRate(0.0f);

    setObjColMuteki_();

    mBumpSfxTimer = 0;
    mHitWallCounter = 0;
    mHitKeepOutTimer = 0;
    mDrownState = 0;
    mIsReturning = 0;
    mWasHitWall = 0;
    mPlayerHitTimer = 0;

    *(void**)((char*)mSearchablePlayer + 56) = &mSearchOwnerMtx;
}

void EnemyBall::firstCalc_() {
    Cmn::KDUtl::AttT *att = getKdAttT();
    
    calcSearchOwnerMtx_();

    mSearchablePlayer->search();

    this->calcChangeState_();

    this->calcChangeAnim_();
    this->calcMuteki_();
    this->calcTrackPaintable_();
    this->setObjColMuteki_();

    sead::Vector3<float> &movVel = mMovable->mVelocity;
    float vel = sqrtf((movVel.mX * movVel.mX) + (movVel.mY * movVel.mY) + (movVel.mZ * movVel.mZ));
    mXLink->setLocalPropertyValue(1, vel);

    if (mBumpSfxTimer > 0) mBumpSfxTimer--;
    if (mHitKeepOutTimer > 0) mHitKeepOutTimer--;
    if (mPlayerHitTimer > 0) mPlayerHitTimer--;

    mBulletKnockbackable->updatePos(&att->mPos);
}

void EnemyBall::calcSearchOwnerMtx_() {
    Cmn::KDUtl::AttT *att = getKdAttT();
    sead::Vector3<float> *physFrontDir = &mEnemyPhysics->mFrontDir;

    mPrevPos = att->mPos;

    mSearchDir.mX = (1.0f * physFrontDir->mZ) - (0.0f * physFrontDir->mY);
    mSearchDir.mY = (0.0f * physFrontDir->mX) - (physFrontDir->mZ * 0.0f);
    mSearchDir.mZ = (physFrontDir->mY * 0.0f) - (1.0f * physFrontDir->mX);

    mCrossDir = sead::Vector3<float>(0.0f, 1.0f, 0.0f);
    mSearchOwnerFwd = *physFrontDir;
}

void EnemyBall::calcSearch_() {
    mSearchablePlayer->search();
}

void EnemyBall::stateEnterWait_() {
    mAnimSetController->start("Wait");
    Cmn::GfxUtl::setTwoColorCompTeam(mActorFullModel, Cmn::Def::Team::Bravo);
}

void EnemyBall::stateWait_() {
    Game::EnemyFunctions::rotateByFrontDir(getKdAttT(), mEnemyPhysics->mFrontDir);
}

void EnemyBall::stateEnterNotice_() {
    mAnimSetController->start("Find");
    mFrontDirRotatable->mInterp = Cmp::FrontDirRotatable::Interp::Disabled;
}

void EnemyBall::stateNotice_() {
    sead::Vector3<float> *physFrontDir = &mEnemyPhysics->mFrontDir;
    sead::Vector3<float> *targetPos = mSearchablePlayer->getTargetPos();
    float rotSpeed = getRotSpeed_();

    Game::Cmp::FrontDirRotatable::CalcArg calcArg;
    calcArg.mPhysFrontDir = physFrontDir;
    calcArg.mCurrentPos = &getKdAttT()->mPos;
    calcArg.mTargetPos = targetPos;
    calcArg.mSpeed = rotSpeed;
    mFrontDirRotatable->calc(calcArg);

    Game::EnemyFunctions::rotateByFrontDir(getKdAttT(), mEnemyPhysics->mFrontDir);
}

void EnemyBall::stateEnterMove_() {
    mFrontDirRotatable->mInterp = Cmp::FrontDirRotatable::Interp::Enabled;
    mAnimSetController->start("Roll");
    mDrownState = 0;
}

void EnemyBall::stateMove_() {
    Cmn::KDUtl::AttT *att = getKdAttT();
    bool hitWall = false;

    if (mEnemyPhysics->mHitFlags & 1) {
        if (mWasHitWall && mHitWallCounter < 30) {
            mHitWallCounter++;
        } else {
            onHitWall_();
            mHitWallCounter = 0;
        }
        hitWall = true;
    }

    sead::Vector3<float> prevVel;
    prevVel.mX = mMovable->mVelocity.mX + mBulletKnockbackable->mVelocity.mX;
    prevVel.mY = mMovable->mVelocity.mY + mBulletKnockbackable->mVelocity.mY;
    prevVel.mZ = mMovable->mVelocity.mZ + mBulletKnockbackable->mVelocity.mZ;

    float speed = getSpeed_();
    sead::Vector3<float> *physFrontDir = &mEnemyPhysics->mFrontDir;

    sead::Vector3<float> moveDir;
    moveDir.mX = (speed * physFrontDir->mX * 2.0f) + att->mPos.mX;
    moveDir.mY = (speed * physFrontDir->mY * 2.0f) + att->mPos.mY;
    moveDir.mZ = (speed * physFrontDir->mZ * 2.0f) + att->mPos.mZ;

    sead::Vector3<float> *nrmGnd = mEnemyPhysics->getNrmGnd();

    Game::Cmp::Movable::CalcArg movCalcArg;
    movCalcArg.mCurrentPos = &att->mPos;
    movCalcArg.mTargetPos = &moveDir;
    movCalcArg.mNrmGnd = nrmGnd;
    movCalcArg.mSpeed = speed;
    movCalcArg.mAcc = getFamilyParams()->mAcc.mValue;
    movCalcArg.mFlag = 0;
    movCalcArg.mExtra = 0;

    int movResult = mMovable->calc(movCalcArg);

    if (!hitWall && (movResult & 1))
        onHitKeepOutWall_(prevVel);

    mWasHitWall = hitWall ? 1 : 0;

    calcRoll_();
    this->calcFDRotatable_();

    Game::EnemyFunctions::rotateByFrontDir(getKdAttT(), mEnemyPhysics->mFrontDir);
}

void EnemyBall::stateEnterDie_() {
    splashBulletForDie_(1, -1.0f, NULL);
    ((void (*)(Game::EnemyBase*))((*getEnmVtable())->setDead))(this);
    this->reserveSleepAll_(Lp::Sys::Actor::ListNodeKind::None);
}

void EnemyBall::stateDie_() {
}

void EnemyBall::calcChangeAnim_() {
    if (mStateMachine.mStateId == 4)
        return;

    if ((u8)mDrownState) {
        if (!mIsReturning) {
            if (!mSinkable->mIsDamaged) {
                if (mSinkable->mEffectFrame >= getFamilyParams()->mReturnWaitFrame.mValue) {
                    mAnimSetController->start("Return");
                    mIsReturning = 1;
                    return;
                }
            }
        }
    }

    int stateId = mStateMachine.mStateId;
    if ((stateId & 0xFFFFFFFE) == 2) {
        if (mAnimSetController->isEndCurrentAnim()) {
            mAnimSetController->start("Roll");
            mDrownState = 0;
            mIsReturning = 0;
            if (mRollable) {
                mRollable->reset();
            }
        }
    }
}

void EnemyBall::calcMuteki_() {
    switch (mMutekiState) {
    case 0: {
        if (mSinkable->mIsDamaged) {
            if (mSinkable->mDamageFrame >= getFamilyParams()->mChanceSinkFrame.mValue) {
                mMutekiState = 3;
                if (mChanceAnimSetController->isEndCurrentAnim()) {
                    mChanceAnimSetController->start("Chance");
                }
                mChanceAnimSetController->setUpdateRate(1.0f);
            }
        }
        break;
    }
    case 1: {
        if (mStateMachine.mStateId >= 2) {
            if (mSinkable->mIsDamaged) {
                if (mChanceAnimSetController->isEndCurrentAnim()) {
                    Cmn::GfxUtl::setTwoColorCompTeam(mActorFullModel, Cmn::Def::Team::Alpha);
                    mChanceAnimSetController->reset();
                }
            } else {
                if (mSinkable->mEffectFrame >= getFamilyParams()->mMutekiRefreshFrame.mValue) {
                    mMutekiState = 2;
                    if (mChanceAnimSetController->isEndCurrentAnim()) {
                        Cmn::GfxUtl::setTwoColorCompTeam(mActorFullModel, Cmn::Def::Team::Bravo);
                        mChanceAnimSetController->start("Chance");
                        float numFrame = mChanceAnimSetController->getCurrentAnimNumFrame();
                        mChanceAnimSetController->setCurrentFrame(numFrame);
                    } else {
                        float curFrame = mChanceAnimSetController->getCurrentFrame();
                        mChanceAnimSetController->setCurrentFrame(curFrame);
                    }
                    mChanceAnimSetController->setUpdateRate(-1.0f);
                }
            }
        }
        break;
    }
    case 2: {
        float curFrame = mChanceAnimSetController->getCurrentFrame();
        if (curFrame == 0.0f)
            mMutekiState = 0;
        break;
    }
    case 3: {
        if (mChanceAnimSetController->isEndCurrentAnim())
            mMutekiState = 1;
        break;
    }
    }
}

void EnemyBall::calcRoll_() {
    int stateId = mStateMachine.mStateId;
    if ((stateId & 0xFFFFFFFE) != 2)
        return;

    bool isDamaged = mSinkable->mIsDamaged;

    if (!isDamaged) {
        mDrownState = 0;
        calcRollable_(true);
        return;
    }

    if (mDrownState)
        return;

    Cmn::KDUtl::AttT *att = getKdAttT();
    float axisX = mRollable->mRollAxis.mX;
    float axisY = mRollable->mRollAxis.mY;
    float axisZ = mRollable->mRollAxis.mZ;

    float dot = (att->mtx10 * axisX) + (att->mtx11 * axisY) + (att->mtx12 * axisZ);

    float sign;
    float chanceStopAngle = getFamilyParams()->mChanceStopRollAngle.mValue;
    if (chanceStopAngle > 0.0f)
        sign = 1.0f;
    else
        sign = -1.0f;

    if ((dot * sign) >= 0.0f && chanceStopAngle != 0.0f) {
        calcRollable_(false);
        return;
    }

    float colX = (att->mtx00 * axisX) + (att->mtx01 * axisY) + (att->mtx02 * axisZ);
    float colZ = (att->mtx20 * axisX) + (att->mtx21 * axisY) + (att->mtx22 * axisZ);

    float lx = mMovable->mVelocity.mX + mBulletKnockbackable->mVelocity.mX;
    float lz = mMovable->mVelocity.mZ + mBulletKnockbackable->mVelocity.mZ;

    float crossLen = sqrtf((dot * lx - colX * 0.0f) * (dot * lx - colX * 0.0f) +
                           (colX * lz - colZ * lx) * (colX * lz - colZ * lx) +
                           (colZ * 0.0f - dot * lz) * (colZ * 0.0f - dot * lz));
    float dotProd = (dot * 0.0f + colX * lx) + (colZ * lz);

    float angle = atan2f(crossLen, dotProd);
    if (dot > 0.0f)
        angle = -angle;

    float rollAngle = angle + mRollable->mRollAngle;
    float stopAngle = chanceStopAngle * 0.017453f;

    if (rollAngle >= stopAngle && stopAngle >= angle) {
        float drownAngle = stopAngle - angle;
        calcRollableForce_(drownAngle);
        mDrownState = 1;
        mIsReturning = 0;
        mAnimSetController->start("Drown");
    }

    if (!mDrownState) {
        calcRollable_(isDamaged == false);
    }
}

void EnemyBall::getMoveVel_(sead::Vector3<float> *out) {
    out->mX = mMovable->mVelocity.mX + mBulletKnockbackable->mVelocity.mX;
    out->mY = mMovable->mVelocity.mY + mBulletKnockbackable->mVelocity.mY;
    out->mZ = mMovable->mVelocity.mZ + mBulletKnockbackable->mVelocity.mZ;
}

void EnemyBall::calcRollableForce_(float angle) {
    Cmp::Rollable::CalcArg arg;
    arg.mMode = 6;
    arg.mExtraValue = angle;
    mRollable->calc(arg);
}

void EnemyBall::calcRollable_(bool isOnGround) {
    Cmn::KDUtl::AttT *att = getKdAttT();

    Cmp::Rollable::CalcArg arg;
    arg.mVelocity.mX = att->mPos.mX - mPrevPos.mX;
    arg.mVelocity.mY = att->mPos.mY - mPrevPos.mY;
    arg.mVelocity.mZ = att->mPos.mZ - mPrevPos.mZ;

    if (isOnGround) {
        arg.mMode = 0;
    } else {
        float rollSpeed = mRollable->mRollAngle;
        arg.mMode = 5;
        arg.mFactor = 0.999f;
        if (rollSpeed < 0.05f)
            mRollable->mRollAngle = 0.05f;
    }

    mRollable->calc(arg);
}

void EnemyBall::calcFDRotatable_() {
    if (!mSearchablePlayer->isLockOn())
        return;

    sead::Vector3<float> *physFrontDir = &mEnemyPhysics->mFrontDir;
    sead::Vector3<float> *targetPos = mSearchablePlayer->getTargetPos();

    Game::Cmp::FrontDirRotatable::CalcArg calcArg;
    calcArg.mPhysFrontDir = physFrontDir;
    calcArg.mCurrentPos = &getKdAttT()->mPos;
    calcArg.mTargetPos = targetPos;
    calcArg.mSpeed = getRotSpeed_();
    mFrontDirRotatable->calc(calcArg);
}

float EnemyBall::getRotSpeed_() const {
    if (mSinkable->mIsDamaged)
        return getFamilyParams()->mRotSpeedPL.mValue;
    return getFamilyParams()->mRotSpeed.mValue;
}

void EnemyBall::calcCheckGndColMoveSphere_() {
    Cmn::KDUtl::AttT *att = getKdAttT();
    float dx = att->mPos.mX - mPrevPos.mX;
    float dy = att->mPos.mY - mPrevPos.mY;
    float dz = att->mPos.mZ - mPrevPos.mZ;
    float distSq = dx * dx + dy * dy + dz * dz;

    float radius = getFamilyParams()->mTrackPaintRadius.mValue;
    if (distSq < (radius * 0.8f) * (radius * 0.8f))
        return;

    float dist = sqrtf(distSq);
    sead::Vector3<float> dir;
    if (dist > 0.0f) {
        dir.mX = dx / dist;
        dir.mY = dy / dist;
        dir.mZ = dz / dist;
    } else {
        dir.mX = dx;
        dir.mY = dy;
        dir.mZ = dz;
    }

    att->mPos = mPrevPos;
    att->mPos.mX += dir.mX * dist;
    att->mPos.mY += dir.mY * dist;
    att->mPos.mZ += dir.mZ * dist;
}

void EnemyBall::calcChangeState_() {
    int stateId = mStateMachine.mStateId;
    if (stateId == 4)
        return;

    if ((int)mHealth <= 0) {
        mStateMachine.changeState(4);
        return;
    }

    switch (stateId) {
    case 0:
        if (mSearchablePlayer->isLockOn()) {
            mStateMachine.changeState(1);
            return;
        }
        {
            if (mSinkable->mIsDamaged && mSinkable->mDamageFrame >= 3)
                mStateMachine.changeState(2);
        }
        break;
    case 1: {
        sead::SafeStringBase<char> findAnim("Find");
        if (!mAnimSetController->isCurrentAnim(findAnim)
            || mAnimSetController->isEndCurrentAnim()) {
            mStateMachine.changeState(2);
        }
        break;
    }
    case 2:
        if (!mEnemyPhysics->isHitGnd()) {
            if (!checkExistGnd_(100.0f))
                mStateMachine.changeState(3);
        }
        break;
    case 3:
        if (mEnemyPhysics->isHitGnd())
            mStateMachine.changeState(2);
        break;
    }
}

bool EnemyBall::checkIsFall_() {
    return !checkExistGnd_(100.0f);
}

void EnemyBall::rotMtxByFrontDir_() {
    Game::EnemyFunctions::rotateByFrontDir(getKdAttT(), mEnemyPhysics->mFrontDir);
}

void EnemyBall::onHitWall_() {
    sead::Vector3<float> vel;
    vel.mX = mMovable->mVelocity.mX + mBulletKnockbackable->mVelocity.mX;
    vel.mY = mMovable->mVelocity.mY + mBulletKnockbackable->mVelocity.mY;
    vel.mZ = mMovable->mVelocity.mZ + mBulletKnockbackable->mVelocity.mZ;

    if (!mBumpSfxTimer) {
        xlink2::Handle handle;
        mXLink->searchAndPlayWrap("Bump", false, &handle);
        mBumpSfxTimer = 15;
    }

    sead::Vector3<float> reflected;
    Game::EnemyFunctions::wallHitReflectArg reflArg;
    reflArg.outVelocity = &reflected;
    reflArg.physics = mEnemyPhysics;
    reflArg.inVelocity = &vel;
    reflArg.wallNormal = &mEnemyPhysics->mNrmWall;
    Game::EnemyFunctions::wallHitReflect(reflArg);

    mMovable->mVelocity = reflected;
    mDrownState = 0;
    mAnimSetController->start("Roll");
    mBulletKnockbackable->reset();
    mHitKeepOutTimer = 0;
}

void EnemyBall::onHitKeepOutWall_(sead::Vector3<float> const& prevVel) {
    sead::Vector3<float> *physNrmWall = &mMovable->mKeepOutNrm;

    if (!mBumpSfxTimer) {
        xlink2::Handle handle;
        mXLink->searchAndPlayWrap("Bump", false, &handle);
        mBumpSfxTimer = 15;
    }

    sead::Vector3<float> reflected;
    Game::EnemyFunctions::wallHitReflectArg reflArg;
    reflArg.outVelocity = &reflected;
    reflArg.physics = mEnemyPhysics;
    reflArg.inVelocity = &prevVel;
    reflArg.wallNormal = physNrmWall;
    Game::EnemyFunctions::wallHitReflect(reflArg);

    mMovable->mVelocity = reflected;
    mDrownState = 0;
    mAnimSetController->start("Roll");
    mBulletKnockbackable->reset();
    mHitKeepOutTimer = 0;
}

void EnemyBall::onHitImpl_(sead::Vector3<float> const& hitPos, sead::Vector3<float> const& hitNrm) {
    if (!mBumpSfxTimer) {
        xlink2::Handle handle;
        mXLink->searchAndPlayWrap("Bump", false, &handle);
        mBumpSfxTimer = 15;
    }

    sead::Vector3<float> reflected;
    Game::EnemyFunctions::wallHitReflectArg reflArg;
    reflArg.outVelocity = &reflected;
    reflArg.physics = mEnemyPhysics;
    reflArg.inVelocity = &hitPos;
    reflArg.wallNormal = &hitNrm;
    Game::EnemyFunctions::wallHitReflect(reflArg);

    mMovable->mVelocity = reflected;
    mDrownState = 0;
    mAnimSetController->start("Roll");
    mBulletKnockbackable->reset();
    mHitKeepOutTimer = 0;
}

float EnemyBall::getSpeed_() const {
    if (mIsReturning) {
        if (getFamilyParams()->mStopMoveOnReturn.mValue)
            return 0.0f;
    }
    if (mSinkable->mIsDamaged)
        return getFamilyParams()->mSpeedPL.mValue;
    return getFamilyParams()->mSpeed.mValue;
}

void EnemyBall::interactCol(Game::CollisionEventArg const& arg) {
    if ((mStateMachine.mStateId & 0xFFFFFFFE) != 2)
        return;

    Cmn::Actor *other = (Cmn::Actor *)arg.mCollidee;
    if (!other)
        return;

    typedef bool (*CheckRTTIFn)(void *, void *);
    CheckRTTIFn checkRTTI = (CheckRTTIFn)(*(u64 *)(*(u64 *)other + 192));

    if (checkRTTI(other, Game::Player::sRTTI)) {
        interactColPlayer_((Game::Player *)other);
    } else if (checkRTTI(other, Game::Bullet::sRTTI)) {
        interactColBullet_((Game::Bullet *)other);
    }
}

void EnemyBall::interactColPlayer_(Game::Player *player) {
    if (!mPlayerHitTimer) {
        Game::EnemyFunctions::calcBallImpactArg impactArg;
        impactArg.playerPos = &player->mPosition;
        impactArg.enemyPos = &getKdAttT()->mPos;
        impactArg.velocity = &mMovable->mVelocity;
        impactArg.impactToPlayer = getFamilyParams()->mImpactToPlayer.mValue;
        impactArg.impactToPlayerX = getFamilyParams()->mImpactToPlayerX.mValue;

        sead::Vector3<float> impact = Game::EnemyFunctions::calcBallImpact(impactArg);
        PlayerInformImpact(player, &impact, 1, 1.0f, 1.0f);
    }

    mPlayerHitTimer = 10;
    if (mDrownState)
        return;

    float vx = mMovable->mVelocity.mX;
    float vy = mMovable->mVelocity.mY;
    float vz = mMovable->mVelocity.mZ;
    float speed = sqrtf(vx * vx + vy * vy + vz * vz);
    if (speed > 0.0f) {
        vx /= speed;
        vy /= speed;
        vz /= speed;
    }

    float rx = vx - ((vz * 1.0f) - (vy * 0.0f));
    float ry = vy - ((vx * 0.0f) - (vz * 0.0f));
    float rz = vz - ((vy * 0.0f) - (vx * 1.0f));
    float rlen = sqrtf(rx * rx + ry * ry + rz * rz);
    if (rlen > 0.0f) {
        rx /= rlen;
        ry /= rlen;
        rz /= rlen;
    }

    if (rz != 0.0f || ry != 0.0f || rx != 0.0f) {
        mEnemyPhysics->mFrontDir.mX = rx;
        mEnemyPhysics->mFrontDir.mY = ry;
        mEnemyPhysics->mFrontDir.mZ = rz;
        mMovable->mVelocity.mX = speed * rx;
        mMovable->mVelocity.mY = speed * ry;
        mMovable->mVelocity.mZ = speed * rz;
    }
}

void EnemyBall::interactColBullet_(Game::Bullet *bullet) {
    if (bullet->mTeam == Cmn::Def::Team::Bravo)
        return;
    if (mDrownState)
        return;

    if (getFamilyParams()->mMutekiBulletIgnore.mValue
        && getFamilyParams()->mMuteki.mValue
        && mMutekiState != 0
        && (!getFamilyParams()->mIsMutekiChanceChanging.mValue || mMutekiState != 3)
        && (!getFamilyParams()->mIsMutekiNormalChanging.mValue || mMutekiState != 2))
        return;

    paintHitBullet_();
    mBulletKnockbackable->onInteractColBullet(bullet, getKdAttT()->mPos, mSinkable->mIsDamaged);
    mAnimSetController->start("Back");

    mIsReturning = 0;
    float vx = mMovable->mVelocity.mX;
    float vy = mMovable->mVelocity.mY;
    float vz = mMovable->mVelocity.mZ;
    float speed = sqrtf(vx * vx + vy * vy + vz * vz);
    if (speed > 0.0f) {
        vx /= speed;
        vy /= speed;
        vz /= speed;
    }

    float crossX = (vz * 1.0f) - (vy * 0.0f);
    float crossY = (vx * 0.0f) - (vz * 0.0f);
    float crossZ = (vy * 0.0f) - (vx * 1.0f);

    typedef float* (*GetBulletVelFn)(void*);
    float *bulletVel = ((GetBulletVelFn)(*(u64*)(*(u64*)bullet + 680)))(bullet);
    float bvx = bulletVel[0];
    float bvz = bulletVel[2];
    float bvLen = sqrtf(bvx * bvx + bvz * bvz);
    if (bvLen > 0.0f) {
        float inv = 1.0f / bvLen;
        bvx *= inv;
        bvz *= inv;
    }

    float dotCrossBullet = (crossX * bvx) + (crossY * 0.0f) + (crossZ * bvz);
    if (dotCrossBullet > 0.0f) {
        crossX = -crossX;
        crossY = -crossY;
        crossZ = -crossZ;
    }

    float impactX = getFamilyParams()->mImpactBulletX.mValue;
    float newX = vx + impactX * crossX;
    float newY = vy + impactX * crossY;
    float newZ = vz + impactX * crossZ;
    float newLen = sqrtf(newX * newX + newY * newY + newZ * newZ);
    if (newLen > 0.0f) {
        newX /= newLen;
        newY /= newLen;
        newZ /= newLen;
    }

    if (newZ != 0.0f || newY != 0.0f || newX != 0.0f) {
        mEnemyPhysics->mFrontDir.mX = newX;
        mEnemyPhysics->mFrontDir.mY = newY;
        mEnemyPhysics->mFrontDir.mZ = newZ;
        mMovable->mVelocity.mX = speed * newX;
        mMovable->mVelocity.mY = speed * newY;
        mMovable->mVelocity.mZ = speed * newZ;
        Game::EnemyFunctions::rotateByFrontDir(
            getKdAttT(), mEnemyPhysics->mFrontDir);
    }
}

void EnemyBall::paintHitBullet_() {
    float bulletPaintRad = getFamilyParams()->mBulletHitPaintRadius.mValue;
    if (bulletPaintRad <= 0.0f)
        return;

    sead::Vector3<float> *nrmGnd = mEnemyPhysics->getNrmGnd();
    Cmn::KDUtl::AttT *att = getKdAttT();
    Game::PaintUtl::requestColAndPaint(
        &att->mPos,
        bulletPaintRad,
        nrmGnd,
        (int)mTeam,
        1,
        -1,
        -1.0f,
        false);
}

void EnemyBall::setDirectionToPhysics_() {
    sead::Vector3<float> *physFrontDir = &mEnemyPhysics->mFrontDir;
    Cmn::KDUtl::AttT *att = getKdAttT();
    att->mtx02 = physFrontDir->mX;
    att->mtx12 = physFrontDir->mY;
    att->mtx22 = physFrontDir->mZ;
    att->mtx01 = 0.0f;
    att->mtx11 = 1.0f;
    att->mtx21 = 0.0f;
    att->normalize_YX_Debug("GameEnemyBall.cpp", 839);
}

float EnemyBall::getPlayerDamage() const {
    if ((mStateMachine.mStateId & 0xFFFFFFFE) != 2)
        return 0.0f;
    if (mMutekiState == 1)
        return 0.0f;
    if (mMutekiState == 3)
        return 0.0f;
    return (float)getFamilyParams()->mDamagePlayer.mValue;
}

void EnemyBall::calcTrackPaintable_() {
    if (!mTrackPaintable)
        return;

    mTrackPaintable->mIsEnabled = (mStateMachine.mStateId != 0) && ((int)mHitKeepOutTimer < 1);

    bool isSloped = false;
    if (mEnemyPhysics->isHitGnd()) {
        sead::Vector3<float> *nrmGnd = mEnemyPhysics->getNrmGnd();
        float dotUp = Cmn::KDUtl::Math::calcAngleAbs(*nrmGnd, sead::Vector3<float>::ey);
        isSloped = dotUp > 0.05236f;
    }

    Cmn::KDUtl::AttT *att = getKdAttT();
    sead::Vector3<float> *physFrontDir = &mEnemyPhysics->mFrontDir;
    float offset = getFamilyParams()->mTrackPaintableOffsetZ.mValue;
    float px = (offset * physFrontDir->mX) + att->mPos.mX;
    float py = (offset * physFrontDir->mY) + att->mPos.mY;
    float pz = (offset * physFrontDir->mZ) + att->mPos.mZ;
    if (isSloped)
        py += 5.0f;

    mTrackPaintable->mPos.mX = px;
    mTrackPaintable->mPos.mY = py;
    mTrackPaintable->mPos.mZ = pz;

    sead::Vector3<float> *nrmGnd = mEnemyPhysics->getNrmGnd();
    mTrackPaintable->mNrm = *nrmGnd;
}

void EnemyBall::setObjColMuteki_() {
    Game::Cmp::EnemyObjCollision *objCol = mEnemyPhysics->mObjCollision;
    u32 colNodeNum = objCol->getColNodeNum();
    if (!colNodeNum)
        return;

    int colType;
    if (getFamilyParams()->mMuteki.mValue
        && (mMutekiState != 0)
        && (!getFamilyParams()->mIsMutekiChanceChanging.mValue || mMutekiState != 3)
        && (!getFamilyParams()->mIsMutekiNormalChanging.mValue || mMutekiState != 2)) {
        colType = 1;
    } else {
        colType = 2;
    }

    *(int*)(*(u64*)((char*)objCol + 40) + 116) = colType;
}

int EnemyBall::setXLinkLocalPropertyDefinition_(int idx) {
    int nextIdx = Game::EnemyBase::setXLinkLocalPropertyDefinition_(idx);
    static xlink2::PropertyDefinition sVelocityProp;
    static bool sInitProp = false;
    if (!sInitProp) {
        sVelocityProp.mName = "Velocity";
        sVelocityProp.mType = 2;
        sVelocityProp.mIsGlobal = false;
        sVelocityProp.mRangeF[0] = 0.0f;
        sVelocityProp.mRangeF[1] = 100.0f;
        sInitProp = true;
    }
    Lp::Sys::XLinkIUser::pushLocalPropertyDefinition(&sVelocityProp);
    return nextIdx + 1;
}

// ====================================================================
// Reimplementations of functions inlined/removed in 5.5.2
// ====================================================================

bool EnemyFunctions::existGndCheck(const sead::Vector3<float> &pos, float radius, float distance) {
    Cmn::KDGndCol::CheckIF checker(nullptr);
    sead::Vector3<float> dir;
    dir.mX = 0.0f;
    dir.mY = -1.0f;
    dir.mZ = 0.0f;
    return checker.checkMoveSphere(pos, dir, distance, radius, 1, 255,
                                   Cmn::KDGndCol::Manager::cWallNrmY_L, 1.0f) & 1;
}

bool EnemyBase::checkExistGnd_(float dist) const {
    sead::Vector3<float> pos = mPos;
    float checkDist = dist > 0.0f ? dist : 200.0f;
    return EnemyFunctions::existGndCheck(pos, 5.0f, checkDist);
}

} // namespace Game

namespace Game { namespace Cmp {

void Rollable::calc(const Rollable::CalcArg &arg) {
    float vx = arg.mVelocity.mX;
    float vy = arg.mVelocity.mY;
    float vz = arg.mVelocity.mZ;

    if (arg.mMode != 1 && arg.mMode != 3)
        vy = 0.0f;

    float speed = sqrtf(vx * vx + vy * vy + vz * vz);
    if (speed > 0.0f) {
        float inv = 1.0f / speed;
        vx *= inv;
        vy *= inv;
        vz *= inv;
    }

    if (arg.mMode == 0 || arg.mMode == 1) {
        if (speed <= 0.0001f)
            return;
    }

    switch (arg.mMode) {
    case 0:
    case 1:
        mRollAngle = speed / mRadius;
        break;
    case 2:
    case 3:
        mRollAngle = (speed * arg.mFactor) / mRadius;
        break;
    case 5:
        mRollAngle *= arg.mFactor;
        if ((mRollAngle < 0.0f ? -mRollAngle : mRollAngle) < 0.0001f)
            mRollAngle = 0.0f;
        break;
    default:
        break;
    }

    float ax = vz;
    float ay = 0.0f;
    float az = -vx;

    float axisLen = sqrtf(ax * ax + ay * ay + az * az);
    sead::Vector3<float> axis;
    axis.mX = ax;
    axis.mY = ay;
    axis.mZ = az;

    if (axisLen > 0.0f) {
        float inv = 1.0f / axisLen;
        axis.mX *= inv;
        axis.mY *= inv;
        axis.mZ *= inv;
    }

    if (axisLen < 1.192e-7f)
        return;

    float angle;
    if (arg.mMode == 6)
        angle = arg.mExtraValue;
    else
        angle = mRollAngle;

    calcRollAtt_(angle, axis);
}

}} // namespace Game::Cmp

void Game::EnemyFunctions::wallHitReflect(const wallHitReflectArg &arg) {
    float vx = arg.inVelocity->mX;
    float vy = arg.inVelocity->mY;
    float vz = arg.inVelocity->mZ;

    float speed = sqrtf(vx * vx + vy * vy + vz * vz);
    float xzLen = sqrtf(vx * vx + vz * vz);

    float dirX = -vx;
    float dirY = 0.0f;
    float dirZ = -vz;

    if (xzLen > 0.0f) {
        float inv = 1.0f / xzLen;
        dirX *= inv;
        dirZ *= inv;
    }

    float nrmX = arg.wallNormal->mX;
    float nrmZ = arg.wallNormal->mZ;
    float cross = dirX * nrmZ - dirZ * nrmX;
    float dot = dirZ * nrmZ + dirX * nrmX;
    float alpha = atan2f(cross, dot);

    float theta = -2.0f * alpha;
    float cosT = cosf(theta);
    float sinT = sinf(theta);

    float refX = dirX * cosT + dirZ * sinT;
    float refY = 0.0f;
    float refZ = dirZ * cosT - dirX * sinT;

    float refLen = sqrtf(refX * refX + refY * refY + refZ * refZ);
    if (refLen > 0.0f) {
        float inv = 1.0f / refLen;
        refX *= inv;
        refY *= inv;
        refZ *= inv;
    }

    if (refX != 0.0f || refY != 0.0f || refZ != 0.0f) {
        arg.outVelocity->mX = speed * refX;
        arg.outVelocity->mY = speed * refY;
        arg.outVelocity->mZ = speed * refZ;

        if (arg.physics) {
            Game::Cmp::EnemyPhysics *phys = (Game::Cmp::EnemyPhysics*)arg.physics;
            phys->mFrontDir.mX = refX;            
            phys->mFrontDir.mY = refY;
            phys->mFrontDir.mZ = refZ;
        }
    }
}
