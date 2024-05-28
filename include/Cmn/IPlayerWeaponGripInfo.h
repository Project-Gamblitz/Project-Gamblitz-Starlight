#pragma once

#include "Cmn/GfxUtl.h"

namespace Cmn{
    class IPlayerWeaponGripInfoVftable{
        public:
        u64 getSkelAnimKey_PWG;
        u64 getWeapon_PWG;
        u64 canGrip_PWG;
    };
    class IPlayerWeaponGripInfo{
        public:
        IPlayerWeaponGripInfoVftable *vftable;
        u64 *mPlayerPtr;
        _BYTE _10[0x20];
    };
};