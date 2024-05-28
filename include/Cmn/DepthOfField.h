#pragma once

#include "types.h"

namespace Cmn
{
    class DepthOfField{
        public:
        struct LensParam{
            float _0;
            float mLensStrength;
            float mLensStrength2;
            float mLensStrength3;
            float _10;
            float _14;
        };
        void setLensParam(Cmn::DepthOfField::LensParam const&);
        _BYTE _0[0x10];
        Cmn::DepthOfField::LensParam mLensParam;
    };
}; // namespace Cmn
