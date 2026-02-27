#pragma once

#include "types.h"
#include "sead/vector.h"

namespace Game{
namespace Cmp{
    class EnemyObjCollision{
        public:
        int getColNodeNum() const { return unk20; }
        _BYTE unk0[0x20];
        int unk20;
        _BYTE unk24[0x110 - 0x24];
        sead::Vector3<float> _110;
    };
    class EnemyPhysics{
        public:
        bool isHitGnd() const;
        sead::Vector3<float> *getNrmGnd() const;
        _BYTE unk0[0x2C];
        u8 mHitFlags;
        _BYTE unk2D[0x48 - 0x2D];
        EnemyObjCollision *mObjCollision;
        _BYTE unk50[0x158 - 0x50];
        sead::Vector3<float> mFrontDir;
        sead::Vector3<float> _164;
        sead::Vector3<float> mNrmWall;
    };
};
};