#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/ComponentBase.h"
#include "Cmn/ComponentHolder.h"

namespace Game{
namespace Cmp{
    class FrontDirRotatable : public Cmn::ComponentBase{
        public:
        enum class Interp{
            Disabled, Enabled
        };
        FrontDirRotatable(Cmn::ComponentHolder *, Interp);
        Interp mInterp;
        int _2C;
    };
};
};