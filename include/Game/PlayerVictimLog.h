#pragma once

#include "types.h"

#include "sead/heap.h"
#include "Cmn/Def/Team.h"
#include "Cmn/Actor.h"
#include "Game/Player/Player.h"

namespace Game{
    class PlayerVictimLog{
        public:
        bool checkVictimizer(unsigned int, int) const;
        Game::Player *mPlayer;
    };
};