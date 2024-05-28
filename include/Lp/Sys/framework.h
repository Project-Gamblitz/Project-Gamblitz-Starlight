#pragma once

#include "types.h"
#include "sead/framework.h"
#include "worker.h"

namespace Lp {
    namespace Sys{
        class Framework : public sead::GameFrameworkNx{
            public:
            static Lp::Sys::Framework *spInstance;

        };
    };
};