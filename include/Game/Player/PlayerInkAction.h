#pragma once

#include "types.h"
#include "Game/WeaponParams.h"
#include "Game/Player/PlayerShotGuide.h"
#include "Cmn/KDUtl/MtxT.h"

namespace Game {
	class Player;
	class PlayerInkAction{
		public:
		sead::Vector3<float> getShotVel_BombStd() const;
		void startRoller_Shot();
		void startSlosher_Shot();
		void shotBullet_N(unsigned char);
		void shotBullet_C(float);
		void shotBullet_G(unsigned char, bool);
		void shotBullet_UM(bool, bool);
		void shotBombChase();
		void shotBombCurling();
		void shotBombRobo();
		void shotBombSplash();
		void shotBombQuick();
		void shotBombSuction();
		void shotBombPiyo();
		void shotSuperBall();
		void fillInkMagazine();
		void refillBubbleBullet();
		void calcShotGuideCollision();
		int getBombThrowSpanFrm(bool);
		void startLauncherThrowAnim();
		void calcShotPosVecMtx(Cmn::KDUtl::MtxT *, float, float, float, bool, bool, bool);
		void calcBullet_B();
		Game::MainWeaponShooterParamaters *getShooterParameters() const;
		Game::MainWeaponRollerCommonParameters *getRollerParameters() const;
		Game::MainWeaponTwinsParameters *getTwinsParameters() const;

		Game::Player *mPlayer;
		_BYTE _8[0x1C];
		u32 mWeaponType;
		_BYTE _28[0x84];
		u32 mChargerChargeFrame;
		u32 mChargerChargeFrameProt;
		u32 mChargerChargeState;
		_BYTE _BC[0x174]; // 1DC
		u32 mSlosherRqLeftFrm;
		_BYTE _234[0x64];
		Game::PlayerShotGuide *mPlayerShotGuide;
	};
};