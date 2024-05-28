#pragma once

#include "types.h"
#include "sead/vector.h"

namespace Game {
	class PlayerBehindCamera{
        class Up{
            public:
            sead::Vector3<float> _0;
            sead::Vector3<float> mUp;
        };
		public:
        void calcShotDir_Next(sead::Vector3<float> *, float *);
        void resetObjAimParam();
        void resetAfterMainTypeChanged();
        void reset(bool, bool);
        void updateFilter_Yaw(float, float);

        _BYTE _0[0x38];
        float _38;
        _BYTE _40[0x9C];
        Game::PlayerBehindCamera::Up *mUp;
        sead::Vector3<float> mLookAt;
        u32 _EC;
        sead::Vector3<float> *mPosition;
        _BYTE _F8[0x1C];
        sead::Vector4<float> mShotDir3;
        u32 _124;
        float mUpRot5;
        _BYTE _12C[0x24];
        float mFovy;
        u32 _154;
        float mFar;
        _BYTE _15C[0x100]; // F8
        sead::Vector3<float> mShotDir2;
        float mUpRot3;
        _BYTE _26C[0x18];
        sead::Vector3<float> mShotDir;
        float mUpRot2;
        float mTest;
        _BYTE _29C[0x8];
        float mUpRot;
        _BYTE _2AC[0xE8];
        float mUpRot4;
    };
};