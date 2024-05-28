#pragma once

#include "types.h"
#include "Game/PreScoreVersus.h"


namespace Game {
    class RefereeVersus{
	public:
    _BYTE _0[0x50];
    Game::PreScoreVersus *mPreScoreVersus;
    _BYTE _58[0xC8];
    };
};