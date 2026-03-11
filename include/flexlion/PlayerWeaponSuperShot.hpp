#pragma once

#include "Utils.hpp"
#include "Cmn/PlayerWeapon.h"
#include "Prot/Prot.h"

#define SUPERSHOT_SPECIAL_ID 24

namespace Game {
    class BulletSuperShot;

    class PlayerWeaponSuperShot {
    public:
        static PlayerWeaponSuperShot *sInstance;
        static constexpr int DAMAGE = 1500;

        PlayerWeaponSuperShot();

        void initialize();
        void onCalc();
        void playerFirstCalc(Game::Player *player);

        static Cmn::PlayerWeapon* initWeaponXLink(Cmn::PlayerWeapon *weapon);
        static void supershotJumpHook();

    private:
        void launchBullet(Game::Player *player);
        void sleepGachihokoBullet(Game::BulletGachihoko *bullet);

        bool mXlinkSet[10];
        bool mFiredBullet[10];
        Game::BulletSuperShot *mBullet[10];
        bool mInitialized;
    };
}
