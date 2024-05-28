#pragma once

#include "types.h"
#include "Cmn/Actor.h"
#include "Cmn/IPlayerWeaponGripInfo.h"

namespace Cmn {
	class PlayerWeaponGripHelper : public Cmn::Actor{
		public:
		PlayerWeaponGripHelper(const gsys::ModelAnimation *, Cmn::IPlayerWeaponGripInfo *);
		void calc(sead::Matrix34<float> *,sead::Vector3<float> const&);
		void reset();
		_BYTE _348[0x188D0 - 0x348];
	};
};