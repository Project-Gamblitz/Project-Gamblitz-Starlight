#pragma once

#include "types.h"
#include "sead/vector.h"

namespace Game{
namespace Cmp{
    class EnemyObjCollision{
        public:
        _BYTE _0[0x110];
        sead::Vector3<float> _110;
    };
    class EnemyPhysics{
        public:
        _BYTE _0[0x48];
        EnemyObjCollision *mObjCollision;
    };
};
};