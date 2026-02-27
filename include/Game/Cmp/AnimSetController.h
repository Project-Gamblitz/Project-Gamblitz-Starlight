#pragma once

#include "types.h"
#include "Cmn/ComponentBase.h"
#include "Game/AnimSetController.h"

namespace Game{
namespace Cmp{
    class AnimSetController : public Cmn::ComponentBase{
        public:
        AnimSetController(Cmn::ComponentHolder *, int, sead::Heap *);
        void start(sead::SafeStringBase<char> const&animName){
            mAnimSetCtrl->tryStart(animName, Game::AnimSetController::ErrorLevel::NONE);
        };
        void reset() { mAnimSetCtrl->reset(); };
        bool isEndCurrentAnim() const;
        bool isCurrentAnim(sead::SafeStringBase<char> const&) const;
        float getCurrentFrame() const;
        float getCurrentAnimNumFrame() const;
        void setCurrentFrame(float);
        void setUpdateRate(float);
        void setModel(gsys::Model *model) { mAnimSetCtrl->mModel = model; };
        void setMatSlot(int slot) { mAnimSetCtrl->setMatSlot(slot); };
        Game::AnimSetController *mAnimSetCtrl;
        u64 _30;
    };
};
};