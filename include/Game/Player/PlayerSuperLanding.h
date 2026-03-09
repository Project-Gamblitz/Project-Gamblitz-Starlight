#pragma once

#include "types.h"
#include "sead/vector.h"

namespace Game{
    class PlayerSuperLanding{
        public:
        void calcLandingPos();
        _BYTE _0[0x84];
        float mLandingDist;
        int mLandingAttr; // 0x88 — tentative: collision attribute from calcLandingPos hit (verify at runtime)
        sead::Vector3<float> mLandingPos;
    };
};