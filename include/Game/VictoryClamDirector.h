#pragma once

#include "types.h"

#include "sead/heap.h"
#include "Cmn/Def/Team.h"
#include "sead/vector.h"
#include "Cmn/Def/ResetType.h"
#include "Game/Obj.h"

namespace Game{
    class Player;
    class ClamSpawnInfo{
        public:
        ClamSpawnInfo(){
            mTeam = Cmn::Def::Team::Neutral;
            someBoolNotFF = 0;
            notZero = 1;
        }
        void set(){
            mTeam = Cmn::Def::Team::Neutral;
            someBoolNotFF = 0;
            notZero = 1;
        }
        enum ClamType{
            mNormal, mGolden
        };
        _BYTE _0[0x58];
        Cmn::Def::Team mTeam;
        u32 someBoolNotFF;
        bool _5C;
        bool notZero;
        u64 _60;
    };
    class VictoryClam : public Game::MapObjBase{
        public:
        _BYTE _594[0x654 - sizeof(Game::MapObjBase)];  
        bool mIsOnGround;
        _BYTE _655[0x7];
        u32 mDissapearFrame;
        _BYTE _660[0x138];
        bool _798;
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
    class VictoryClamDirector{
        public:
        void load();
        void addHoldingClamGoldenNum_(int, int);
        void addHoldingClamNum_(int, int);
        void getBestCount(Cmn::Def::Team const&) const;
        void informBankDirect(int, int, int);
        void spawnClam_(sead::Vector3<float> const&, sead::Vector3<float> const&, Game::ClamSpawnInfo::ClamType const&, int, Cmn::Def::Team const&, bool, bool);
        void spawnClam(sead::Vector3<float> const&, sead::Vector3<float> const&, Game::ClamSpawnInfo const&);
        _BYTE _0[0x11E8];
        u32 mScoreAlpha;
        u32 mScoreBravo;
        u64 _11F0;
        u64 _11F8;
        _BYTE _1200[0x14];
        u32 mNormalClamNum[0xA];
        u32 mGoldenClamNum[0xA];
    };
    class VictoryClamHolding : public Game::Obj{
        public:
        void setTrackPlayer(Game::Player const*);
        void initialize(Game::VictoryClamDirector *,int,int,bool);
        bool calcSleep_();
        _BYTE _594[0x5D0 - sizeof(Game::Obj)];
        Game::VictoryClamDirector *clamDirector;
        _BYTE _5D8[0x8];
        Game::Player *mTrackPlayer;
        u32 mTrackPlayerId;
    };
    class VictoryClamContainer : public Game::MapObjBase{
        public:
        void initialize(sead::Vector3<float> const&, unsigned int);
        void reset_(Cmn::Def::ResetType);
    };
};