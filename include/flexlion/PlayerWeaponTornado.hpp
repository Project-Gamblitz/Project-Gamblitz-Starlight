#pragma once

#include "Utils.hpp"
#include "Cmn/PlayerWeapon.h"
#include "Prot/Prot.h"
#include "flexlion/InkstrikeMgr.hpp"

namespace Game {

    class PlayerWeaponTornado {
    public:
        static PlayerWeaponTornado *sInstance;

        PlayerWeaponTornado();

        void playerFirstCalc(Game::Player *player);

        static Cmn::PlayerWeapon* initWeaponXLink(Cmn::PlayerWeapon *weapon);
        static void tornadoJumpHook();

    private:
        bool mXlinkSet[10];
        bool mOnActivatePlayed[10];
    };
}
