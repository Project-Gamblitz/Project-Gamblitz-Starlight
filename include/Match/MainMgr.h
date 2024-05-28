#pragma once

#include "types.h"
#include "Cmn/Actor.h"

namespace Match{
    class VersusPrivateSeq{

    };
    class MainMgr{
        public:
        Cmn::Actor *createSeqActor();
    };
};