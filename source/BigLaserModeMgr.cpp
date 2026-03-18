#include "flexlion/BigLaserModeMgr.hpp"
#include "flexlion/ProcessMemory.hpp"
#include "flexlion/FsLogger.hpp"
#include "Cmn/Actor.h"

namespace Flexlion {

BigLaserModeMgr *BigLaserModeMgr::sInstance = NULL;

// Function pointer to original BulletSuperLaser::shot (IDA 0x7100589B58, binary 0x589B58)
static void (*sBulletSuperLaserShotOrig)(void*, void*, int, int, sead::Vector3<float>*, sead::Vector3<float>*, int) = NULL;

typedef void (*SetLinkUserNameFn)(void*, const sead::SafeStringBase<char> *);
static SetLinkUserNameFn sRealSetLinkUserName = NULL;

// --- Split bullet pool tracking ---
// Dynamic getClassName flag: when true, Actor::create resolves XLink as "BulletSuperLaser" (Princess Cannon)
bool BigLaserModeMgr::sCreateAsPrincessCannon = false;

// Bullet pools: track pointers for per-shot elink/slink caching
static void *sKWBullets[20];   // Killer Wail (BulletOldSuperLaser XLink)
static void *sPCBullets[20];   // Princess Cannon (BulletSuperLaser XLink)
static int sKWCount = 0;
static int sPCCount = 0;
static int sKWNextIdx = 0;
static int sPCNextIdx = 0;

void BigLaserModeMgr::registerBullet(void *bullet, bool isPrincessCannon) {
	if (isPrincessCannon) {
		if (sPCCount < 20) sPCBullets[sPCCount++] = bullet;
	} else {
		if (sKWCount < 20) sKWBullets[sKWCount++] = bullet;
	}
	// elink/slink caching is done lazily in lazyCacheXLinkPtrs()
	// because XLink UserInstances may not exist yet at registration time
}

void BigLaserModeMgr::resetBulletPools() {
	sKWCount = 0;
	sPCCount = 0;
	sKWNextIdx = 0;
	sPCNextIdx = 0;
	// Reset cached pointers so they get re-cached from the new pools
	sDbgKWELink = NULL;
	sDbgKWSLink = NULL;
	sDbgPCELink = NULL;
	sDbgPCSLink = NULL;
}

bool BigLaserModeMgr::isBulletKillerWail(void *bullet) {
	for (int i = 0; i < sKWCount; i++) {
		if (sKWBullets[i] == bullet) return true;
	}
	return false;
}

bool BigLaserModeMgr::isBulletPrincessCannon(void *bullet) {
	for (int i = 0; i < sPCCount; i++) {
		if (sPCBullets[i] == bullet) return true;
	}
	return false;
}

// Per-bullet getClassName: checks pool membership so actorLoad resolves correct XLink.
// Called via naked wrapper (bulletSuperLaserGetClassNameOverride) which forwards this in X0.
const char *BigLaserModeMgr::getClassNameHelper(void *self) {
	// During initial pool creation, bullets aren't registered yet — use the flag
	if (sCreateAsPrincessCannon) {
		sDbgGetClassPC++;
		return "BulletSuperLaser";
	}
	// After creation, check per-bullet pool membership
	if (isBulletPrincessCannon(self)) {
		sDbgGetClassPC++;
		return "BulletSuperLaser";
	}
	sDbgGetClassKW++;
	return "BulletOldSuperLaser";
}

void *BigLaserModeMgr::findFreeBullet(bool wantPrincessCannon) {
	void **pool = wantPrincessCannon ? sPCBullets : sKWBullets;
	int count = wantPrincessCannon ? sPCCount : sKWCount;

	// Check offset+56 == 0 to find an actually free bullet (same check as pool allocator)
	for (int i = 0; i < count; i++) {
		void *bullet = pool[i];
		if (bullet && *(u64 *)((char *)bullet + 56) == 0) {
			return bullet;
		}
	}
	return NULL;
}

// Lazy-cache elink/slink UserInstance pointers from pool bullets.
// At registration time (during BulletMgr init), XLink and its UserInstances
// may not be created yet (they're resolved during actorLoad, not actorOnCreate).
// By shot time, all bullets have been through their full lifecycle.
void BigLaserModeMgr::lazyCacheXLinkPtrs() {
	if (sDbgKWELink == NULL && sKWCount > 0) {
		for (int i = 0; i < sKWCount; i++) {
			u64 *xlink = *(u64 **)((char *)sKWBullets[i] + 0x320);
			if (xlink && xlink[1]) {
				sDbgKWELink = (void *)xlink[1];
				sDbgKWSLink = (void *)xlink[2];
				break;
			}
		}
	}
	if (sDbgPCELink == NULL && sPCCount > 0) {
		for (int i = 0; i < sPCCount; i++) {
			u64 *xlink = *(u64 **)((char *)sPCBullets[i] + 0x320);
			if (xlink && xlink[1]) {
				sDbgPCELink = (void *)xlink[1];
				sDbgPCSLink = (void *)xlink[2];
				break;
			}
		}
	}
}

int BigLaserModeMgr::dbgKWPoolCount() { return sKWCount; }
int BigLaserModeMgr::dbgPCPoolCount() { return sPCCount; }
int BigLaserModeMgr::sDbgLastMode = 0;
bool BigLaserModeMgr::sDbgLastWasKW = false;
bool BigLaserModeMgr::sDbgLastSwapped = false;
void *BigLaserModeMgr::sDbgLastBullet = NULL;
void *BigLaserModeMgr::sDbgKWELink = NULL;
void *BigLaserModeMgr::sDbgKWSLink = NULL;
void *BigLaserModeMgr::sDbgPCELink = NULL;
void *BigLaserModeMgr::sDbgPCSLink = NULL;
bool BigLaserModeMgr::sDbgCached = false;
void *BigLaserModeMgr::sDbgCachedPCELink = NULL;
void *BigLaserModeMgr::sDbgCachedKWELink = NULL;
void *BigLaserModeMgr::sDbgBulletElinkAfter = NULL;

BigLaserModeMgr::BigLaserModeMgr(){
	sInstance = this;
	reset();
}

BigLaserMode BigLaserModeMgr::getMode(int playerIdx) const {
	if(playerIdx < 0 || playerIdx >= 10) return cKillerWail;
	return mMode[playerIdx];
}

void BigLaserModeMgr::setMode(int playerIdx, BigLaserMode mode){
	if(playerIdx < 0 || playerIdx >= 10) return;
	mMode[playerIdx] = mode;
}

void BigLaserModeMgr::reset(){
	for(int i = 0; i < 10; i++){
		mMode[i] = cKillerWail;
	}
}

void BigLaserModeMgr::onBulletSuperLaserShot(void *bullet, void *player){
	// No-op
}

void BigLaserModeMgr::initHook(){
	sBulletSuperLaserShotOrig = (decltype(sBulletSuperLaserShotOrig))ProcessMemory::MainAddr(0x589B58);
}

// Called right after Actor::create<PlayerWeaponBigLaser> in the jump hook.
// The jump hook skips the normal checkAndCreateSpecialWeapon flow for BigLaser,
// so setLinkUserName would never be called. We call it here with the vanilla
// "BigLaser" name so the weapon's XLink resolves to PlayerWeapon_BigLaser (5.5.2 PC effects).
//
// Weapon effects are always Princess Cannon (5.5.2). Beam effects are controlled
// per-shot by swapping elink/slink UserInstance pointers on the bullet.
Cmn::PlayerWeapon* BigLaserModeMgr::initWeaponXLink(Cmn::PlayerWeapon *weapon, void *parent) {
	u64 *origVtable = *(u64 **)weapon;
	SetLinkUserNameFn realSetLinkUserName = (SetLinkUserNameFn)origVtable[87];
	sRealSetLinkUserName = realSetLinkUserName;

	// Set weapon XLink name to vanilla "BigLaser" (Princess Cannon / 5.5.2)
	sead::SafeStringBase<char> name = sead::SafeStringBase<char>::create("BigLaser");
	realSetLinkUserName(weapon, &name);

	return weapon;
}

__attribute__((naked)) void BigLaserModeMgr::bigLaserJumpHook(){
	asm("CMP W20, #0");
	asm("B.EQ #8");
	asm("B _ZN3Cmn15PlayerCustomMgr27checkAndCreateSpecialWeaponEiNS_12PlayerCustom4KindEb_CC");
	asm("MOV X0, X19");
	asm("MOV X1, XZR");
	asm("BL _ZN2Lp3Sys5Actor6createIN3Cmn20PlayerWeaponBigLaserEEEPT_PS1_PN4sead4HeapE");
	asm("MOV X1, X19");
	asm("BL _ZN8Flexlion15BigLaserModeMgr15initWeaponXLinkEPN3Cmn12PlayerWeaponEPv");
	asm("B _ZN3Cmn15PlayerCustomMgr27checkAndCreateSpecialWeaponEiNS_12PlayerCustom4KindEb_1D4");
}

} // namespace Flexlion

// Global-scope hook function (called from patch BL replacement)
//
// The bullet pool allocator (case 49) always allocates from "BulletSuperLaser" pool (PC).
// The bullet has PC XLink by default. For KW mode, swap elink/slink UserInstance pointers
// to KW ones (cached from KW pool bullets). For PC mode, use the bullet as-is.
void bulletSuperLaserShotHook(void *bullet, void *player, int weaponId1, int weaponId2,
                              sead::Vector3<float> *pos, sead::Vector3<float> *dir, int param)
{
	if (player != NULL && Flexlion::BigLaserModeMgr::sInstance != NULL) {
		Game::Player *p = (Game::Player *)player;
		Flexlion::BigLaserMode mode = Flexlion::BigLaserModeMgr::sInstance->getMode(p->mIndex);
		bool wantPC = (mode == Flexlion::cPrincessCannon);

		// Debug
		Flexlion::BigLaserModeMgr::sDbgLastMode = wantPC ? 1 : 0;
		Flexlion::BigLaserModeMgr::sDbgLastBullet = bullet;

		// Lazy-cache elink/slink pointers (may not have been available at pool creation time)
		Flexlion::BigLaserModeMgr::lazyCacheXLinkPtrs();

		// Swap elink/slink UserInstance pointers to match desired mode.
		u64 *xlink = *(u64 **)((char *)bullet + 0x320);
		if (xlink) {
			if (!wantPC && Flexlion::BigLaserModeMgr::sDbgKWELink) {
				// Default KW mode: swap bullet's PC elink/slink to KW
				xlink[1] = (u64)Flexlion::BigLaserModeMgr::sDbgKWELink;
				xlink[2] = (u64)Flexlion::BigLaserModeMgr::sDbgKWSLink;
				Flexlion::BigLaserModeMgr::sDbgLastSwapped = true;
			} else if (wantPC && Flexlion::BigLaserModeMgr::sDbgPCELink) {
				// PC mode: ensure PC elink/slink (restore if previously swapped to KW)
				xlink[1] = (u64)Flexlion::BigLaserModeMgr::sDbgPCELink;
				xlink[2] = (u64)Flexlion::BigLaserModeMgr::sDbgPCSLink;
				Flexlion::BigLaserModeMgr::sDbgLastSwapped = false;
			} else {
				Flexlion::BigLaserModeMgr::sDbgLastSwapped = false;
			}
		}
	}

	// Call original BulletSuperLaser::shot
	Flexlion::sBulletSuperLaserShotOrig(bullet, player, weaponId1, weaponId2, pos, dir, param);
}

// Debug counters for getClassName calls
int Flexlion::BigLaserModeMgr::sDbgGetClassKW = 0;
int Flexlion::BigLaserModeMgr::sDbgGetClassPC = 0;

// Replaces BulletSuperLaser::getClassName (binary 0x58ACB4) via slpatch B redirect.
// Naked wrapper: X0 = this (bullet pointer) from the original virtual call.
// Forwards to getClassNameHelper which checks per-bullet pool membership so that
// actorLoad (which runs asynchronously after Actor::create) resolves correct XLink.
__attribute__((naked)) const char *bulletSuperLaserGetClassNameOverride() {
	// X0 = this from the caller, passed as first arg to getClassNameHelper(void *self)
	asm("B _ZN8Flexlion15BigLaserModeMgr18getClassNameHelperEPv");
}

// BigLaserItemOnline pickup hook — replaces BL to sub_71010241EC inside pickup handler
// Sets Princess Cannon mode when a player picks up the item
void bigLaserItemPickupHook(Game::Player *player, int chargeValue) {
	// Original behavior: *(player + 0xE40) = chargeValue
	*(int*)((char*)player + 0xE40) = chargeValue;

	// Set Princess Cannon mode — bullet effects will be swapped at shot time
	if (Flexlion::BigLaserModeMgr::sInstance) {
		Flexlion::BigLaserModeMgr::sInstance->setMode(player->mIndex, Flexlion::cPrincessCannon);
	}
}
