#pragma once

#include "types.h"

namespace Lp {
    namespace Sys {
        class PauseMgr{
            public:
            static Lp::Sys::PauseMgr *sInstance;
            bool isCalcScene() const;
            _BYTE _0[0x334];
            int pauseType;
        };
    };
};