#pragma once

#include "types.h"
#include "Cmn/Actor.h"
#include "Game/SeqMgrBase.h"
#include "Game/RefereeVersus.h"

namespace Game {
	class ScoreVersus{
		public:
		Cmn::Def::Team getWinningTeam() const; // sig = (08 50 40 39 08 01 00 52 00 1D 40 92 FD 7B C1 A8) - 0x18
	};
	class SeqVersusResult : public Cmn::Actor {
		public:
		_BYTE _348[0x3A8 - 0x348];
		Game::ScoreVersus *mScoreVersus;
	};
    class SeqMgrVersus : public Cmn::Actor {
	public:
		int convCloneClockToGameFrame(unsigned int) const;
		_BYTE _348[0xE8];
		Game::SeqVersusResult *mSeqVersusResult;
		u64 _438;
		Game::RefereeVersus* mRefereeVersus;
    };
};