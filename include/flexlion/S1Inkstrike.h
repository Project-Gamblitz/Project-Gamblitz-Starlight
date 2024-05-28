#pragma once

#include "types.h"
#include "string"
#include "vector"
#include <functional>
#include "flexlion/Utils.hpp"
#include "flexlion/DrawUtils.hpp"
#include "starlight/collector.hpp"
#include "Cmn/Mush/MushDataHolder.h"
#include "starlight/collector.hpp"
#include "Prot/Prot.h"
#include "Game/BulletMgr.h"
#include "nn/vfx/System.h"
using namespace starlight;

namespace Starlion{
    class S1Inkstrike{
        public:
        S1Inkstrike();
        void linkPlayerWeaponBullet(Game::BulletSpSuperMissile *bullet);
        void onRender();
        void updateSuperball(int id);
        Cmn::PlayerWeapon *weapons[10];
        Game::BulletSpSuperBall *superballs[10];
        Game::BulletSpSuperMissile *bullets[10];
        sead::Matrix34<float> mtxs[10];
        bool isShot;
        bool isBulletActive[10];
    };
};
