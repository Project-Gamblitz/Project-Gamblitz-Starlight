#pragma once

#include "types.h"

namespace sead {
    class DebugMenu{
        public:
        void setEnable(bool);
    };
    class SeadMenuMgr{
        public:
        static sead::SeadMenuMgr *sInstance;

        _BYTE _0[0x228];
        sead::DebugMenu *mDebugMenu;
        _BYTE _230[0x10];
        bool mIsEnabled;
    };
};