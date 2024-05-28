#pragma once

#include "types.h"
#include "nn/hid.h"
#include "sead/task.h"

namespace Cmn {
    class CtrlChecker{
        public:
        static Cmn::CtrlChecker *sInstance;
        CtrlChecker(sead::TaskConstructArg const&);
        void checkSupportApplet();
        void checkConnectNum();
        void calc();
        void startSupportApplet();
        void deleteInstance();

        _BYTE _0[0x248];
        u32 mInSupportAppletConnected;
        u32 mConnectedNum;
        _BYTE _250[0x28];
    };
};