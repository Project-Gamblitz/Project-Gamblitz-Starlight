#include "flexlion/PlayerWeaponTornado.hpp"
#include "starlight/collector.hpp"
#include "Game/Player/Player.h"

using namespace starlight;

namespace Game {

PlayerWeaponTornado *PlayerWeaponTornado::sInstance = NULL;

// --- PlayerWeapon vtable patch (xlink name) ---
static u64 sWeaponVtable[128];
static bool sWeaponVtablePatched = false;
static void nopSetLinkUserName(void *, const void *) {}
typedef void (*SetLinkUserNameFn)(void*, const sead::SafeStringBase<char> *);

Cmn::PlayerWeapon* PlayerWeaponTornado::initWeaponXLink(Cmn::PlayerWeapon *weapon) {
	// Call the VIRTUAL setLinkUserName through the vtable (the non-virtual one is a NOP)
	u64 *origVtable = *(u64 **)weapon;
	SetLinkUserNameFn realSetLinkUserName = (SetLinkUserNameFn)origVtable[87];
	sead::SafeStringBase<char> tornadoName = sead::SafeStringBase<char>::create("Tornado");
	realSetLinkUserName(weapon, &tornadoName);
	if (!sWeaponVtablePatched) {
		memcpy(sWeaponVtable, origVtable, sizeof(sWeaponVtable));
		sWeaponVtable[87] = (u64)&nopSetLinkUserName;
		sWeaponVtablePatched = true;
	}
	*(u64 **)weapon = sWeaponVtable;
	return weapon;
}

PlayerWeaponTornado::PlayerWeaponTornado(){
	sInstance = this;
	memset(mXlinkSet, 0, sizeof(mXlinkSet));
	memset(mOnActivatePlayed, 0, sizeof(mOnActivatePlayed));
}

void PlayerWeaponTornado::playerFirstCalc(Game::Player *player){
	int id = player->mIndex;
	if(player->isInSpecial() && player->mSpecialWeaponId == TORNADO_SPECIAL_ID && !mXlinkSet[id]){
		mXlinkSet[id] = true;
		mOnActivatePlayed[id] = false;
	} else if(!player->isInSpecial() && mXlinkSet[id]){
		mXlinkSet[id] = false;
		mOnActivatePlayed[id] = false;
	}

	// Emit OnActivate sound once the weapon's xlink is ready.
	// actorOnActivate fires before async xlink creation finishes, so we must emit manually.
	// Matches game's actorOnActivate pattern: setIsActive(true) then onActivateEmitAndPlay().
	if(mXlinkSet[id] && !mOnActivatePlayed[id]){
		Cmn::PlayerWeapon *weapon = player->mPlayerCustomMgr->getWeapon(Cmn::PlayerCustom::Kind_Wpn_Special, TORNADO_SPECIAL_ID);
		if(weapon != NULL){
			Lp::Sys::XLink *xlink = ((Cmn::Actor*)weapon)->mXLink;
			if(xlink != NULL){
				xlink->setIsActive(true);
				xlink->onActivateEmitAndPlay();
				mOnActivatePlayed[id] = true;
			}
		}
	}
}

void PlayerWeaponTornado::tornadoJumpHook(){
	asm("CMP W20, #0");
	asm("B.EQ #8");
	asm("B _ZN3Cmn15PlayerCustomMgr27checkAndCreateSpecialWeaponEiNS_12PlayerCustom4KindEb_CC");
	asm("MOV X0, X19");
	asm("MOV X1, XZR");
	asm("BL _ZN2Lp3Sys5Actor6createIN3Cmn19PlayerWeaponShooterEEEPT_PS1_PN4sead4HeapE");
	asm("BL _ZN4Game19PlayerWeaponTornado15initWeaponXLinkEPN3Cmn12PlayerWeaponE");
	asm("B _ZN3Cmn15PlayerCustomMgr27checkAndCreateSpecialWeaponEiNS_12PlayerCustom4KindEb_1D4");
}

} // namespace Game
