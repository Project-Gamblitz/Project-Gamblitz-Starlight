#pragma once

#include "types.h"
#include "Cmn/Actor.h"

namespace Game {
	class PlazaCoconutShot{
        public:
        int calcCoconutCamera(bool, sead::Vector3<float> const&, float, float *, float *, float *, sead::Vector3<float>*, sead::Vector3<float>*, float *);
        _BYTE _0[0x358];
        u32 _358;
	};
};