#pragma once

#include "types.h"
#include "Cmn/SceneMgrFader.h"

namespace Lp {
    namespace Sys {
        class ProcessSwitch{
            public:
            static Lp::Sys::ProcessSwitch *sInstance;
            _BYTE _0[0x130];
            bool mIsDocked;
        };
    };
};