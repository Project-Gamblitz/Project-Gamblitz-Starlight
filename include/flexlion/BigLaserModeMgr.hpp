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

	// Initialize model hook function pointers
	static void initModelHook();

	// Mode determined by initWeaponXLink, read by setupWithModel hook
	static BigLaserMode sLastCreateMode;

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

	// Per-weapon tracking: both KW and PC models are pre-created at setup time.
	// Runtime switching is a pointer swap (weapon+0x338 and 0x4F8), no re-creation.
	static void trackWeapon(Cmn::PlayerWeapon *weapon, BigLaserMode setupMode);
	static void resetWeaponTracking();
	// Swaps active model pointer on tracked weapon (no model re-creation)
	static bool checkAndResetupModel(Cmn::PlayerWeapon *weapon, BigLaserMode currentMode);

	// Called from bigLaserItemPickupHook — swaps pre-created model pointers for PC mode.
	static void reSetupForPlayer(int playerIdx);

	// Swap the BigLaser trie entry to use OldBigLaser archive for KW bullet model creation.
	static void swapBulletModelArchive(bool useOldBigLaser);

	// Vtable hook for BulletSuperLaser::load — wraps with trie swap for KW bullets
	static void bulletLoadHook(void *bullet);

	// Vtable hook for BulletSuperLaser::firstCalc — swaps param set for KW vs PC
	static void bulletFirstCalcHook(void *bullet);

	// Construct KW param set (must be called after param manager is initialized)
	static void initParamSets();

	// BigLaser animation mode switching (human form)
	static void initAnimHook();
	static void cacheAnimClipId(void *animCtrlSet, int playerIdx, int animIdx, const char *pcName);
	static void swapPlayerAnimsToPC(Game::Player *player);
	static void swapPlayerAnimsToKW(Game::Player *player);
	static void resetAnimCache();

	// Reverse of reSetupForPlayer — swaps model back to KW
	static void reSetupForPlayerKW(int playerIdx);

private:
	BigLaserMode mMode[10];
};

} // namespace Flexlion
