#pragma once

#include "Utils.hpp"
#include "Cmn/PlayerWeapon.h"
#include "Prot/Prot.h"

#define SUPERSHOT_SPECIAL_ID 24

namespace Game {
    class PlayerWeaponSuperShot {
    public:
        static PlayerWeaponSuperShot *sInstance;
        static constexpr int DAMAGE = 1500;

        PlayerWeaponSuperShot();

        // Called once after mush data is loaded to patch vtable pointers
        void initialize();

        // Per-frame update for controlled player (charge state forcing, bullet iteration)
        void onCalc();

        // Per-player first calc hook (xlink setup, PutBack/FireImpact triggers)
        void playerFirstCalc(Game::Player *player);

        // Hook callbacks for BulletGachihoko behavior
        static int getBurstWaitFrame(Game::BulletGachihoko *bullet);
        static int getBurstWarnFrame(Game::BulletGachihoko *bullet);
        static int calcHokoDamage(Game::BulletGachihoko *bullet, int armortype, Cmn::Def::Team team, sead::Vector3<float> const& pos);

        // Vtable patch: sets xlink name to SuperShot and prevents setFromMush overwrite
        static Cmn::PlayerWeapon* initWeaponXLink(Cmn::PlayerWeapon *weapon);

        // ASM hook for weapon creation jump table
        static void supershotJumpHook();

    private:
        bool mXlinkSet[10];
        bool mFiredBullet[10];
        bool mInitialized;
    };
}
