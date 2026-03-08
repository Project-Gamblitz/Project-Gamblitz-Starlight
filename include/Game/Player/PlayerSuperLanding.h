#pragma once

#include "types.h"
#include "sead/vector.h"

namespace Game{
    class PlayerSuperLanding{
        public:
        void calcLandingPos();
        _BYTE _0[0x84];
        float mLandingDist;
        _BYTE _88[0x4];
        sead::Vector3<float> mLandingPos;
    };
};