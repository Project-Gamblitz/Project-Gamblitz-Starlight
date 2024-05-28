#pragma once

#include "types.h"
#include "sead/vector.h"

namespace Cmn {
    namespace KDUtl {
        class MtxT {
            public:
            MtxT(){
                memset(this, 0, sizeof(Cmn::KDUtl::MtxT));
            };
            float mtx00;
            float mtx10;
            float mtx20;
            float mtx01;
            float mtx11;
            float mtx21;
            float mtx02;
            float mtx12;
            float mtx22;
            sead::Vector3<float> mPos;
        };
    };
};