#pragma once

#include "Game/Player/Player.h"
#include "Cmn/GfxUtl.h"
#include "Prot/Prot.h"
#include "Game/BulletMgr.h"
#include "starlight/collector.hpp"

namespace Starlion{
    class PlayerKingSquid : public Game::PlayerKingSquid{
        public:
        PlayerKingSquid(Game::Player *player);
        void calc();
        void enter();
        void setupModel();
        Game::Player *mPlayer;
        Game::PlayerModel *mPlayerModel;
        Game::PlayerMotion *mPlayerMotion;
        Game::PlayerJointSquid *mPlayerJointKingSquid;
        gsys::Model *mKingSquidModel;
        Game::PlayerAnimCtrlSet *mKingSquidAnim;
        Game::AnimSetController *mKingSquidAnimCtrl;
        Game::PlayerJointSquid *mPlayerJointSquid;
        gsys::Model *mModel;
        Game::PlayerAnimCtrlSet *mSquidAnim;
        Game::AnimSetController *mSquidAnimCtrl;
        Cmn::MaterialParamAccessKeySet mKingSquidMat;
        Cmn::MaterialParamAccessKeySet mSquidMat;
        Game::BulletSpAquaBall *mBullet;
        Cmn::EffectManualHandle *mEffectManualHandle;
        bool mWasInSquid;
        //int mRushEndFrame;
        int mAerialFrm;
        bool mIsRush;
        bool mEntered;
    };
    class KingSquidMgr{
        public:
        static KingSquidMgr *sInstance;
        KingSquidMgr();
        void onCalc();
        Lp::Sys::ModelArc *getArc(Starlion::PlayerKingSquid *kingSquid);
        Lp::Sys::ModelArc *mKingSquidArcs[2];
        PlayerKingSquid *mPlayerKingSquid[10];
    };
};