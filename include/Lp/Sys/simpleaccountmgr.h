#pragma once

#include "types.h"
#include "sead/string.h"
#include "nn/account.h"


namespace Lp {
    namespace Sys{
        class SimpleAccountMgr{
            public:
            static Lp::Sys::SimpleAccountMgr *sInstance;
            _BYTE _0[0x118]; // 178
            nn::account::Uid mUid;
            _BYTE _128[0x50];
            sead::SafeStringBase<char16_t> mAccountName;
        };
    };
};