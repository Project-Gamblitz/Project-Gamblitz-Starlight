#pragma once
#include "Cmn/Actor.h"

namespace Game {
    class BulletGraduallyPaint : public Cmn::Actor {};
}

struct BulletGraduallyPaintInitArg {
    int      unk0;
    int      texType;
    float    posX;
    float    posY;
    float    posZ;
    float    velX;
    float    velY;
    float    velZ;
    int      unk32;
    int      unk36;
    float    startRadius;
    float    endRadius;
    float    totalFrames;
    int      team;
    u64      unk56;
    u16      flags;
};