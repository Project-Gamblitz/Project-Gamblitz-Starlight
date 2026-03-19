#include "flexlion/BigLaserModeMgr.hpp"
#include "flexlion/ProcessMemory.hpp"
#include "Cmn/Actor.h"
#include "Cmn/IPlayerCustomInfo.h"

namespace Flexlion {

BigLaserModeMgr *BigLaserModeMgr::sInstance = NULL;

// Function pointer to original BulletSuperLaser::shot (IDA 0x7100589B58, binary 0x589B58)
static void (*sBulletSuperLaserShotOrig)(void*, void*, int, int, sead::Vector3<float>*, sead::Vector3<float>*, int) = NULL;

// Function pointer to bullet pool activation (IDA 0x7101954EE0, binary 0x1954EE0)
static void (*sBulletActivateFn)(void*, char) = NULL;

typedef void (*SetLinkUserNameFn)(void*, const sead::SafeStringBase<char> *);

// Dynamic getClassName flag: when true, Actor::create resolves XLink as "BulletSuperLaser" (Princess Cannon)
bool BigLaserModeMgr::sCreateAsPrincessCannon = false;

// Bullet pools: track pointers for per-shot elink/slink caching
static void *sKWBullets[20];   // Killer Wail (BulletOldSuperLaser XLink)
static void *sPCBullets[20];   // Princess Cannon (BulletSuperLaser XLink)
static int sKWCount = 0;
static int sPCCount = 0;

// Cached XLink UserInstance pointers
void *BigLaserModeMgr::sKWELink = NULL;
void *BigLaserModeMgr::sKWSLink = NULL;
void *BigLaserModeMgr::sPCELink = NULL;
void *BigLaserModeMgr::sPCSLink = NULL;

void BigLaserModeMgr::registerBullet(void *bullet, bool isPrincessCannon) {
	if (isPrincessCannon) {
		if (sPCCount < 20) sPCBullets[sPCCount++] = bullet;
	} else {
		if (sKWCount < 20) sKWBullets[sKWCount++] = bullet;
	}
}

void BigLaserModeMgr::resetBulletPools() {
	sKWCount = 0;
	sPCCount = 0;
	sKWELink = NULL;
	sKWSLink = NULL;
	sPCELink = NULL;
	sPCSLink = NULL;
}

static bool isBulletPrincessCannon(void *bullet) {
	for (int i = 0; i < sPCCount; i++) {
		if (sPCBullets[i] == bullet) return true;
	}
	return false;
}

// Per-bullet getClassName: checks pool membership so actorLoad resolves correct XLink.
const char *BigLaserModeMgr::getClassNameHelper(void *self) {
	if (sCreateAsPrincessCannon) return "BulletSuperLaser";
	if (isBulletPrincessCannon(self)) return "BulletSuperLaser";
	return "BulletOldSuperLaser";
}

// Lazy-cache elink/slink UserInstance pointers from pool bullets.
// XLink UserInstances may not exist at registration time (resolved during actorLoad).
void BigLaserModeMgr::lazyCacheXLinkPtrs() {
	if (sKWELink == NULL && sKWCount > 0) {
		for (int i = 0; i < sKWCount; i++) {
			u64 *xlink = *(u64 **)((char *)sKWBullets[i] + 0x320);
			if (xlink && xlink[1]) {
				sKWELink = (void *)xlink[1];
				sKWSLink = (void *)xlink[2];
				break;
			}
		}
	}
	if (sPCELink == NULL && sPCCount > 0) {
		for (int i = 0; i < sPCCount; i++) {
			u64 *xlink = *(u64 **)((char *)sPCBullets[i] + 0x320);
			if (xlink && xlink[1]) {
				sPCELink = (void *)xlink[1];
				sPCSLink = (void *)xlink[2];
				break;
			}
		}
	}
}

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

void BigLaserModeMgr::initHook(){
	sBulletSuperLaserShotOrig = (decltype(sBulletSuperLaserShotOrig))ProcessMemory::MainAddr(0x589B58);
	sBulletActivateFn = (decltype(sBulletActivateFn))ProcessMemory::MainAddr(0x1954EE0);
}

// Called right after Actor::create<PlayerWeaponBigLaser> in the jump hook.
// parent = PlayerCustomMgr* (X19). PlayerCustomMgr+840 = IPlayerCustomInfo*.
// Reads the player's BigLaserMode to decide which XLink user name to set:
//   KillerWail (default)  → "OldBigLaser"  → PlayerWeapon_OldBigLaser (S1 effects)
//   PrincessCannon (pickup) → "BigLaser"   → PlayerWeapon_BigLaser    (5.5.2 effects)
Cmn::PlayerWeapon* BigLaserModeMgr::initWeaponXLink(Cmn::PlayerWeapon *weapon, void *parent) {
	u64 *origVtable = *(u64 **)weapon;
	SetLinkUserNameFn realSetLinkUserName = (SetLinkUserNameFn)origVtable[87];

	// Default to KillerWail (S1) XLink name
	char *xlinkName = (char *)"OldBigLaser";

	// Get player index from parent (PlayerCustomMgr+840 = IPlayerCustomInfo*)
	if (sInstance && parent) {
		Cmn::IPlayerCustomInfo *info = *(Cmn::IPlayerCustomInfo **)((char *)parent + 840);
		if (info && info->vtable && info->vtable->getGamePlayer) {
			Game::Player *player = info->vtable->getGamePlayer(info);
			if (player) {
				BigLaserMode mode = sInstance->getMode(player->mIndex);
				if (mode == cPrincessCannon) {
					xlinkName = (char *)"BigLaser";
				}
			}
		}
	}

	sead::SafeStringBase<char> name = sead::SafeStringBase<char>::create(xlinkName);
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

// Shot hook: ensure the correct bullet type (KW vs PC) is used for each player's mode.
//
// Instead of swapping XLink UserInstance pointers (which breaks effect positioning because
// the emitter tree caches model references internally at UserInstance creation time), we
// swap the ENTIRE BULLET. If the pool allocator returned a bullet whose XLink type doesn't
// match the player's BigLaserMode, we find a free bullet of the correct type from our
// registered pools and fire shot() on that instead. Each bullet's native elink has correct
// model references, so effects appear at the beam position.
//
// The "wrong" bullet stays activated but idle — it leaks a pool slot until scene change.
// This is bounded and self-correcting: as wrong-type bullets leak, the pool increasingly
// returns correct-type bullets. With N bullets per type, at most N slots can leak.
void bulletSuperLaserShotHook(void *bullet, void *player, int weaponId1, int weaponId2,
                              sead::Vector3<float> *pos, sead::Vector3<float> *dir, int param)
{
	void *shotBullet = bullet;

	if (player != NULL && Flexlion::BigLaserModeMgr::sInstance != NULL) {
		Game::Player *p = (Game::Player *)player;
		Flexlion::BigLaserMode mode = Flexlion::BigLaserModeMgr::sInstance->getMode(p->mIndex);
		bool wantPC = (mode == Flexlion::cPrincessCannon);
		bool bulletIsPC = Flexlion::isBulletPrincessCannon(bullet);

		if (wantPC != bulletIsPC) {
			// Type mismatch: find a free bullet of the correct type from our pool
			int count = wantPC ? Flexlion::sPCCount : Flexlion::sKWCount;
			void **pool = wantPC ? Flexlion::sPCBullets : Flexlion::sKWBullets;

			for (int i = 0; i < count; i++) {
				void *candidate = pool[i];
				// *(actor+0x38) == 0 means not currently in a beam/shot lifecycle
				if (*(u64 *)((char *)candidate + 0x38) == 0) {
					// Activate the candidate (links into active list; safe if already linked)
					if (Flexlion::sBulletActivateFn) {
						Flexlion::sBulletActivateFn(candidate, 0);
					}
					shotBullet = candidate;
					break;
				}
			}
		}
	}

	// Call original BulletSuperLaser::shot on the (possibly swapped) bullet
	Flexlion::sBulletSuperLaserShotOrig(shotBullet, player, weaponId1, weaponId2, pos, dir, param);
}

// Replaces BulletSuperLaser::getClassName (binary 0x58ACB4) via slpatch B redirect.
// X0 = this from the virtual call, forwarded to getClassNameHelper for pool-based dispatch.
__attribute__((naked)) const char *bulletSuperLaserGetClassNameOverride() {
	asm("B _ZN8Flexlion15BigLaserModeMgr18getClassNameHelperEPv");
}

// BigLaserItemOnline pickup hook — replaces BL to sub_71010241EC inside pickup handler
// Sets Princess Cannon mode when a player picks up the item
void bigLaserItemPickupHook(Game::Player *player, int chargeValue) {
	// Original behavior: *(player + 0xE40) = chargeValue
	*(int*)((char*)player + 0xE40) = chargeValue;

	// Set Princess Cannon mode
	if (Flexlion::BigLaserModeMgr::sInstance) {
		Flexlion::BigLaserModeMgr::sInstance->setMode(player->mIndex, Flexlion::cPrincessCannon);
	}
}
