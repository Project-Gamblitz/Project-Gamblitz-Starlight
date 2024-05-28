#pragma once

#include "types.h"
#include "Cmn/Actor.h"
#include "Cmn/Def/Mode.h"

namespace Cnet{
    class MatchmakeExeBase{
        public:
        void reqAutoMatch();
        void reqCreateMatch();
        _BYTE _0[0x108];
        Cmn::Def::VersusMode mVersusMode;
    };
};