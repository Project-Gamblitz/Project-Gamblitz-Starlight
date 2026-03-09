#pragma once

#include "types.h"
#include "sead/vector.h"

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
            bool checkSphere(const sead::Vector3<float> &pos, float radius,
                             unsigned int kindFloor, unsigned int kindWall, float wallNrmY);
            bool checkMoveSphere(const sead::Vector3<float> &pos, const sead::Vector3<float> &dir,
                                    float distS0, float radiusS1, unsigned int flagW3, unsigned int maskW4,
                                    float wallNrmS2, float scaleS3);
            void clearHit();

            // Layout from 3.1.0 RE:
            // 0x00: DWORD result flags (bit 0 = floor hit, bit 1 = sub-block hit)
            // 0x08: HitInfoImpl* pointer
            // 0x20: Actor* pointer
            int mResultFlags;    // 0x00
            int _pad04;          // 0x04
            void *mHitInfoImpl;  // 0x08
            _BYTE _rest[0x60];   // 0x10
        };
        class CheckIF_EX : public CheckIF {
        public:
            CheckIF_EX(Cmn::Actor *actor);
        };
    };
};