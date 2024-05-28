#pragma once

#include "types.h"
#include "Game/RefereeVersus.h"
#include "Game/Obj.h"

namespace Game {
    class RefereeVersusGachi : public Game::RefereeVersus{
	public:
    Game::MapObjBase *mGimmickObj;
    _BYTE _128[0x48];
    };
};