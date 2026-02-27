#pragma once

#include "sead/string.h"
#include "types.h"

namespace Cmn {
    namespace Def {
        enum class VersusMode
        {
            Regular = 0x0,
            Gachi = 0x1,
            Fest = 0x2,
            Private = 0x3,
			League = 0x4,
        };
		enum class VersusRule
        {
            Pnt = 0x0,
            Vgl = 0x1,
            Var = 0x2,
            Vlf = 0x3,
			Vcl = 0x4,
        };
        enum Mode {
            cVersus,
            cCoop,
            cMission,
            cPlaza,
            cWorld,
            cTutorial,
            cShootingRange,
            cWalkThrough,
            cTwoShot,
            cStaffRoll, 
            cLobby,
            cMatch, 
            cShop, 
            cCustomize, 
            cPlayerMake, 
            cMissionOcta, 
            cBaseCampOcta, 
            cDepartureOcta, 
            cCentralOcta, 
            cStaffRollOcta, 
            cBoot, 
            cOther, 
            cFreeTest, 
            cDbgSetting, 
            cDevOther
        };
		Cmn::Def::Mode getCurMode();
    };
};