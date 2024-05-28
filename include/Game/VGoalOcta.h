#pragma once

#include "types.h"

namespace Game {
    class VGoalOcta : public Cmn::Actor
	{
	public:
        static Game::VGoalOcta* findVGoalOcta(Cmn::Def::Team const&);
		void calcCapPos_();
    };
};