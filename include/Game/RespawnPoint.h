#pragma once

#include "types.h"
#include "Cmn/Actor.h"
#include "string.h"
#include "Game/Obj.h"

namespace Game {
	class RespawnPoint : public Game::MapObjBase{
		public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
		void calcBulletCollision_();
	};
};