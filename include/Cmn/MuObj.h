#pragma once

#include "types.h"

namespace Cmn {
	class MuObj{
		public:
		MuObj();
		_BYTE _0[0x50]; // 7338
		sead::SafeStringBase<char> mUnitConfigName;
		_BYTE _60[0x84]; // 72D8
		sead::Vector3<float> mRot;
		_BYTE _F0[0x7248];
	};
};