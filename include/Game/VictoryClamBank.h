#pragma once

#include "types.h"
#include "Cmn/Actor.h"
#include "Game/Player/Player.h"
#include "Lp/Utl.h"
#include "Game/VictoryClamDirector.h"
#include "sead/vector.h"
#include "Cmn/Def/ResetType.h"
#include "Game/Obj.h"

namespace Game {
    class VictoryClamBankLandPos : public Game::MapObjBase
	{
	public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        Game::VictoryClamBankLandPos *getNextLandPos_WinAlpha();
        Game::VictoryClamBankLandPos *getNextLandPos_WinBravo();
    };
    class VictoryClamBank : public Game::Obj
	{
	public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        void calcPlayerCollision_(Game::Player*);
        void appear();
        void reset_(Cmn::Def::ResetType);
        _BYTE _598[0x5D0 - sizeof(Game::Obj)];
        Game::VictoryClamDirector *mClamDirector;
        _BYTE _5D8[0x40];
        Game::VictoryClamBankLandPos *mLandPos;
        _BYTE _61C[0xC];
        u32 mFrameVariable2;
        u32 _630;
        u32 mBankedFrame;
    };
    class VictoryClamBasket : public Game::MapObjBase{
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
};