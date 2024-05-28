#pragma once

#include "types.h"
#include "Cmn/Actor.h"
#include "Game/SeqMgrVersus.h"

namespace Game {
    class SeqMgrBase : public Cmn::Actor {
	public:
    int getStateID_Current() const;
    };
};