#pragma once

#include "types.h"

namespace Game {
	class Player;
	class PlayerTrouble{
		public:
		void informDamageFull_Sender();
		void informAirFall();
		void preReset(bool);
		
		Game::Player *mPlayer;
		_BYTE _8[0x30];
		u32 mPlayerState1;
		u32 mPlayerState2;
		u32 _40;
		u32 mPlayerState3;
		u32 _48;
		u32 mVisibility;
	};
};