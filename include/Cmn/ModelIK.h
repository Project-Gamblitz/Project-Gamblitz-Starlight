#pragma once

#include "Cmn/GfxUtl.h"

namespace Cmn{
    class ModelIKCreateArg{
        public:
        gsys::ModelNW *mModel;
        int mBones[3];
    };
    class ModelIK{
        public:
        ModelIK(Cmn::ModelIKCreateArg const&);
        virtual ~ModelIK();
        void calc(sead::Matrix34<float> const&);
        gsys::ModelNW *mModel;
        int mBones[3];
    };
};