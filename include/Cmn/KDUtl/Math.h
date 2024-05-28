#pragma once

#include "types.h"
#include "sead/vector.h"

namespace Cmn {
    namespace KDUtl {
        class Math {
            public:
            static float atan2Table(float, float);
            static float calcAngleAbs(sead::Vector3<float> const&, sead::Vector3<float> const&);
            static void rotateVecAxisY(sead::Vector3<float> *,sead::Vector3<float> const&,float);
        };
    };
};