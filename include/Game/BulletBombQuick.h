#pragma once

#include "types.h"
#include "Cmn/Actor.h"

namespace Game {
	class BulletBombQuick : public Cmn::Actor {
		public :
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();

	};
};