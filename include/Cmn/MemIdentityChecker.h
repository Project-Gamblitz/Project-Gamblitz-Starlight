#pragma once

#include "types.h"
#include "sead/string.h"


namespace Cmn {
    class MemIdentityChecker{
        public:
        static Cmn::MemIdentityChecker *sInstance;
        void calcHash();
        _BYTE _0[0x4038];
        sead::SafeStringBase<char> mHash;
    };
};