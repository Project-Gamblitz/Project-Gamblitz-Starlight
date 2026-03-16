#pragma once

#include "types.h"
#include "Cmn/Actor.h"

namespace Game {
	class BulletBombBase : public Cmn::Actor {
		public :
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
};
