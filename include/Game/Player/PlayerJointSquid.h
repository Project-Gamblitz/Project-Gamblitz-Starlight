#pragma once

#include "types.h"
#include "Game/AnimSetController.h"

namespace Cmn{
    class PlayerInfo;
};
namespace Game{
    class Player;
    class PlayerJoint{
        public:
        u64 _0;
        Game::Player *mPlayer;
    };
    class PlayerJointSquid : public Game::PlayerJoint{
        public:
        PlayerJointSquid(const Game::Player *);
        void reset();
        _BYTE _10[0x1F0];
    };
};