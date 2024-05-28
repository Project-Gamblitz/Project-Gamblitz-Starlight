#pragma once

#include "types.h"
#include "Cmn/Actor.h"
#include "Game/Obj.h"

namespace Game {
    class Gachihoko : public Game::MapObjBase
	{
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
		void calcBreakBarrier_();
    };
};