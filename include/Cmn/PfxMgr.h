#pragma once

#include "types.h"
#include "Cmn/DepthOfField.h"

namespace Cmn {
    class PfxMgr {
        public:
        static Cmn::PfxMgr *sInstance;
        _BYTE _0[0x31];
        bool mIsCalcDof;
        bool _32;
        bool _33;
        bool _34;
        bool mIsUseDOF;
        _BYTE _36[0x12];
        Cmn::DepthOfField *mDOF;
    };
};