#pragma once

#include "types.h"
#include "Cmn/Actor.h"
#include "string.h"
#include "Cmn/MuObj.h"

namespace Game {
	class ActorMgr{
		public:
        Cmn::Actor *createActor_(sead::SafeStringBase<char> const&, Cmn::MuObj*);
	};
};