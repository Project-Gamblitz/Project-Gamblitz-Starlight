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

    // Reimplementation of Game::SighterTarget::startAllMarking (removed in 5.5.2)
    // Offsets are from 3.1.0 - verify against 5.5.2 SighterTarget layout
    void SighterTarget_startAllMarking(Game::SighterTarget *sighterTarget, int a2, int a3);
};