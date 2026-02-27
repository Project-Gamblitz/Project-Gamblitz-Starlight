#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/ComponentBase.h"
#include "Cmn/ComponentHolder.h"

namespace Game{
namespace Cmp{
    class Movable : public Cmn::ComponentBase{
        public:
        struct CalcArg {
            sead::Vector3<float> *mCurrentPos;
            sead::Vector3<float> *mTargetPos;
            sead::Vector3<float> *mNrmGnd;
            float mSpeed;
            float mAcc;
            u8 mFlag;
            u64 mExtra;
        };
        Movable(Cmn::ComponentHolder *);
        int calc(CalcArg const&calcArg);
        sead::Vector3<float> mVelocity;
        sead::Vector3<float> mKeepOutNrm;
        _BYTE unk40[0x50 - 0x40];
    };
};
};