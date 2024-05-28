#pragma once

#include "types.h"
#include "Game/RefereeVersusGachi.h"
#include "Game/Gachihoko.h"

namespace Game {
    class SeqMgrVersus;
	class RefereeVersusVBall : public RefereeVersusGachi {
	public:
        RefereeVersusVBall(Game::SeqMgrVersus *);
		void preStateCalc_InGame();
		void postStateUpdateLyt_InGame();
    };
};