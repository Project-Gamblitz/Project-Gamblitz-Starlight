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

	// Initialize function pointer to original BulletSuperLaser::shot
	static void initHook();

	// Jump table hook: creates PlayerWeaponBigLaser with mode-based XLink name
	__attribute__((naked)) static void bigLaserJumpHook();

	// Sets XLink user name on newly created BigLaser weapon
	static Cmn::PlayerWeapon* initWeaponXLink(Cmn::PlayerWeapon *weapon, void *parent);

	// Split bullet pool: dynamic getClassName flag + tracking
	static bool sCreateAsPrincessCannon;
	static void registerBullet(void *bullet, bool isPrincessCannon);
	static void resetBulletPools();
	static void lazyCacheXLinkPtrs();

	// Per-bullet getClassName helper (receives `this` via naked wrapper)
	static const char *getClassNameHelper(void *self);

	// Cached XLink UserInstance pointers from pool bullets
	static void *sKWELink;
	static void *sKWSLink;
	static void *sPCELink;
	static void *sPCSLink;

private:
	BigLaserMode mMode[10];
};

} // namespace Flexlion
