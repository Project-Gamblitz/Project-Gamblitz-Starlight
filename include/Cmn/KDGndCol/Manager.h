#pragma once

#include "types.h"

namespace Cmn {
    class Actor;
    namespace KDGndCol{
        class Manager{
        public:
            static float cWallNrmY_H;
            static float cWallNrmY_V;
            static float cWallNrmY_S;
            static float cWallNrmY_X;
            static float cWallNrmY_L;
        };
        class CheckIF {
        public:
            CheckIF(Cmn::Actor *actor);
            bool checkMoveSphere(const sead::Vector3<float> &pos, const sead::Vector3<float> &dir,
                                    float distS0, float radiusS1, unsigned int flagW3, unsigned int maskW4,
                                    float wallNrmS2, float scaleS3);
            _BYTE _data[0x70];
        };
    };
};