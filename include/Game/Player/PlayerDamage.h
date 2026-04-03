#pragma once

#include "types.h"
#include "Game/DamageReason.h"

namespace Game {
    class PlayerDamage {
		public:
        _BYTE _0[0x30];
        u32 mDamage;

        static int informDamage(void *playerDamage, int attackerIdx, Cmn::Def::DMG dmg, const DamageReason &reason, bool flag);
    };
}