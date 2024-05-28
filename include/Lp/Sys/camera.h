#pragma once

#include "types.h"
#include "sead/camera.h"

namespace Lp{
    namespace Sys{
        class DbgCamera{
            public:
            void calc(bool);
            _BYTE _0[0x18];
            sead::Camera cam;
        };
        class DbgCameraMgr{
            public:
            static Lp::Sys::DbgCameraMgr *sInstance;
            sead::Camera *getRenderCamera_(int) const;
        };
    }
};