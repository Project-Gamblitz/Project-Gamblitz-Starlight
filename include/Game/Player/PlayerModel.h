#pragma once

#include "types.h"

#include "Game/Player/Player.h"
#include "Cmn/PlayerCustomMgr.h"
#include "gsys/model.h"
#include "flexlion/ProcessMemory.hpp"

#ifndef GAME_PLAYERMODEL_H
#define GAME_PLAYERMODEL_H

namespace Cmn{
    class MaterialParamAccessKeySet{
        public:
        MaterialParamAccessKeySet(){
            _0 = 0;
            _8 = 0;
        }
        void setup(gsys::Model const&,sead::SafeStringBase<char> const&,sead::Heap *);
        u64 _0;
        u64 _8;
    };
};
namespace Game {

    class Player;
    class PlayerMgr;
    class PlayerModel {
        public:
        void setup();
        void flushStainColor();
        void setEnableDepthSilhouette(bool);
        void setDepthSilhouetteColor(sead::Color4f const&);
        void update_AfterModelUpdateWorldMatrix_HumanLike();
        
        Game::Player* mPlayer;
        Game::PlayerMgr* mPlayerMgr;
        Cmn::PlayerCustomMgr *mPlayerCustomMgr; // Cmn::PlayerCustomMgr *
        gsys::Model **mFullModel;
        gsys::Model *mHalfModel;
        gsys::Model *mSquidModel;
        _BYTE _30[0x58];
        gsys::ModelOcclusionQuery *mOcclusionQuery;
        float mExternalScale;
        _BYTE _94[0xC];
        Cmn::MaterialParamAccessKeySet mFullModelDepthSilhoutteAccessKey;
        Cmn::MaterialParamAccessKeySet mHalfModelDepthSilhoutteAccessKey;
        Cmn::MaterialParamAccessKeySet mSquidModelDepthSilhoutteAccessKey;
    };
    class SquidModelCallback : public gsys::IModelCallback{
        public:
        SquidModelCallback(Game::PlayerModel *pmodel){
            _0 = ProcessMemory::MainAddr(0x2C10430 + 0x10);
            _8 = (u64)(*pmodel->mFullModel);
            _10 = (u64)pmodel->mHalfModel;
            _18 = *((u64*)(((u64)pmodel->mPlayer) + (0xFC8)));
            _20 = *((u64*)(((u64)pmodel->mPlayer) + (0x1038)));
            _28 = (u64)pmodel;
        };
    };
};

#endif