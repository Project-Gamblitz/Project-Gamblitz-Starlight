#pragma once

#include "GfxUtl.h"
#include "Cmn/Def/PlayerModelType.h"
#include "Game/PlayerModelResource.h"
#include "Lp/Utl.h"
#include "sead/ringbuffer.h"

namespace Cmn{
    class PlayerCustomUtl{
        public:
        static gsys::Model *createPlayerModelHuman_AllHair(sead::SafeStringBase<char> const&name, Cmn::Def::Team team, Game::PlayerModelResource const& res, Lp::Utl::ModelCreateArg const& createArg, Lp::Utl::AnimCreateArg const& animCreateArg, Cmn::Def::PlayerModelType modelType);
        static gsys::Model * createPlayerModelHuman(sead::SafeStringBase<char> const&,Cmn::Def::Team,Game::PlayerModelResource const&,Lp::Utl::ModelCreateArg const&,Lp::Utl::AnimCreateArg const&,Cmn::Def::PlayerModelType,sead::RingBuffer<int> const&);
        static void applyEyeColorAnim(gsys::Model *,int,int);
        static void applySkinColorAnim(gsys::Model *,int,int);
        static void changeHair(gsys::Model *,Cmn::Def::PlayerModelType,int);
        static void applyShoesSoleThickness(Cmn::IPlayerCustomInfo *);
    };
};