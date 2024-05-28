#pragma once

#include "types.h"
#include "Cmn/KDUtl/AttT.h"

namespace Game{
    class EnemyFunctions{
        public:
        static void rotateByFrontDir(Cmn::KDUtl::AttT *idk, sead::Vector3<float> vec){
            idk->mtx00 = vec.mZ;
            idk->mtx10 = 0.0f;
            idk->mtx20 = vec.mX;
            idk->normalize_YZ_Debug("GameEnemyFunctions.cpp", 710);
        };
    };
};