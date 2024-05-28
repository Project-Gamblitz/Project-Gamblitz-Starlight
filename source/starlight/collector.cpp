#include "starlight/collector.hpp"

namespace starlight {
    Controller Collector::mController;
    Controller Collector::mControllerDebug;

    // Scene
    Lp::Sys::Scene *Collector::mScene;
    Cmn::GfxMgr *Collector::mGfxMgr;

    // Managers
    Game::MainMgr* Collector::mMainMgrInstance;
    Game::PlayerMgr* Collector::mPlayerMgrInstance;
    Cmn::TeamColorMgr* Collector::mTeamColorMgr;
    sead::HeapMgr* Collector::mHeapMgr;
	Cmn::AppUBOMgr* Collector::mAppUBOMgrInstance;
    Game::EnemyMgr* Collector::mEnemyMgr;
    Game::VictoryClamDirector* Collector::mVictoryClamDirector;
    Cmn::PBRMgr* Collector::mPBRMgr;
    Cmn::CameraAccessor* Collector::mCameraAccessor;
    Lp::Sys::SimpleAccountMgr* Collector::mSimpleAccountMgr;
    Cmn::MemIdentityChecker *Collector::mMemIdentityChecker;
    //agl::utl::ScreenShotMgr *Collector::mScreenShotMgr;
    Game::MapObjMgr *Collector::mMapObjMgr;

    // Utils
    Cmn::StaticMem* Collector::mStaticMemInstance;
    Cmn::GfxSetting* Collector::mGfxSettingInstance;

    // Player Information
    Game::Player* Collector::mControlledPlayer;
    Cmn::PlayerInfo* Collector::mControlledPlayerInfo;
    Cmn::PlayerCtrl* Collector::mPlayerCtrlInstance;
    Cmn::PlayerInfoAry* Collector::mPlayerInfoAry;

    // Mush
    Cmn::MushDataHolder* Collector::mMushDataHolder;
    Cmn::MushWeaponInfo* Collector::mMushWeaponInfo;
    Cmn::MushMapInfo* Collector::mMushMapInfo;

    // Coop
    Game::Coop::Setting* Collector::mCoopSetting;
    Game::Coop::EventDirector* Collector::mCoopEventDirector;
    Game::Coop::PlayerDirector* Collector::mCoopPlayerDirector;
    Game::Coop::EnemyDirector* Collector::mCoopEnemyDirector;

    void Collector::init() {
        
    }

    void Collector::collect(){
        mController.mController = Lp::Utl::getCtrl(0);
        mControllerDebug.mController = Lp::Utl::getCtrl(1);
        mMainMgrInstance = Game::MainMgr::sInstance;
        mStaticMemInstance = Cmn::StaticMem::sInstance;
        mGfxSettingInstance = Cmn::GfxSetting::sInstance;
        mHeapMgr = sead::HeapMgr::sInstance;
		mAppUBOMgrInstance = Cmn::AppUBOMgr::sInstance;
        mPlayerMgrInstance = Game::PlayerMgr::sInstance;
        mPlayerCtrlInstance = Cmn::PlayerCtrl::sInstance;
        mMushDataHolder = Cmn::MushDataHolder::sInstance;
        mEnemyMgr = Game::EnemyMgr::sInstance;
        mPBRMgr = Cmn::PBRMgr::sInstance;
        mCameraAccessor = Cmn::CameraAccessor::sInstance;
        mSimpleAccountMgr = Lp::Sys::SimpleAccountMgr::sInstance;
        mMemIdentityChecker = Cmn::MemIdentityChecker::sInstance;
        mCoopSetting = Cmn::Singleton<Game::Coop::Setting>::GetInstance_();
        mCoopEventDirector = Cmn::Singleton<Game::Coop::EventDirector>::GetInstance_();
        mCoopPlayerDirector = Cmn::Singleton<Game::Coop::PlayerDirector>::GetInstance_();
		mVictoryClamDirector = Cmn::Singleton<Game::VictoryClamDirector>::GetInstance_();
        mCoopEnemyDirector = Cmn::Singleton<Game::Coop::EnemyDirector>::GetInstance_();
        //mScreenShotMgr = agl::utl::ScreenShotMgr::sInstance;
        mMapObjMgr = Game::MapObjMgr::sInstance;
        mScene = Lp::Utl::getCurScene();
        mGfxMgr = Cmn::GfxUtl::getGfxMgr();

        if(mGfxSettingInstance != NULL){
            mTeamColorMgr = mGfxSettingInstance->mTeamColorMgr;
        } else{
            mTeamColorMgr = NULL;
        }
        if(mStaticMemInstance){
            mPlayerInfoAry = mStaticMemInstance->mPlayerInfoAry;
        } else {
            mPlayerInfoAry = NULL;
        }

        if(mPlayerMgrInstance){
            mControlledPlayer = mPlayerMgrInstance->getControlledPerformer();
            if(mControlledPlayer != NULL){
                mControlledPlayerInfo = mControlledPlayer->mPlayerInfo;
            } else{
                mControlledPlayerInfo = NULL;
            }
        } else {
            mControlledPlayer = NULL;
        }
        
        if(mMushDataHolder){
            mMushWeaponInfo = mMushDataHolder->mMushWeaponInfo;
            mMushMapInfo = mMushDataHolder->mMushMapInfo;
        } else {
            mMushWeaponInfo = NULL;
            mMushMapInfo = NULL;
        }
    }
};