#include "flexlion/BigLaserModeMgr.hpp"
#include "flexlion/ProcessMemory.hpp"
#include "Cmn/Actor.h"
#include "Cmn/IPlayerCustomInfo.h"
#include "Lp/Sys/modelarc.h"

namespace Flexlion {

BigLaserModeMgr *BigLaserModeMgr::sInstance = NULL;

// Function pointer to original BulletSuperLaser::shot (IDA 0x7100589B58, binary 0x589B58)
static void (*sBulletSuperLaserShotOrig)(void*, void*, int, int, sead::Vector3<float>*, sead::Vector3<float>*, int) = NULL;

// Function pointer to bullet pool activation (IDA 0x7101954EE0, binary 0x1954EE0)
static void (*sBulletActivateFn)(void*, char) = NULL;

typedef void (*SetLinkUserNameFn)(void*, const sead::SafeStringBase<char> *);

// Dynamic getClassName flag: when true, Actor::create resolves XLink as "BulletSuperLaser" (Princess Cannon)
bool BigLaserModeMgr::sCreateAsPrincessCannon = false;

// Set by initWeaponXLink, read by setupWithModel — bridges the mode across the two calls
// since iCustomPlayerInfo on the weapon may not be set yet at setupWithModel time.
BigLaserMode BigLaserModeMgr::sLastCreateMode = cKillerWail;

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

// Per-weapon tracking: weapon pointer + both pre-created models
struct BigLaserWeaponTrack {
	Cmn::PlayerWeapon *weapon;
	BigLaserMode activeMode;
	void *kwModel;  // OldBigLaser model (Killer Wail)
	void *pcModel;  // BigLaser model (Princess Cannon)
};

static BigLaserWeaponTrack sWeaponTracks[10];
static int sWeaponTrackCount = 0;

void BigLaserModeMgr::trackWeapon(Cmn::PlayerWeapon *weapon, BigLaserMode setupMode) {
	// Update existing entry if weapon already tracked
	for (int i = 0; i < sWeaponTrackCount; i++) {
		if (sWeaponTracks[i].weapon == weapon) {
			sWeaponTracks[i].activeMode = setupMode;
			return;
		}
	}
	if (sWeaponTrackCount < 10) {
		sWeaponTracks[sWeaponTrackCount].weapon = weapon;
		sWeaponTracks[sWeaponTrackCount].activeMode = setupMode;
		sWeaponTracks[sWeaponTrackCount].kwModel = NULL;
		sWeaponTracks[sWeaponTrackCount].pcModel = NULL;
		sWeaponTrackCount++;
	}
}

void BigLaserModeMgr::resetWeaponTracking() {
	sWeaponTrackCount = 0;
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
	sLastCreateMode = cKillerWail;

	// Get player index from parent (PlayerCustomMgr+840 = IPlayerCustomInfo*)
	if (sInstance && parent) {
		Cmn::IPlayerCustomInfo *info = *(Cmn::IPlayerCustomInfo **)((char *)parent + 840);
		if (info && info->vtable && info->vtable->getGamePlayer) {
			Game::Player *player = info->vtable->getGamePlayer(info);
			if (player) {
				BigLaserMode mode = sInstance->getMode(player->mIndex);
				sLastCreateMode = mode;
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

// Function pointer to base PlayerWeapon::setupWithModel (IDA 0x710018983C, binary 0x18983C)
static void (*sBaseSetupWithModel)(void*) = NULL;

// Function pointer to model bone search (IDA 0x71017DE8D0, binary 0x17DE8D0)
static int (*sFindBoneInModel)(void*, const sead::SafeStringBase<char>*) = NULL;

// Model pre-registration: original sub_71003DEC68
static void (*sOrigWeaponModelReg)(void*, u32, u32, char) = NULL;

// Lp::Sys::ModelArc for OldBigLaser (loaded directly like rival/player model arcs)
static Lp::Sys::ModelArc *sOldBigLaserArc = NULL;

void BigLaserModeMgr::initModelHook() {
	sBaseSetupWithModel = (decltype(sBaseSetupWithModel))ProcessMemory::MainAddr(0x18983C);
	sFindBoneInModel = (decltype(sFindBoneInModel))ProcessMemory::MainAddr(0x17DE8D0);
	sOrigWeaponModelReg = (decltype(sOrigWeaponModelReg))ProcessMemory::MainAddr(0x3DEC68);

	// Load OldBigLaser archive early (same timing as KingSquid/Tornado ModelArcs)
	sead::SafeStringBase<char> name = sead::SafeStringBase<char>::create("Wsp_OldBigLaser");
	sOldBigLaserArc = new Lp::Sys::ModelArc(name, NULL, 0, NULL, NULL);
}

// Swap the active model on a tracked BigLaser weapon.
// No model re-creation — both models were pre-created at setup time.
// weapon+0x4F8 (cached model) is kept in sync so any future sBaseSetupWithModel
// call by the game (e.g., during firing) uses the correct model.
bool BigLaserModeMgr::checkAndResetupModel(Cmn::PlayerWeapon *weapon, BigLaserMode currentMode) {
	for (int i = 0; i < sWeaponTrackCount; i++) {
		if (sWeaponTracks[i].weapon != weapon) continue;
		if (sWeaponTracks[i].activeMode == currentMode) return true;

		void *targetModel = (currentMode == cKillerWail)
			? sWeaponTracks[i].kwModel
			: sWeaponTracks[i].pcModel;
		if (!targetModel) return true;

		*(void **)((char *)weapon + 0x338) = targetModel;
		*(void **)((char *)weapon + 0x4F8) = targetModel;

		sWeaponTracks[i].activeMode = currentMode;
		return true;
	}
	return false;
}

// Called from bigLaserItemPickupHook — swaps pre-created model pointers.
void BigLaserModeMgr::reSetupForPlayer(int playerIdx) {
	for (int i = 0; i < sWeaponTrackCount; i++) {
		Cmn::PlayerWeapon *w = sWeaponTracks[i].weapon;
		if (!w || sWeaponTracks[i].activeMode == cPrincessCannon) continue;

		if (!w->iCustomPlayerInfo) continue;
		Game::Player *p = w->iCustomPlayerInfo->vtable->getGamePlayer(w->iCustomPlayerInfo);
		if (!p || p->mIndex != playerIdx) continue;

		void *pcModel = sWeaponTracks[i].pcModel;
		if (!pcModel) break;

		// Swap to PC model — no re-creation needed
		*(void **)((char *)w + 0x338) = pcModel;
		*(void **)((char *)w + 0x4F8) = pcModel;

		// Re-find muzzle bone (same skeleton, but safety check)
		sead::SafeStringBase<char> muzzleName("muzzle");
		*(int *)((char *)w + 0x6B0) = sFindBoneInModel(pcModel, &muzzleName);

		sWeaponTracks[i].activeMode = cPrincessCannon;
		break;
	}
}

} // namespace Flexlion

// Wraps BL to sub_71003DEC68 at binary 0x3D08D4 (special weapon model pre-registration).
void weaponModelPreRegHook(void *mgr, u32 type, u32 weaponId, char flag) {
	if (!mgr) return;
	Flexlion::sOrigWeaponModelReg(mgr, type, weaponId, flag);
}

// Replaces sub_71001A4EC0 (shared setupWithModel for all weapon types, binary 0x1A4EC0).
// For BigLaser weapons: creates BOTH models at setup time (PC first, then KW).
// Both models are pre-created so runtime switching is just a pointer swap —
// no sBaseSetupWithModel re-call needed (which crashes from archive invalidation).
// weapon+0x4F8 (cached model ptr) is kept in sync with the active model so the game's
// own calls to sBaseSetupWithModel (e.g., during firing) use the correct model.
void bigLaserSetupWithModelHook(Cmn::PlayerWeapon *weapon) {
	if (!weapon) return;

	char *weaponName = *(char **)((char *)weapon + 0x568);
	bool isBigLaser = (weaponName && strcmp(weaponName, "Wsp_BigLaser") == 0
	                   && Flexlion::sOldBigLaserArc != NULL);

	if (isBigLaser) {
		u64 origArchive = *(u64 *)((char *)weapon + 0x500);
		char *origName = *(char **)((char *)weapon + 0x568);

		// --- Step 1: Create PC model (BigLaser) with the original archive ---
		Flexlion::sBaseSetupWithModel(weapon);
		void *pcModel = *(void **)((char *)weapon + 0x338);

		// --- Step 2: Create KW model (OldBigLaser) ---
		*(u64 *)((char *)weapon + 0x500) = (u64)Flexlion::sOldBigLaserArc;
		*(const char **)((char *)weapon + 0x568) = "Wsp_OldBigLaser";
		*(u64 *)((char *)weapon + 0x4F8) = 0; // force re-creation
		Flexlion::sBaseSetupWithModel(weapon);
		void *kwModel = *(void **)((char *)weapon + 0x338);

		// --- Step 3: Restore archive/name ---
		*(u64 *)((char *)weapon + 0x500) = origArchive;
		*(char **)((char *)weapon + 0x568) = origName;

		// --- Step 4: Set initial active model based on mode ---
		void *activeModel;
		Flexlion::BigLaserMode initMode = Flexlion::BigLaserModeMgr::sLastCreateMode;
		if (initMode == Flexlion::cKillerWail) {
			activeModel = kwModel;
		} else {
			activeModel = pcModel;
		}
		*(void **)((char *)weapon + 0x338) = activeModel;
		*(void **)((char *)weapon + 0x4F8) = activeModel;

		// --- Step 5: Track both models ---
		Flexlion::BigLaserModeMgr::trackWeapon(weapon, initMode);
		// Store model pointers in the last tracked entry
		for (int i = Flexlion::sWeaponTrackCount - 1; i >= 0; i--) {
			if (Flexlion::sWeaponTracks[i].weapon == weapon) {
				Flexlion::sWeaponTracks[i].kwModel = kwModel;
				Flexlion::sWeaponTracks[i].pcModel = pcModel;
				break;
			}
		}
	} else {
		Flexlion::sBaseSetupWithModel(weapon);
	}

	// Find "muzzle" bone in the active model
	void *model = *(void **)((char *)weapon + 0x338);
	if (model) {
		sead::SafeStringBase<char> muzzleName("muzzle");
		*(int *)((char *)weapon + 0x6B0) = Flexlion::sFindBoneInModel(model, &muzzleName);
	}
}

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

					// Deactivate the wrong bullet — the pool allocator already
					// activated it (linked into active list). If left active, the
					// game loop will process it and replay old XLink effects.
					// Inline unlink from the doubly-linked active list at actor+96.
					u64 *node = (u64 *)((char *)bullet + 96);
					u64 next = node[0];
					u64 prev = node[1];
					if (next) *(u64 *)(next + 8) = prev;
					if (prev) *(u64 *)(prev) = next;
					node[0] = 0;
					node[1] = 0;
					// Clear parent pointer and decrement list count
					u64 parent = node[3];
					if (parent) {
						--*(int *)(parent + 16);
						node[3] = 0;
					}

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

	// Set Princess Cannon mode and re-setup weapon model (safe: we're in game logic phase)
	if (Flexlion::BigLaserModeMgr::sInstance) {
		Flexlion::BigLaserModeMgr::sInstance->setMode(player->mIndex, Flexlion::cPrincessCannon);
		Flexlion::BigLaserModeMgr::reSetupForPlayer(player->mIndex);
	}
}
