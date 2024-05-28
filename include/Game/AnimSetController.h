#pragma once

#include "types.h"
#include "gsys/model.h"

namespace Game{
    class AnimSetController{
        public:
        enum ErrorLevel{
            NONE
        };
        AnimSetController(int,sead::Heap *);
        void calc();
        void resetMatAnim();
        void tryStart(sead::SafeStringBase<char> const&,Game::AnimSetController::ErrorLevel);
        void setSkelSlot(int);
        void setMatSlot(int);
        void load();
        void reset();
        void setPartialSkeletalAnm(gsys::PartialSkeletalAnm const*);
        u32 _0;
        u32 _4;
        _BYTE _8[0x20];
        gsys::Model *mModel;
        _BYTE _28[0x30];
    };
};