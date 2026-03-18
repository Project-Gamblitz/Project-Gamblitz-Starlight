#include "flexlion/BigLaserModeMgr.hpp"
#include "flexlion/ProcessMemory.hpp"
#include "flexlion/FsLogger.hpp"
#include "Cmn/Actor.h"

namespace Flexlion {

BigLaserModeMgr *BigLaserModeMgr::sInstance = NULL;

// Function pointer to original BulletSuperLaser::shot (IDA 0x7100589B58, binary 0x589B58)
static void (*sBulletSuperLaserShotOrig)(void*, void*, int, int, sead::Vector3<float>*, sead::Vector3<float>*, int) = NULL;

// --- PlayerWeapon vtable patch for BigLaser XLink name ---
static u64 sBigLaserWeaponVtable[128];
static bool sBigLaserWeaponVtablePatched = false;
static void bigLaserNopSetLinkUserName(void *, const void *) {}
typedef void (*SetLinkUserNameFn)(void*, const sead::SafeStringBase<char> *);
// Saved real setLinkUserName for mode-aware XLink switching at shot time
static SetLinkUserNameFn sRealSetLinkUserName = NULL;

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
	Game::Player *p = (Game::Player *)player;
	int idx = p->mIndex;
	BigLaserMode mode = getMode(idx);

	// Switch weapon XLink based on mode — must use sRealSetLinkUserName
	// because the vtable entry is NOP'd to prevent other code from overwriting
	if(sRealSetLinkUserName != NULL){
		Cmn::PlayerWeapon *weapon = p->mPlayerCustomMgr->getWeapon(Cmn::PlayerCustom::Kind_Wpn_Special, BIGLASER_SPECIAL_ID);
		if(weapon != NULL){
			if(mode == cKillerWail){
				sead::SafeStringBase<char> wName = sead::SafeStringBase<char>::create("OldBigLaser");
				sRealSetLinkUserName(weapon, &wName);
			} else {
				// Princess Cannon: restore default BigLaser weapon XLink
				sead::SafeStringBase<char> wName = sead::SafeStringBase<char>::create("BigLaser");
				sRealSetLinkUserName(weapon, &wName);
			}
		}
	}
}

void BigLaserModeMgr::initHook(){
	sBulletSuperLaserShotOrig = (decltype(sBulletSuperLaserShotOrig))ProcessMemory::MainAddr(0x589B58);
}

Cmn::PlayerWeapon* BigLaserModeMgr::initWeaponXLink(Cmn::PlayerWeapon *weapon) {
	// Call the VIRTUAL setLinkUserName through the vtable
	u64 *origVtable = *(u64 **)weapon;
	SetLinkUserNameFn realSetLinkUserName = (SetLinkUserNameFn)origVtable[87];
	// Save the real function for mode-aware switching at shot time
	sRealSetLinkUserName = realSetLinkUserName;
	sead::SafeStringBase<char> name = sead::SafeStringBase<char>::create("OldBigLaser");
	realSetLinkUserName(weapon, &name);
	if (!sBigLaserWeaponVtablePatched) {
		memcpy(sBigLaserWeaponVtable, origVtable, sizeof(sBigLaserWeaponVtable));
		sBigLaserWeaponVtable[87] = (u64)&bigLaserNopSetLinkUserName;
		sBigLaserWeaponVtablePatched = true;
	}
	*(u64 **)weapon = sBigLaserWeaponVtable;
	return weapon;
}

__attribute__((naked)) void BigLaserModeMgr::bigLaserJumpHook(){
	asm("CMP W20, #0");
	asm("B.EQ #8");
	asm("B _ZN3Cmn15PlayerCustomMgr27checkAndCreateSpecialWeaponEiNS_12PlayerCustom4KindEb_CC");
	asm("MOV X0, X19");
	asm("MOV X1, XZR");
	asm("BL _ZN2Lp3Sys5Actor6createIN3Cmn20PlayerWeaponBigLaserEEEPT_PS1_PN4sead4HeapE");
	asm("BL _ZN8Flexlion15BigLaserModeMgr15initWeaponXLinkEPN3Cmn12PlayerWeaponE");
	asm("B _ZN3Cmn15PlayerCustomMgr27checkAndCreateSpecialWeaponEiNS_12PlayerCustom4KindEb_1D4");
}

} // namespace Flexlion

// Global-scope hook function (called from patch BL replacement)
void bulletSuperLaserShotHook(void *bullet, void *player, int weaponId1, int weaponId2,
                              sead::Vector3<float> *pos, sead::Vector3<float> *dir, int param)
{
	// TODO: weapon XLink switching at shot time crashes — setLinkUserName is unsafe mid-gameplay
	// Need to find a better hook point (special activation, not shot) for weapon XLink switching

	// Call original BulletSuperLaser::shot
	Flexlion::sBulletSuperLaserShotOrig(bullet, player, weaponId1, weaponId2, pos, dir, param);
}

// Replaces BulletSuperLaser::getClassName (binary 0x58ACB4) via slpatch B redirect.
// Makes all BulletSuperLaser instances resolve XLink as "BulletOldSuperLaser" (S1 Killer Wail).
const char *bulletSuperLaserGetClassNameOverride() {
	return "BulletOldSuperLaser";
}

// BigLaserItemOnline pickup hook — replaces BL to sub_71010241EC inside pickup handler
// Sets Princess Cannon mode when a player picks up the item
void bigLaserItemPickupHook(Game::Player *player, int chargeValue) {
	// Original behavior: *(player + 0xE40) = chargeValue
	*(int*)((char*)player + 0xE40) = chargeValue;

	// Set Princess Cannon mode for this player
	if (Flexlion::BigLaserModeMgr::sInstance) {
		Flexlion::BigLaserModeMgr::sInstance->setMode(player->mIndex, Flexlion::cPrincessCannon);
	}
}
