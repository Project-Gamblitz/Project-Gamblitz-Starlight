#pragma once

#include "types.h"
#include "Cmn/Actor.h"

namespace Game {
	class BulletBombSuction : public Cmn::Actor {
		public :
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        _BYTE _348[0x128];
        u32 mBurstChargeFrm;
	};
};