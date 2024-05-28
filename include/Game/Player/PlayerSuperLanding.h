#pragma once

#include "types.h"
#include "sead/vector.h"

namespace Game{
    class PlayerSuperLanding{
        public:
        void calcLandingPos();
        _BYTE _0[0x8C];
        sead::Vector3<float> mLandingPos;
    };
};