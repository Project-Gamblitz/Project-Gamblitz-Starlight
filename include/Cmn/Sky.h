#pragma once

#include "types.h"
#include "Cmn/Stardust.h"

namespace Cmn{
    class Sky{
        public:
        void initialize(gsys::ModelScene *, sead::Heap *);
        _BYTE _0[0x11F8];
        Cmn::Stardust *mStardust;
    };
};