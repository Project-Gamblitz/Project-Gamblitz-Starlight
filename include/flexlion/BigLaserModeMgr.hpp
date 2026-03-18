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

	// Jump table hook: creates PlayerWeaponBigLaser with "BigLaser" XLink name
	__attribute__((naked)) static void bigLaserJumpHook();

	// Sets XLink user name on newly created BigLaser weapon and patches vtable
	static Cmn::PlayerWeapon* initWeaponXLink(Cmn::PlayerWeapon *weapon, void *parent);

	// Split bullet pool: dynamic getClassName flag + tracking
	static bool sCreateAsPrincessCannon;
	static void registerBullet(void *bullet, bool isPrincessCannon);
	static void resetBulletPools();
	static bool isBulletKillerWail(void *bullet);
	static bool isBulletPrincessCannon(void *bullet);
	static void *findFreeBullet(bool wantPrincessCannon);
	static void lazyCacheXLinkPtrs();

	// Per-bullet getClassName helper (receives `this` via naked wrapper)
	static const char *getClassNameHelper(void *self);
	static int dbgKWPoolCount();
	static int dbgPCPoolCount();

	// Debug: last shot hook state
	static int sDbgLastMode;        // 0=KW, 1=PC
	static bool sDbgLastWasKW;      // was the allocated bullet KW?
	static bool sDbgLastSwapped;    // did we swap?
	static void *sDbgLastBullet;    // bullet actually used

	// Debug: getClassName call counters
	static int sDbgGetClassKW;     // times getClassName returned BulletOldSuperLaser
	static int sDbgGetClassPC;     // times getClassName returned BulletSuperLaser

	// Debug: XLink user instance pointers from pool bullets
	static void *sDbgKWELink;     // elink ptr from KW pool[0]
	static void *sDbgKWSLink;     // slink ptr from KW pool[0]
	static void *sDbgPCELink;     // elink ptr from PC pool[0]
	static void *sDbgPCSLink;     // slink ptr from PC pool[0]

	// Debug: shot-time XLink swap state
	static bool sDbgCached;           // was sXLinkPtrsCached true at shot time?
	static void *sDbgCachedPCELink;   // cached PC elink value used for swap
	static void *sDbgCachedKWELink;   // cached KW elink value used for swap
	static void *sDbgBulletElinkAfter; // bullet's elink[1] AFTER swap

private:
	BigLaserMode mMode[10];
};

} // namespace Flexlion
