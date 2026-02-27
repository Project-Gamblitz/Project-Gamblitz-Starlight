#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/ComponentBase.h"
#include "Cmn/ComponentHolder.h"

namespace Game{
namespace Cmp{
    class FrontDirRotatable : public Cmn::ComponentBase{
        public:
        class CalcArg {
        public:
            inline CalcArg(){
                mPhysFrontDir = nullptr;
                mCurrentPos = nullptr;
                mTargetPos = nullptr;
                mSpeed = 0.0f;
                mExtraValue = -1.0f;
            };
            sead::Vector3<float> *mPhysFrontDir;
            sead::Vector3<float> *mCurrentPos;
            sead::Vector3<float> *mTargetPos;
            float mSpeed;
            float mExtraValue;
        };
        enum class Interp{
            Disabled, Enabled
        };
        FrontDirRotatable(Cmn::ComponentHolder *, Interp);
        void calc(CalcArg const&calcArg);
        Interp mInterp;
        int _2C;
    };
};
};