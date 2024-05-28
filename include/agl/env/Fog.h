#pragma once

#include "types.h"
#include "sead/vector.h"
#include "sead/color.h"
#include "agl/env/EnvObj.h"

namespace agl {
    namespace env {
        class Fog : public agl::env::EnvObj{
            public:
            _BYTE _0[0x188];
            sead::Color4f mColor;
        };
    };
};