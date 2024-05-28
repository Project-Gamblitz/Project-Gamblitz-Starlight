#pragma once

#include "types.h"
#include "Cmn/SceneMgrFader.h"

namespace Lp {
    namespace Sys {
        class SceneMgr{
            public:
            static Lp::Sys::SceneMgr *sInstance;
            _BYTE _0[0x6E0];
            Cmn::SceneMgrFader *mSceneMgrFader;
        };
    };
};