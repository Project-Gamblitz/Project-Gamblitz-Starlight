#pragma once

#include "types.h"
#include "Cmn/Def/Team.h"

namespace Game{
    class PreScoreVersus{
        public:
        float getPaintRatio_Team_InGame(Cmn::Def::Team) const;
    };
};