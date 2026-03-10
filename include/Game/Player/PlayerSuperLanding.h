#pragma once

#include "types.h"
#include "sead/vector.h"

namespace Game{
    class PlayerSuperLanding{
        public:
        void calcLandingPos();
        _BYTE _0[0x84];
        float mLandingDist; // 0x84
        int _pad88;         // 0x88
        sead::Vector3<float> mLandingPos; // 0x8C
    };
};