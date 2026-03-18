#pragma once

#include "types.h"
#include "Game/Player/Player.h"
#include "sead/string.h"

#define BIGLASER_SPECIAL_ID 16

namespace Flexlion {

enum BigLaserMode {
	cKillerWail,      // S1 -- default, from normal special gauge
	cPrincessCannon   // 5.5.2 -- from BigLaserItemOnline pickup
};

class BigLaserModeMgr {
public:
	static BigLaserModeMgr *sInstance;

	BigLaserModeMgr();

	BigLaserMode getMode(int playerIdx) const;
	void setMode(int playerIdx, BigLaserMode mode);
	void reset();

	// Called after BulletSuperLaser::shot() to swap XLink users (bullet + weapon)
	void onBulletSuperLaserShot(void *bullet, void *player);

	// Initialize function pointer to original BulletSuperLaser::shot
	static void initHook();

	// Jump table hook: creates PlayerWeaponBigLaser with "OldBigLaser" XLink name
	__attribute__((naked)) static void bigLaserJumpHook();

	// Sets XLink user name on newly created BigLaser weapon and patches vtable
	static Cmn::PlayerWeapon* initWeaponXLink(Cmn::PlayerWeapon *weapon);

private:
	BigLaserMode mMode[10];
};

} // namespace Flexlion
