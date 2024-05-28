#pragma once

#include "types.h"
#include "Cmn/Actor.h"
#include "string.h"
#include "Cmn/MuObj.h"
#include "Game/ActorMgr.h"

namespace Game {
	class EnemyMgr : public Game::ActorMgr{
		public:
        static Game::EnemyMgr *sInstance;
	};
};