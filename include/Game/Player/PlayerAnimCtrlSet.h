#pragma once

#include "types.h"
#include "Game/Player/PlayerJointSquid.h"

namespace Game {
	class PlayerAnimCtrlSet{
		public:
		PlayerAnimCtrlSet(gsys::Model *,bool,Cmn::PlayerInfo const&);
		void setup(Game::PlayerJoint *);
		void registAnim(int,char const*,bool,bool);
		bool isValid(int, int) const;
		_BYTE _0[0x178];
	};
};