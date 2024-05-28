#pragma once

#include "types.h"
#include "Game/DamageReason.h"
#include "Game/Player/Player.h"


namespace Game {
    class VersusBeatenPage {
        public:
        void start(Game::DamageReason const&, Cmn::PlayerInfo const*, int, int, bool);
    };
};