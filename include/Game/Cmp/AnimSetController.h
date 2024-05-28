#pragma once

#include "types.h"
#include "Game/AnimSetController.h"

namespace Game{
namespace Cmp{
    class AnimSetController{
        public:
        void start(sead::SafeStringBase<char> const&animName){
            mAnimSetCtrl->tryStart(animName, Game::AnimSetController::ErrorLevel::NONE);
        };
        _BYTE _0[0x28];
        Game::AnimSetController *mAnimSetCtrl;
    };
};
};