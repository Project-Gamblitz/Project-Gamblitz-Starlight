#pragma once

#include "types.h"
#include "Cmn/Sky.h"

namespace Game{
    class SimulationMgr{
        public:
        static Game::SimulationMgr *sInstance;
        _BYTE _0[0x408];
        Cmn::Sky *mSky;
    };
};