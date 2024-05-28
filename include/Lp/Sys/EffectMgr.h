#pragma once

#include "types.h"
#include "sead/ptcl/PtclSystem.h"

namespace Lp {
    namespace Sys {
        class EffectMgr {
            public:
            static Lp::Sys::EffectMgr *sInstance;
            _BYTE _0[0x338];
            sead::ptcl::PtclSystem *mPtclSystem;
        };
    };
};