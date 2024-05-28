#pragma once

#include "types.h"

namespace Lp {
    namespace Sys {
        class DbgMode{
            public:
            static Lp::Sys::DbgMode *sInstance;
            void calcSys();
            void calcScene();
            void entryDraw();
        };
    };
};