#pragma once

#include "types.h"
#include "Cmn/Actor.h"
#include "Lp/Sys/iusehionode.h"
#include "Game/Player/Player.h"

namespace Game {
    class FixedAimMode{
        public:
        FixedAimMode(Lp::Sys::IHioNode *);
        void enterAim_();
        void calcAim_();
        void exitAim_();
        void calcAimMtx_(sead::Matrix34<float> *, Cmn::Actor const*) const;
        void resetCameraAt_();
        _BYTE _0[0x8];
        Game::Player *aimAt;
        _BYTE _10[0x78]; // F0
        sead::Vector3<float> at;
        _BYTE _98[0x6C];
    };
	
};