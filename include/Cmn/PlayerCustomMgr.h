#pragma once

#include "types.h"
#include "Cmn/PlayerCustomPart.h"
#include "Cmn/PlayerWeapon.h"

namespace Cmn {
	class PlayerCustom{
		public:
		enum Kind : __int32{
			Kind_None = -1,
			Kind_Gear_Clt = 0x0,
			Kind_Gear_ShsL = 0x1,
			Kind_Gear_ShsR = 0x2,
			Kind_Gear_Hed = 0x3,
			Kind_Armor_Clt = 0x4,
			Kind_Armor_ShsL = 0x5,
			Kind_Armor_ShsR = 0x6,
			Kind_Armor_Hed = 0x7,
			Kind_Bottom = 0x8,
			Kind_Wpn_Main = 0x9,
			Kind_Wpn_Main2 = 0xA,
			Kind_Wpn_Special = 0xB,
			Kind_Wpn_Special2 = 0xC,
			Kind_Tank = 0xD,
			Kind_Mantle = 0xE,
			Kind_End = 0xF,
		};
	};
	class PlayerCustomMgr{
		public:
		void changeTeamColor(sead::Color4f const&);
		void informGearChanged(Cmn::Def::GearKind,int);
		void informWeaponSpecialChanged(int);
		void informWeaponMainChanged(int);
		void informWeaponChanged(Cmn::PlayerCustomPart *,Cmn::PlayerCustomPart *);
		void startUnsetupInCalc(Cmn::Def::GearKind,int);
		void startSetupInCalc(Cmn::Def::GearKind,int,sead::Heap *,int);
		bool isEndUnsetupInCalc(Cmn::Def::GearKind,int) const;
		void attachGear(bool);
		void informBottomChanged(int,bool);
		void informTankChanged(int);
        void informArmorLevel(int, bool);
		void requestDraw();
		void calcDraw_OnlyWeapon();
		void hideHumanBone_AfterCalcDraw();
		void hideHumanBone_BeforeCalcDraw();
		void applyHumanBone_HideEar(int);
		void updateSetupState();
		void updateHarnessVisible(gsys::Model *);
		Cmn::PlayerCustomPart *getMantle() const;
		void calcDraw_ButWeapon();
		void setup();
		void setInfoAndCreateActor(Cmn::IPlayerCustomInfo *,bool);
		Cmn::PlayerWeapon *getWeapon(Cmn::PlayerCustom::Kind,int);
		_BYTE _0[0x430];
		sead::ListImpl mMainWeaponsFirst;
		sead::ListImpl mMainWeaponsSecond;
		sead::ListImpl mSpecialWeaponsFirst;
		sead::ListImpl mSpecialWeaponsSecond;
	};
};