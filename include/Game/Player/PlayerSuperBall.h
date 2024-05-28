#pragma once
#include "Cmn/Actor.h"
#include "Game/BulletSpSuperBall.h"

namespace Game{
    class PlayerSuperBall{
        public:
        _BYTE _0[0x30];
        Game::BulletSpSuperBall *mBullet;
        void reset();
    };
};