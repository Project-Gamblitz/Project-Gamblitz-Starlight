#pragma once

#include "types.h"

namespace Game {
    class Player;
	class PlayerShotGuide{
        public:
        Game::Player *mPlayer;
        bool mShow;
    };
};