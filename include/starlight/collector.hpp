#pragma once

#include "input.hpp"

#include "Game/MainMgr.h"
#include "Game/PlayerMgr.h"
#include "Game/Player/Player.h"
#include "Game/Coop/Setting.h"
#include "Game/Coop/EventDirector.h"
#include "Game/Coop/PlayerDirector.h"
#include "Game/Coop/EnemyDirector.h"
#include "Game/VictoryClamDirector.h"
#include "Cmn/PlayerCtrl.h"
#include "Cmn/AppUBOMgr.h"
#include "Cmn/StaticMem.h"
#include "Cmn/GfxSetting.h"
#include "Cmn/TeamColorMgr.h"
#include "Cmn/Mush/MushDataHolder.h"
#include "Cmn/Mush/MushWeaponInfo.h"
#include "Cmn/Mush/MushMapInfo.h"
#include "Cmn/Singleton.h"
#include "Game/EnemyMgr.h"
#include "Lp/Utl.h"
#include "sead/heap.h"
#include "Cmn/PBRMgr.h"
#include "Cmn/CameraAccessor.h"
#include "Lp/Sys/simpleaccountmgr.h"
#include "Cmn/MemIdentityChecker.h"
#include "agl/utl/screenshotmgr.h"
#include "Scene.h"
#include "Cmn/GfxMgr.h"

namespace starlight
{
    class Collector {
        public:
        static Controller mController;
        static Controller mControllerDebug;

        // Scene
        static Lp::Sys::Scene *mScene;
        static Cmn::GfxMgr *mGfxMgr;

        // Managers
        static Game::MainMgr* mMainMgrInstance;
        static Game::PlayerMgr* mPlayerMgrInstance;
        static Cmn::TeamColorMgr* mTeamColorMgr;
        static sead::HeapMgr* mHeapMgr;
		static Cmn::AppUBOMgr *mAppUBOMgrInstance;
        static Game::VictoryClamDirector *mVictoryClamDirector;
        static Game::EnemyMgr *mEnemyMgr;
        static Cmn::PBRMgr *mPBRMgr;
        static Cmn::CameraAccessor *mCameraAccessor;
        static Lp::Sys::SimpleAccountMgr *mSimpleAccountMgr;
        static Cmn::MemIdentityChecker *mMemIdentityChecker;
       // static agl::utl::ScreenShotMgr *mScreenShotMgr;
        static Game::MapObjMgr *mMapObjMgr;
        
        // Utils
        static Cmn::StaticMem* mStaticMemInstance;
        static Cmn::GfxSetting* mGfxSettingInstance;

        // Player Information
        static Game::Player* mControlledPlayer;
        static Cmn::PlayerInfo* mControlledPlayerInfo;
        static Cmn::PlayerCtrl* mPlayerCtrlInstance;
        static Cmn::PlayerInfoAry* mPlayerInfoAry;

        // Mush
        static Cmn::MushDataHolder* mMushDataHolder;
        static Cmn::MushWeaponInfo* mMushWeaponInfo;
        static Cmn::MushMapInfo* mMushMapInfo;

        // Coop
        static Game::Coop::Setting *mCoopSetting;
        static Game::Coop::EventDirector *mCoopEventDirector;
        static Game::Coop::PlayerDirector *mCoopPlayerDirector;
        static Game::Coop::EnemyDirector *mCoopEnemyDirector;

        static void init();
        static void collect();
    };
}


