#include "flexlion/BigLaserModeMgr.hpp"
#include "flexlion/ProcessMemory.hpp"
#include "flexlion/Utils.hpp"
#include "Cmn/Actor.h"
#include "Cmn/IPlayerCustomInfo.h"
#include "Lp/Sys/modelarc.h"

extern "C" {
    extern u8 _ZTVN4Game27MessagePlayerPerformSpecialE[];
    extern void *_ZN3Cmn18MessageBroadcaster9sInstanceE;
    void _ZN3Cmn17MessageDispatcher15dispatchMessageERKNS_7MessageE(void *, const void *);
}

static void dispatchPerformSpecial(Game::Player *player) {
    u64 msg[2];
    msg[0] = (u64)(_ZTVN4Game27MessagePlayerPerformSpecialE + 0x10);
    msg[1] = (u64)player;
    if (_ZN3Cmn18MessageBroadcaster9sInstanceE)
        _ZN3Cmn17MessageDispatcher15dispatchMessageERKNS_7MessageE(
            _ZN3Cmn18MessageBroadcaster9sInstanceE, msg);
}

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

// Cached BigLaser trie entry for bullet model swapping (declared before resetBulletPools)
static u64 sBigLaserTrieEntry = 0;
static const char *sOrigTrieV6 = NULL;   // original string at trieEntry+0x68
static const char *sOrigTrieV10 = NULL;  // original string at trieEntry+0xC0
static bool sOldBigLaserModelRegistered = false;
static bool sParamPtrsCached = false;

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
	sBigLaserTrieEntry = 0;
	sOrigTrieV6 = NULL;
	sParamPtrsCached = false; // re-cache param pointers per stage (sKWValues persist)
	sOrigTrieV10 = NULL;
	sOldBigLaserModelRegistered = false;
	resetAnimCache();
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

// Trie access: sub_7100009768(trie, type, index) → trie entry pointer
static u64 (*sTrieGetByIndex)(u64, u32, u32) = NULL;
// Trie name lookup: sub_71000877BC(trie, type, &safeString) → pointer to index
static int *(*sTrieNameLookup)(u64, u32, const sead::SafeStringBase<char>*) = NULL;
// Model archive registration: sub_71003B6010(mgr, mgr+568, &safeString)
static void (*sRegisterModelArchive)(void*, void*, const sead::SafeStringBase<char>*) = NULL;

static void resolveBigLaserTrieEntry() {
	if (sBigLaserTrieEntry) return;
	u64 *globalMgr = (u64 *)ProcessMemory::MainAddr(0x2D56A30);
	u64 managerObj = *globalMgr;
	u64 trie = *(u64 *)(managerObj + 0x50);

	sead::SafeStringBase<char> name("BigLaser");
	int *result = sTrieNameLookup(trie, 2, &name);
	s16 index = (s16)(*(u16 *)result);

	sBigLaserTrieEntry = sTrieGetByIndex(trie, 2, (u32)index);

	// Cache original archive name strings
	sOrigTrieV6 = *(const char **)(sBigLaserTrieEntry + 0x68);
	sOrigTrieV10 = *(const char **)(sBigLaserTrieEntry + 0xC0);
}

void BigLaserModeMgr::swapBulletModelArchive(bool useOldBigLaser) {
	resolveBigLaserTrieEntry();
	if (!sBigLaserTrieEntry) return;

	if (useOldBigLaser) {
		*(const char **)(sBigLaserTrieEntry + 0x68) = "Wsp_OldBigLaser";
		*(const char **)(sBigLaserTrieEntry + 0xC0) = "Wsp_OldBigLaser";
	} else {
		*(const char **)(sBigLaserTrieEntry + 0x68) = sOrigTrieV6;
		*(const char **)(sBigLaserTrieEntry + 0xC0) = sOrigTrieV10;
	}
}

// Original BulletSuperLaser::load (saved from vtable before patching)
static void (*sOrigBulletSuperLaserLoad)(void*) = NULL;

// Original BulletSuperLaser::firstCalc (saved from vtable before patching)
static void (*sOrigBulletFirstCalc)(void*) = NULL;

// SuperLaser param constructor: sub_71013C3294(void *paramSet, int flags)
static void (*sSuperLaserParamCtor)(void*, int) = NULL;
// SuperLaser bprm loader: sub_71013C3164(void *paramSet, const char *name)
static void (*sSuperLaserBprmLoad)(void*, const char*) = NULL;

// SuperLaser param set management: KW uses OldSuperLaser.bprm, PC uses SuperLaser.bprm.
// We cache pointers to each param's value storage within the ORIGINAL set and do
// in-place value swaps per-bullet. Game is single-threaded so this is safe.

#define SUPERLASER_PARAM_COUNT 14

// Byte offsets of each param object within the 0x370-byte param set
static const int sParamByteOffsets[SUPERLASER_PARAM_COUNT] = {
	208, 256, 304, 352, 400, 448, 496, 544, 592, 640, 688, 736, 784, 832
};

// KW values loaded from OldSuperLaser.bprm at init time (mutable, filled by initParamSets)
static u32 sKWValues[SUPERLASER_PARAM_COUNT];
static bool sKWValuesLoaded = false; // persists across stages — KW values don't change

// Cached pointers to each param's value storage (from getValue virtual call)
static void *sParamValuePtrs[SUPERLASER_PARAM_COUNT];
// Saved PC (bprm) values — restored after each KW bullet calc
static u32 sSavedPCValues[SUPERLASER_PARAM_COUNT];

// Cache pointers to each param's value storage within the original param set,
// then construct a temporary param set, load OldSuperLaser.bprm into it, and
// extract the 14 param values to use for KW bullets.
void BigLaserModeMgr::initParamSets() {
	if (sParamPtrsCached && sKWValuesLoaded) return;
	if (!sSuperLaserParamCtor || !sSuperLaserBprmLoad) return;

	u64 paramMgr = *(u64 *)ProcessMemory::MainAddr(0x2E6BEA0);
	if (!paramMgr) return;

	u64 paramSet = *(u64 *)(paramMgr + 896);
	if (!paramSet) return;

	// Cache pointers to PC param values (from the game's original SuperLaser.bprm set)
	if (!sParamPtrsCached) {
		for (int i = 0; i < SUPERLASER_PARAM_COUNT; i++) {
			u8 *paramObj = (u8 *)paramSet + sParamByteOffsets[i];
			u64 vtable = *(u64 *)paramObj;
			u64 fn = *(u64 *)(vtable + 216); // getValue virtual
			sParamValuePtrs[i] = ((void *(*)(void *))fn)(paramObj);
		}
		sParamPtrsCached = true;
	}

	// Construct a temporary param set on the starlight heap, load OldSuperLaser.bprm,
	// and extract the 14 param values for KW bullets.
	// Only done once — KW values persist across stages.
	if (!sKWValuesLoaded) {
		sead::Heap *heap = Utils::getStarlightHeap();
		void *tempSet = heap->tryAlloc(0x370, 8);
		if (tempSet) {
			sSuperLaserParamCtor(tempSet, 0);
			sSuperLaserBprmLoad(tempSet, "OldSuperLaser");

			for (int i = 0; i < SUPERLASER_PARAM_COUNT; i++) {
				u8 *paramObj = (u8 *)tempSet + sParamByteOffsets[i];
				u64 vtable = *(u64 *)paramObj;
				u64 fn = *(u64 *)(vtable + 216); // getValue virtual
				void *valPtr = ((void *(*)(void *))fn)(paramObj);
				if (i == 8) { // mKnockBackNoBarrierOff is u8
					sKWValues[i] = *(u8 *)valPtr;
				} else {
					sKWValues[i] = *(u32 *)valPtr;
				}
			}
			// Note: tempSet leaks on the starlight heap (0x370 bytes, once per game boot).
			// Acceptable tradeoff — no destructor available and heap persists.
			sKWValuesLoaded = true;
		}
	}
}

// Overwrite param values in-place with KW defaults, saving PC originals.
static void swapToKW() {
	if (!sParamPtrsCached || !sKWValuesLoaded) return;
	for (int i = 0; i < SUPERLASER_PARAM_COUNT; i++) {
		if (i == 8) { // mKnockBackNoBarrierOff is u8
			sSavedPCValues[i] = *(u8 *)sParamValuePtrs[i];
			*(u8 *)sParamValuePtrs[i] = (u8)sKWValues[i];
		} else {
			sSavedPCValues[i] = *(u32 *)sParamValuePtrs[i];
			*(u32 *)sParamValuePtrs[i] = sKWValues[i];
		}
	}
}

// Restore PC (bprm) values after KW bullet calc.
static void restorePC() {
	if (!sParamPtrsCached || !sKWValuesLoaded) return;
	for (int i = 0; i < SUPERLASER_PARAM_COUNT; i++) {
		if (i == 8) {
			*(u8 *)sParamValuePtrs[i] = (u8)sSavedPCValues[i];
		} else {
			*(u32 *)sParamValuePtrs[i] = sSavedPCValues[i];
		}
	}
}

void BigLaserModeMgr::initModelHook() {
	sBaseSetupWithModel = (decltype(sBaseSetupWithModel))ProcessMemory::MainAddr(0x18983C);
	sFindBoneInModel = (decltype(sFindBoneInModel))ProcessMemory::MainAddr(0x17DE8D0);
	sOrigWeaponModelReg = (decltype(sOrigWeaponModelReg))ProcessMemory::MainAddr(0x3DEC68);
	sTrieGetByIndex = (decltype(sTrieGetByIndex))ProcessMemory::MainAddr(0x9768);
	sTrieNameLookup = (decltype(sTrieNameLookup))ProcessMemory::MainAddr(0x877BC);
	sRegisterModelArchive = (decltype(sRegisterModelArchive))ProcessMemory::MainAddr(0x3B6010);
	sSuperLaserParamCtor = (decltype(sSuperLaserParamCtor))ProcessMemory::MainAddr(0x13C3294);
	sSuperLaserBprmLoad = (decltype(sSuperLaserBprmLoad))ProcessMemory::MainAddr(0x13C3164);

	// Load OldBigLaser archive early (same timing as KingSquid/Tornado ModelArcs)
	sead::SafeStringBase<char> name = sead::SafeStringBase<char>::create("Wsp_OldBigLaser");
	sOldBigLaserArc = new Lp::Sys::ModelArc(name, NULL, 0, NULL, NULL);

	// Hook BulletSuperLaser::load via vtable patching (load is deferred, not called
	// during Actor::create, so the trie swap must happen inside load itself).
	// BulletSuperLaser vtable at binary 0x2a46e00; Cmn::Actor base at +0x90 = 0x2a46e90.
	Cmn::ActorVtable *bslVtable = (Cmn::ActorVtable *)ProcessMemory::MainAddr(0x2a46e90);
	sOrigBulletSuperLaserLoad = (void (*)(void *))bslVtable->load;
	bslVtable->load = (u64)BigLaserModeMgr::bulletLoadHook;

	// Hook BulletSuperLaser::firstCalc for per-bullet param set swapping (KW vs PC).
	// firstCalc drives the state machine which calls cState_Shot calc and reads params.
	sOrigBulletFirstCalc = (void (*)(void *))bslVtable->firstCalc;
	bslVtable->firstCalc = (u64)BigLaserModeMgr::bulletFirstCalcHook;
}

// Vtable hook for BulletSuperLaser::load.
// load is called by the framework AFTER Actor::create returns, so the trie swap
// in extraBigLaserBulletHook has already been restored. We wrap each bullet's load
// with the trie swap based on its pool membership.
void BigLaserModeMgr::bulletLoadHook(void *bullet) {
	bool isKW = !isBulletPrincessCannon(bullet);
	if (isKW) {
		swapBulletModelArchive(true);
	}
	sOrigBulletSuperLaserLoad(bullet);
	if (isKW) {
		swapBulletModelArchive(false);
	}
}

// Vtable hook for BulletSuperLaser::firstCalc.
// Overwrites param values in-place with KW defaults for KW bullets,
// restores PC values after the original firstCalc completes.
void BigLaserModeMgr::bulletFirstCalcHook(void *bullet) {
	if (!sParamPtrsCached || !sKWValuesLoaded) initParamSets();
	bool isKW = !isBulletPrincessCannon(bullet);
	if (isKW && sParamPtrsCached && sKWValuesLoaded) {
		swapToKW();
	}
	sOrigBulletFirstCalc(bullet);
	if (isKW && sParamPtrsCached && sKWValuesLoaded) {
		restorePC();
	}
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

// --- BigLaser animation mode switching ---
// Animation name resolution: sub_71017E3EC8(animResource, &safeString) → clipId in lower 32 bits
static u64 (*sResolveAnimClip)(u64, void*) = NULL;

// Per-player BigLaser animation clip cache (human form only)
struct BigLaserAnimClip {
	int animIdx;
	u32 kwClipId;  // OldBigLaser clip ID (for KW mode, captured at cache time)
	u32 pcClipId;  // BigLaserP clip ID (for PC mode)
};

#define MAX_BL_ANIMS 12
static BigLaserAnimClip sAnimClipCache[10][MAX_BL_ANIMS];
static int sAnimClipCacheCount[10] = {};

void BigLaserModeMgr::initAnimHook() {
	sResolveAnimClip = (decltype(sResolveAnimClip))ProcessMemory::MainAddr(0x17E3EC8);
}

void BigLaserModeMgr::resetAnimCache() {
	for (int i = 0; i < 10; i++) sAnimClipCacheCount[i] = 0;
}

void BigLaserModeMgr::cacheAnimClipId(void *animCtrlSet, int playerIdx, int animIdx, const char *pcName) {
	// Lazy init (registration may fire before initAnimHook)
	if (!sResolveAnimClip) {
		sResolveAnimClip = (decltype(sResolveAnimClip))ProcessMemory::MainAddr(0x17E3EC8);
	}
	if (!sResolveAnimClip || playerIdx < 0 || playerIdx >= 10) return;

	// Get animation resource from AnimCtrlSet (+320 → inner, +208 → animRes)
	u64 inner = *(u64 *)((u8 *)animCtrlSet + 320);
	if (!inner) return;
	u64 animRes = *(u64 *)(inner + 208);
	if (!animRes) return;

	// Resolve BigLaserP clip ID
	sead::SafeStringBase<char> nameStr(pcName);
	u32 pcClip = (u32)sResolveAnimClip(animRes, &nameStr);

	// Don't cache if animation not found
	if ((u16)pcClip == 0xFFFF) return;

	int &count = sAnimClipCacheCount[playerIdx];
	if (count >= MAX_BL_ANIMS) return;

	// Check for duplicate
	for (int i = 0; i < count; i++) {
		if (sAnimClipCache[playerIdx][i].animIdx == animIdx) return;
	}

	sAnimClipCache[playerIdx][count].animIdx = animIdx;
	sAnimClipCache[playerIdx][count].kwClipId = 0; // filled by swapPlayerAnimsToPC before overwrite
	sAnimClipCache[playerIdx][count].pcClipId = pcClip;
	count++;
}

void BigLaserModeMgr::swapPlayerAnimsToPC(Game::Player *player) {
	if (!player) return;
	int idx = player->mIndex;
	if (idx < 0 || idx >= 10) return;

	Game::PlayerMotion *motion = player->mPlayerMotion;
	if (!motion) return;

	Game::PlayerAnimCtrlSet *humanAcs = motion->mPlayerAnimCtrlSet;
	if (!humanAcs) return;

	u64 clipArrayBase = *(u64 *)((u8 *)humanAcs + 16);
	if (!clipArrayBase) return;

	for (int i = 0; i < sAnimClipCacheCount[idx]; i++) {
		int aIdx = sAnimClipCache[idx][i].animIdx;
		u32 *clipPtr = *(u32 **)(clipArrayBase + 8 * aIdx);
		if (clipPtr) {
			sAnimClipCache[idx][i].kwClipId = *clipPtr; // save KW clip before overwrite
			*clipPtr = sAnimClipCache[idx][i].pcClipId;
		}
	}
}

void BigLaserModeMgr::swapPlayerAnimsToKW(Game::Player *player) {
	if (!player) return;
	int idx = player->mIndex;
	if (idx < 0 || idx >= 10) return;

	Game::PlayerMotion *motion = player->mPlayerMotion;
	if (!motion) return;

	Game::PlayerAnimCtrlSet *humanAcs = motion->mPlayerAnimCtrlSet;
	if (!humanAcs) return;

	u64 clipArrayBase = *(u64 *)((u8 *)humanAcs + 16);
	if (!clipArrayBase) return;

	for (int i = 0; i < sAnimClipCacheCount[idx]; i++) {
		int aIdx = sAnimClipCache[idx][i].animIdx;
		u32 *clipPtr = *(u32 **)(clipArrayBase + 8 * aIdx);
		if (clipPtr) *clipPtr = sAnimClipCache[idx][i].kwClipId;
	}
}

void BigLaserModeMgr::reSetupForPlayerKW(int playerIdx) {
	for (int i = 0; i < sWeaponTrackCount; i++) {
		Cmn::PlayerWeapon *w = sWeaponTracks[i].weapon;
		if (!w || sWeaponTracks[i].activeMode == cKillerWail) continue;

		if (!w->iCustomPlayerInfo) continue;
		Game::Player *p = w->iCustomPlayerInfo->vtable->getGamePlayer(w->iCustomPlayerInfo);
		if (!p || p->mIndex != playerIdx) continue;

		void *kwModel = sWeaponTracks[i].kwModel;
		if (!kwModel) break;

		*(void **)((char *)w + 0x338) = kwModel;
		*(void **)((char *)w + 0x4F8) = kwModel;

		sead::SafeStringBase<char> muzzleName("muzzle");
		*(int *)((char *)w + 0x6B0) = sFindBoneInModel(kwModel, &muzzleName);

		sWeaponTracks[i].activeMode = cKillerWail;
		break;
	}
}

} // namespace Flexlion

// Wraps BL to sub_71003DEC68 at binary 0x3D08D4 (special weapon model pre-registration).
// Also registers OldBigLaser model archive so the bullet model creation can find it by name.
void weaponModelPreRegHook(void *mgr, u32 type, u32 weaponId, char flag) {
	if (!mgr) return;
	Flexlion::sOrigWeaponModelReg(mgr, type, weaponId, flag);

	// Register OldBigLaser in the resource cache (once per scene)
	if (!Flexlion::sOldBigLaserModelRegistered && Flexlion::sRegisterModelArchive) {
		sead::SafeStringBase<char> name("Wsp_OldBigLaser");
		Flexlion::sRegisterModelArchive(mgr, (char *)mgr + 568, &name);
		Flexlion::sOldBigLaserModelRegistered = true;
	}
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

	// If this BigLaser weapon is already tracked (i.e., both models were pre-created),
	// just let the base setupWithModel use the cached model at weapon+0x4F8.
	// This prevents re-running the dual-model creation (which corrupts model pointers).
	if (isBigLaser) {
		for (int i = 0; i < Flexlion::sWeaponTrackCount; i++) {
			if (Flexlion::sWeaponTracks[i].weapon == weapon) {
				Flexlion::sBaseSetupWithModel(weapon);
				void *model = *(void **)((char *)weapon + 0x338);
				if (model) {
					sead::SafeStringBase<char> muzzleName("muzzle");
					*(int *)((char *)weapon + 0x6B0) = Flexlion::sFindBoneInModel(model, &muzzleName);
				}
				return;
			}
		}
	}

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

	// Lazy init (stage load may precede first render where initModelHook sets function ptrs)
	if (!Flexlion::sParamPtrsCached || !Flexlion::sKWValuesLoaded)
		Flexlion::BigLaserModeMgr::initParamSets();

	// Swap param values in-place so KW bullets use OldSuperLaser.bprm values during shot setup
	bool shotIsKW = !Flexlion::isBulletPrincessCannon(shotBullet);
	if (shotIsKW && Flexlion::sParamPtrsCached && Flexlion::sKWValuesLoaded) {
		Flexlion::swapToKW();
	}

	// Count Killer Wail shots toward the results screen special count
	if (shotIsKW && player != NULL) {
		dispatchPerformSpecial((Game::Player *)player);
	}

	// Call original BulletSuperLaser::shot on the (possibly swapped) bullet
	Flexlion::sBulletSuperLaserShotOrig(shotBullet, player, weaponId1, weaponId2, pos, dir, param);

	if (shotIsKW && Flexlion::sParamPtrsCached && Flexlion::sKWValuesLoaded) {
		Flexlion::restorePC();
	}

	// Princess Cannon is a one-time use — revert to Killer Wail after firing
	if (!shotIsKW && player != NULL && Flexlion::BigLaserModeMgr::sInstance != NULL) {
		Game::Player *p = (Game::Player *)player;
		Flexlion::BigLaserModeMgr::sInstance->setMode(p->mIndex, Flexlion::cKillerWail);
		Flexlion::BigLaserModeMgr::reSetupForPlayerKW(p->mIndex);
		Flexlion::BigLaserModeMgr::swapPlayerAnimsToKW(p);
	}
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

	// Set Princess Cannon mode, re-setup weapon model, and swap animations
	if (Flexlion::BigLaserModeMgr::sInstance) {
		Flexlion::BigLaserModeMgr::sInstance->setMode(player->mIndex, Flexlion::cPrincessCannon);
		Flexlion::BigLaserModeMgr::reSetupForPlayer(player->mIndex);
		Flexlion::BigLaserModeMgr::swapPlayerAnimsToPC(player);
	}
}
