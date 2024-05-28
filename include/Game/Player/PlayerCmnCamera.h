#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/CameraBase.h"

namespace Game {
	class PlayerCmnCamera : public Cmn::CameraBase {
		public :
		PlayerCmnCamera();
		void onCalc();
	};
};