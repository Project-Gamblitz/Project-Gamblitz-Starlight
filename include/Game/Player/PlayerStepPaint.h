#pragma once

#include "types.h"
#include "sead/vector.h"

namespace Game {
	class Player;
	class PlayerStepPaint{
		public:
		void calcStepPaint(sead::Vector3<float> *);
		Game::Player *mPlayer;
		float _8;
		float _0xC;
		u32 _10;
		u32 _14;
		u32 _18;
		u32 _1C;
		u32 _20;
		u32 _24;
		_BYTE _28[0x10C];
		u32 mStepPaintType;
		
	};
};