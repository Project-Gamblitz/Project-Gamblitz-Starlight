#pragma once

#include "types.h"
#include "Def/Team.h"
#include "Def/Mode.h"

namespace Cmn
{
    class PlayerInfo;
    class PlayerInfoUtil {
        public:
        static void setPlayerInfoAgentThree(Cmn::PlayerInfo *a1, Cmn::Def::Team a2);
        static void setPlayerInfoByDummy(Cmn::PlayerInfo *a1, Cmn::Def::Mode a2);
        static void setPlayerInfoBySaveData(Cmn::PlayerInfo *a1, Cmn::Def::Mode a2);
        static void overwriteWeaponGear_Show_ (Cmn::Def::Mode);
    };
};