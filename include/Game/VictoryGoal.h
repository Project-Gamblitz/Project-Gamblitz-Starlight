#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/Actor.h"
#include "Game/Obj.h"

namespace Game {
    class VictoryGoal : public Game::MapObjBase
	{
	public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        static Game::VictoryGoal* findVictoryGoal(Cmn::Def::Team const&);
		sead::Vector3<float> calcCapPos_() const;
    };
};