#pragma once

#include "types.h"
#include "Game/RefereeVersusGachi.h"
#include "Game/Gachihoko.h"

namespace Game {
	class SeqMgrVersus;
	class RefereeVersusVGoal : public RefereeVersusGachi {
	public:
		RefereeVersusVGoal(Game::SeqMgrVersus *);
		void preStateCalc_InGame();
		void postStateUpdateLyt_InGame();
    };
};