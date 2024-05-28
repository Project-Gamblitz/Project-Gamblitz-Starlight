#pragma once

#include "types.h"

#include "sead/heap.h"
#include "Cmn/Def/Team.h"
#include "Cmn/Actor.h"
#include "Game/Player/Player.h"
#include "Game/MainMgr.h"
namespace Game{
    class BulletSpWaterCutter : public Cmn::Actor{
        public:
        void finish();
        void setEnable(bool);
        void set(sead::Vector3<float> const&,sead::Vector3<float> const&, float);
        void setRemainFrame(int);
        void initializeSender(Cmn::Actor const*,int,int,sead::Vector3<float> const&,sead::Vector3<float> const&,unsigned int);
        void start(Game::Player *player, sead::Vector3<float> from, sead::Vector3<float> to){
            Game::MainMgr *mainMgr = Game::MainMgr::sInstance;
            int frm;
            if(mainMgr == NULL){
                frm = 0;
            } else{
                frm = mainMgr->mPaintGameFrame;
            }
            this->initializeSender(player, player->mIndex, player->mIndex, from, to, frm);
        }
        _BYTE _348[0x118];
        u32 mV1;
        _BYTE _464[0xC];
        u32 mV2;
    };
};