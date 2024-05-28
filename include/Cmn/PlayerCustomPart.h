#pragma once

#include "types.h"
#include "Cmn/Def/Gear.h"
#include "Cmn/Actor.h"
#include "Cmn/IPlayerCustomInfo.h"
#include "Cmn/Def/WeaponKind.h"

namespace Cmn {
	class PlayerCustomPartInfo{
		public:
	};
	class PlayerCustomPart : public Cmn::Actor{
		public:
		void setVisible(bool);
        bool isVisible() const;
		void afterModelUpdateWorldMatrix(gsys::Model *);
		void getRootBoneMtx(sead::Matrix34<float> *)const;
		void calcDraw();
		void requestDraw();
		u64 _348;
		Cmn::IPlayerCustomInfo *iCustomPlayerInfo;
	};
};