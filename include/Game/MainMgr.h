#pragma once

#include "types.h"

#include "Cmn/Actor.h"
#include "Game/GfxMgr.h"
#include "Game/Field.h"
#include "Game/SeqMgrBase.h"
#include "Game/SeqMgrVersus.h"
#include "Game/EnemyMgr.h"
#include "Game/PlayerMgr.h"
#include "Game/RivalMgr.h"
#include "Cmn/CameraMgr.h"
#include "Game/HudMgr.h"
#include "Game/MapObjMgr.h"
#include "Game/BulletMgr.h"
#include "Game/Player/PlayerNetControl.h"

namespace Game {
    class MainMgr : public Cmn::Actor, public sead::IDisposer {
        public:
        static Game::MainMgr* sInstance;

        __int64 scene; //Lp::Sys::Scene *scene;
        _QWORD gfxLayer3d;
        _QWORD qword378;
        _QWORD plazaGfxLayer3d;
        __int64 mudata; //Cmn::MuData *mudata;
        Game::SeqMgrBase *seqMgr; //Game::SeqMgrCoop *seqMgr;
        Game::Field* field;
        u64 *mLocatorMgr;
        u64 *mSwitchMgr;
        Game::MapObjMgr *mMapObjMgr;
        Game::EnemyMgr *mEnemyMgr;
        Game::PlayerMgr *mPlayerMgr;
        u64 *mMainWeaponMgr;
        u64 *mSubWeaponMgr;
        u64 *mSpecialWeaponMgr;
        u64 *mOtherWeaponMgr;
        u64 *mItemMgr;
        Game::BulletMgr *mBulletMgr;
        Game::RivalMgr *mRivalMgr;
        u64 *mRivalMgrOcta;
        u64 *mNpcMgr;
        u64 *mSpawnPosMgr;
        Cmn::CameraMgr *mCameraMgr;
        Cmn::CameraMgr *mMinimapCameraMgr;
        Cmn::CameraMgr *mPlazaCameraMgr;
        Game::HudMgr *mHudMgr;
        u64 *mCaptureIconMgr;
        Game::CloneObjMgr *cloneObjMgr; //Game::CloneObjMgr *cloneObjMgr;
        Game::GfxMgr* gfxMgr; //Game::GfxMgr *gfxMgr;
        signed __int32 gap408;
        u32 mPaintGameFrame;
		u32 mGameFrame;
        u32 _45C;
        _QWORD twoShotParamsHolder;
        _BYTE qword420;
        _QWORD qword428;
        _QWORD qword430;
        _QWORD qword438;
        _QWORD qword440;
        _QWORD qword448;
        _QWORD qword450;
        _DWORD dword458;
        _BYTE gap45C[4];
        _QWORD qword460;
        _BYTE byte468;

        unsigned int getPaintGameFrame() const;
        int getGameBigState() const;
    };
};