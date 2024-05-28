#pragma once
#include "Cmn/Actor.h"
#include "Cmn/KDUtl/MtxT.h"

namespace Game{
    class BulletBombCmnCore : public Cmn::Actor{
        public:
        _BYTE _348[0x384-0x348];
        sead::Matrix34<float> mMatrix;
    };
};