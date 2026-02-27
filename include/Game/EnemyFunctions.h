#pragma once

#include "types.h"
#include "Cmn/KDUtl/AttT.h"
#include "Game/PaintUtl.h"
#include "Cmn/KDGndCol/Manager.h"

namespace Game{
    class EnemyFunctions{
        public:
        struct wallHitReflectArg {
            sead::Vector3<float> *outVelocity;
            void *physics;
            const sead::Vector3<float> *inVelocity;
            const sead::Vector3<float> *wallNormal;
        };
        struct calcBallImpactArg {
            const sead::Vector3<float> *playerPos;
            const sead::Vector3<float> *enemyPos;
            const sead::Vector3<float> *velocity;
            float impactToPlayer;
            float impactToPlayerX;
        };
        static void rotateByFrontDir(Cmn::KDUtl::AttT *idk, sead::Vector3<float> vec){
            idk->mtx00 = vec.mZ;
            idk->mtx10 = 0.0f;
            idk->mtx20 = -vec.mX;
            idk->normalize_YZ_Debug("GameEnemyFunctions.cpp", 710);
        };
        static void wallHitReflect(const wallHitReflectArg &arg);
        static sead::Vector3<float> calcBallImpact(const calcBallImpactArg &arg);
        static bool existGndCheck(const sead::Vector3<float> &pos, float radius, float distance);
    };
};