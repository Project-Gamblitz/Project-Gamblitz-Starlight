#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/ComponentBase.h"
#include "Cmn/ComponentHolder.h"
#include "Cmn/KDUtl/AttT.h"

namespace Game{
    class ModelCB;
namespace Cmp{
    class Rollable : public Cmn::ComponentBase{
        public:
        class CalcArg {
        public:
            CalcArg(){
                mVelocity = sead::Vector3<float>::zero;
                mMode = 0;
                mFactor = 0.0f;
                mExtraValue = 0.0f;
            };
            sead::Vector3<float> mVelocity;
            int mMode;
            float mFactor;
            float mExtraValue;
        };

        Rollable(Cmn::ComponentHolder *,Cmn::KDUtl::AttT const*,float,Game::ModelCB *);
        void calc(const CalcArg &calcArg);
        void calcRollAtt_(float angle, const sead::Vector3<float> &axis);

        _BYTE unk28[0x48 - 0x28];
        sead::Vector3<float> mRollAxis;
        float mRadius;
        float mRollAngle;
        int mRollBoneIdx;
        _BYTE unk60[0xD8 - 0x60];
    };
};
};