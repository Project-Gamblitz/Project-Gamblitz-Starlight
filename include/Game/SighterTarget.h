#pragma once

#include "types.h"
#include "Cmn/Actor.h"
// #include "GameIncludes.h"

namespace Game{
    class SighterTarget : public Cmn::Actor {
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        void startMarkedBomb(int);
        void startDeviledBomb(int);
        void stateEnterBurst_();
    };

    // Reimplementations of removed SighterTarget functions
    // Offsets confirmed matching between 3.1.0 and 5.5.2 via calcMarked_
    sead::Vector3<float> SighterTarget_calcMarkingEffectPos(Game::SighterTarget *sighterTarget);
    void SighterTarget_startAllMarking(Game::SighterTarget *sighterTarget, int a2, int a3);
};