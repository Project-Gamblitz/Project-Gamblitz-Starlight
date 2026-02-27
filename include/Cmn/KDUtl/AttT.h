#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/KDUtl/Math.h"

namespace Cmn {
    namespace KDUtl {
        class AttT {
            public:
            AttT(){
                memset(this, 0, sizeof(Cmn::KDUtl::AttT));
            };
            void normalize_YZ_Debug(char const*,int);
            void normalize_YX_Debug(char const*, int);
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