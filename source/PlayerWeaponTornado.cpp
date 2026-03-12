#include "flexlion/PlayerWeaponTornado.hpp"
#include "starlight/collector.hpp"
#include "Game/Player/Player.h"

using namespace starlight;

namespace Game {

PlayerWeaponTornado *PlayerWeaponTornado::sInstance = NULL;

// --- PlayerWeapon vtable patch (xlink name + actorOnActivate) ---
static u64 sWeaponVtable[128];
static bool sWeaponVtablePatched = false;
static void nopSetLinkUserName(void *, const void *) {}
typedef void (*SetLinkUserNameFn)(void*, const sead::SafeStringBase<char> *);

// Custom actorOnActivate: replicates Cmn::Actor::actorOnActivate exactly,
// only skipping the final onActivateEmitAndPlay (we handle it in playerFirstCalc).
static void tornadoActorOnActivate(Cmn::Actor *actor) {
	// xlink vtable[7] only — skip setIsActive to prevent deferred OnActivate emission
	// before slink resources are loaded (causes NULL crash during Boot→Plaza).
	// playerFirstCalc handles setIsActive + onActivateEmitAndPlay when actually needed.
	Lp::Sys::XLink *xlink = actor->mXLink;
	if(xlink) {
		u64 *xlinkVt = *(u64 **)xlink;
		((void(*)(void*))xlinkVt[7])(xlink);
	}
	// Call virtual onActivate (vtable index 41, offset 0x148)
	u64 *vtable = *(u64 **)actor;
	((void(*)(void*))vtable[41])(actor);
	// Component holder activation (actor offset 102*8 = 0x330)
	void *comp = *((void**)actor + 102);
	if (comp) {
		u64 *compVt = *(u64 **)comp;
		((void(*)(void*))compVt[8])(comp);
	}
	// Skip onActivateEmitAndPlay — handled by playerFirstCalc
}

Cmn::PlayerWeapon* PlayerWeaponTornado::initWeaponXLink(Cmn::PlayerWeapon *weapon) {
	// Call the VIRTUAL setLinkUserName through the vtable (the non-virtual one is a NOP)
	u64 *origVtable = *(u64 **)weapon;
	SetLinkUserNameFn realSetLinkUserName = (SetLinkUserNameFn)origVtable[87];
	sead::SafeStringBase<char> tornadoName = sead::SafeStringBase<char>::create("Tornado");
	realSetLinkUserName(weapon, &tornadoName);
	if (!sWeaponVtablePatched) {
		memcpy(sWeaponVtable, origVtable, sizeof(sWeaponVtable));
		sWeaponVtable[17] = (u64)&tornadoActorOnActivate;
		sWeaponVtable[87] = (u64)&nopSetLinkUserName;
		sWeaponVtablePatched = true;
	}
	*(u64 **)weapon = sWeaponVtable;
	return weapon;
}

PlayerWeaponTornado::PlayerWeaponTornado(){
	sInstance = this;
	memset(mOnActivatePlayed, 0, sizeof(mOnActivatePlayed));
	memset(mNotInSpecialCount, 0, sizeof(mNotInSpecialCount));
}

void PlayerWeaponTornado::playerFirstCalc(Game::Player *player){
	int id = player->mIndex;
	bool inTornadoSpecial = player->isInSpecial() && player->mSpecialWeaponId == TORNADO_SPECIAL_ID;

	if(!inTornadoSpecial){
		// Debounce: only reset after 30+ frames out of special (~0.5s at 60fps).
		// Prevents isInSpecial() flicker (1-2 frames) from resetting mOnActivatePlayed,
		// while still allowing reset between special uses (many seconds of recharge).
		if(mNotInSpecialCount[id] < 30) mNotInSpecialCount[id]++;
		if(mNotInSpecialCount[id] >= 30) mOnActivatePlayed[id] = false;
		return;
	}
	mNotInSpecialCount[id] = 0;
	if(mOnActivatePlayed[id]) return;

	Cmn::PlayerWeapon *weapon = player->mPlayerCustomMgr->getWeapon(Cmn::PlayerCustom::Kind_Wpn_Special, TORNADO_SPECIAL_ID);
	if(weapon == NULL) return;

	Lp::Sys::XLink *xlink = ((Cmn::Actor*)weapon)->mXLink;
	if(xlink == NULL) return;

	// Wake slink if sleeping (first use after xlink creation), then emit.
	if(xlink->isSleep()) xlink->setIsActive(true);
	xlink->onActivateEmitAndPlay();
	mOnActivatePlayed[id] = true;
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
