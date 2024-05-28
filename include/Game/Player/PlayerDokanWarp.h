#pragma once

#include "types.h"

namespace Game {
	class PlayerDokanWarp{
		public:
		bool isInMove_NoControl(bool) const;

		Game::Player *mPlayer;
	};
};