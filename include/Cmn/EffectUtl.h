#pragma once

#include "types.h"
#include "nn/vfx/System.h"

namespace Cmn {
    class EffectUtl {
        public:
        static nn::vfx::EmitterSetInfo *getESetInfo(nn::vfx::EmitterSet *);
    };
};