#include "Game/EnemyBall.h"

static bool initVtable = 0;
Game::EnemyBaseVtable cstmVtable;
void initEnmBallVtable(Game::EnemyBall *enm){
    Game::EnemyBaseVtable **vtable = enm->getEnmVtable();
    if(!initVtable){
        memcpy(&cstmVtable, *vtable, sizeof(Game::EnemyBaseVtable));
        //cstmVtable.actordtor0 = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::~EnemyBall); == RET
        //cstmVtable.actordtor2 = (u64)&Game::EnemyBall::destroy; == delete
        cstmVtable.getClassName = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::getClassName);
        //cstmVtable.checkDerivedRuntimeTypeInfo = (u64)((void (*)(Game::EnemyBall *, sead::RuntimeTypeInfo::Interface const *))&Game::EnemyBall::checkDerivedRuntimeTypeInfo);
        //cstmVtable.getRuntimeTypeInfo = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::getRuntimeTypeInfo);
        //cstmVtable.getClassIterNode = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::getClassIterNode);
        cstmVtable.setXlinkLocalPropertyDefinition = (u64)((void (*)(Game::EnemyBall *, int))&Game::EnemyBall::setXLinkLocalPropertyDefinition_);
        cstmVtable.countXlinkLocalProperty = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::countXLinkLocalProperty_);
        cstmVtable.executeStateMachine = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::executeStateMachine_);
        cstmVtable.changeFirstState = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::changeFirstState_);
        cstmVtable.load_ = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::load_);
        cstmVtable.reset_ = (u64)((void (*)(Game::EnemyBall *, Cmn::Def::ResetType))&Game::EnemyBall::reset_);
        cstmVtable.firstCalc_ = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::firstCalc_);
        cstmVtable.createDesignerParams = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::createDesignerParams);
        cstmVtable.isCreateDesignerParamsImpl_ = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::isCreateDesignerParamsImpl_);
        cstmVtable.createMergedParams_ = (u64)((Game::EnemyBallParams (*)(Game::EnemyBall *))&Game::EnemyBall::createMergedParams_);
        cstmVtable.getPlayerDamage = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::getPlayerDamage);
        cstmVtable.interactCol = (u64)((void (*)(Game::EnemyBall *, Game::CollisionEventArg const&))&Game::EnemyBall::interactCol);
        cstmVtable.createEnemyParams_ = (u64)((void (*)(Game::EnemyBall *))&Game::EnemyBall::createEnemyParams_);
        //cstmVtable.getSpeed_ = (u64)((void (*)(Game::EnemyBall *, Game::CollisionEventArg const&))&Game::EnemyBall::getSpeed_); idk its gone
        initVtable = 1;
    }
    *vtable - &cstmVtable;
}

namespace Game{
EnemyBall::EnemyBall(){
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
}
EnemyBall::~EnemyBall(){

}
void EnemyBall::destroy(Game::EnemyBase *enm){
    delete enm;
}
void EnemyBall::load_(){
    mMovable = new Cmp::Movable(mComponentHolder);
    mFrontDirRotatable = new Cmp::FrontDirRotatable(mComponentHolder, Cmp::FrontDirRotatable::Interp::Enabled);
    mRollable = new Cmp::Rollable(mComponentHolder, getKdAttT(), 1.0f, mModelCB); // 1.0f is some param, ill add them later
    mBulletKnockbackable = new Cmp::BulletKnockbackable(mComponentHolder, NULL); // TODO: impl params

}
const char *EnemyBall::getClassName() const{
    return "EnemyBall";
}
void EnemyBall::stateEnterWait_(){
    mAnimSetController->start("Wait");
    Cmn::GfxUtl::setTwoColorCompTeam(mActorFullModel, Cmn::Def::Team::Bravo);
}
void EnemyBall::stateWait_(){
    Game::EnemyFunctions::rotateByFrontDir(getKdAttT(), mEnemyPhysics->mObjCollision->_110);
}
void EnemyBall::stateEnterNotice_(){
    mAnimSetController->start("Find");
}
void EnemyBall::stateNotice_(){
    Game::EnemyFunctions::rotateByFrontDir(getKdAttT(), mEnemyPhysics->mObjCollision->_110);
}
void EnemyBall::stateEnterMove_(){
    
}
void EnemyBall::stateMove_(){
    
}
void EnemyBall::stateEnterDie_(){
    
}
void EnemyBall::stateDie_(){
    
}
EnemyBallParams *EnemyBall::createMergedParams_(){
    return new EnemyBallParams();
}
void EnemyBall::firstCalc_(){
    this->calcTrackPaintable_();
    this->calcRollable_();
    this->calcFDRotatable_();
    this->calcChangeState_();
}
float EnemyBall::getSpeed_() const{
    return 1.0f; // TODO: Params
}
}