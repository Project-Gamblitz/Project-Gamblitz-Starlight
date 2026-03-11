#include "flexlion/PlayerWeaponSuperShot.hpp"
#include "flexlion/ProcessMemory.hpp"
#include "starlight/collector.hpp"

using namespace starlight;

namespace Game {

PlayerWeaponSuperShot *PlayerWeaponSuperShot::sInstance = NULL;

// Vtable patching: override setLinkUserName so setFromMush can't overwrite
// our xlink name before createXLink resolves the user
static u64 sWeaponVtable[128];
static bool sVtablePatched = false;

static void nopSetLinkUserName(void *, const void *) {}

Cmn::PlayerWeapon* PlayerWeaponSuperShot::initWeaponXLink(Cmn::PlayerWeapon *weapon) {
	// Set xlink name to SuperShot BEFORE setupWithoutModel runs
	weapon->setLinkUserName(sead::SafeStringBase<char>::create("SuperShot"));
	// Patch vtable so setFromMush's call to setLinkUserName is a no-op
	if (!sVtablePatched) {
		u64 *origVtable = *(u64 **)weapon;
		memcpy(sWeaponVtable, origVtable, sizeof(sWeaponVtable));
		sWeaponVtable[87] = (u64)&nopSetLinkUserName; // setLinkUserName entry
		sVtablePatched = true;
	}
	*(u64 **)weapon = sWeaponVtable;
	return weapon;
}

PlayerWeaponSuperShot::PlayerWeaponSuperShot(){
	sInstance = this;
	memset(mXlinkSet, 0, sizeof(mXlinkSet));
	memset(mFiredBullet, 0, sizeof(mFiredBullet));
	mInitialized = false;
}

void PlayerWeaponSuperShot::initialize(){
	if(mInitialized) return;
	if(!Utils::isValidWeapon(Cmn::Def::WeaponKind::cSpecial, SUPERSHOT_SPECIAL_ID)) return;
	*(u64*)ProcessMemory::MainAddr(0x2A32000) = (u64)&PlayerWeaponSuperShot::getBurstWaitFrame;
	*(u64*)ProcessMemory::MainAddr(0x2A32008) = (u64)&PlayerWeaponSuperShot::getBurstWarnFrame;
	*(u64*)ProcessMemory::MainAddr(0x2A31F40) = (u64)&PlayerWeaponSuperShot::calcHokoDamage;
	mInitialized = true;
}

void PlayerWeaponSuperShot::onCalc(){
	Game::Player *player = Collector::mControlledPlayer;
	if(player == NULL or !Utils::isSceneLoaded()){
		return;
	}
	if(player->isInSpecial() and player->mSpecialWeaponId == SUPERSHOT_SPECIAL_ID){
		player->mPlayerInkAction->mChargerChargeState = 100;
		Prot::ObfStore(&player->mPlayerInkAction->mChargerChargeFrame, 1000);
	}
	bool hasBullet[10] = {};
	auto iterNode = Game::BulletGachihoko::getClassIterNodeStatic();
	for(Game::BulletGachihoko *ita = (Game::BulletGachihoko*)iterNode->derivedFrontActiveActor(); ita != NULL; ita = (Game::BulletGachihoko*)iterNode->derivedNextActiveActor(ita)){
		if(!Utils::isPlayerClass(ita->mSender)){
			continue;
		}
		Game::Player *bulletPlayer = (Game::Player*)ita->mSender;
		int id = bulletPlayer->mIndex;
		hasBullet[id] = true;
		if(bulletPlayer->isInSpecial() && bulletPlayer->mSpecialWeaponId == SUPERSHOT_SPECIAL_ID && !mFiredBullet[id]){
			Cmn::PlayerWeapon *weapon = bulletPlayer->mPlayerCustomMgr->getWeapon(Cmn::PlayerCustom::Kind_Wpn_Special, SUPERSHOT_SPECIAL_ID);
			if(weapon != NULL){
				weapon->setLinkAction(Cmn::PlayerWeapon::cFireImpact, false);
			}
			mFiredBullet[id] = true;
		}
	}
	for(int i = 0; i < 10; i++){
		if(mFiredBullet[i] && !hasBullet[i]){
			mFiredBullet[i] = false;
		}
	}
}

void PlayerWeaponSuperShot::playerFirstCalc(Game::Player *player){
	int id = player->mIndex;
	if(player->isInSpecial() && player->mSpecialWeaponId == SUPERSHOT_SPECIAL_ID && !mXlinkSet[id]){
		mXlinkSet[id] = true;
		mFiredBullet[id] = false;
	} else if(!player->isInSpecial() && mXlinkSet[id]){
		Cmn::PlayerWeapon *weapon = player->mPlayerCustomMgr->getWeapon(Cmn::PlayerCustom::Kind_Wpn_Special, SUPERSHOT_SPECIAL_ID);
		if(weapon != NULL){
			weapon->setLinkAction(Cmn::PlayerWeapon::cPutBack, false);
		}
		mXlinkSet[id] = false;
		mFiredBullet[id] = false;
	}
}

int PlayerWeaponSuperShot::getBurstWaitFrame(Game::BulletGachihoko *bullet){
	if(bullet == NULL){
		return ((int (*)())ProcessMemory::MainAddr(0x4D7D84))();
	}
	if(!Utils::isPlayerClass(bullet->mSender)){
		return ((int (*)())ProcessMemory::MainAddr(0x4D7D84))();
	}
	Game::Player *player = (Game::Player*)bullet->mSender;
	if(player->isInSpecial() and player->mSpecialWeaponId == SUPERSHOT_SPECIAL_ID){
		return 0;
	}
	return ((int (*)())ProcessMemory::MainAddr(0x4D7D84))();
}

int PlayerWeaponSuperShot::getBurstWarnFrame(Game::BulletGachihoko *bullet){
	if(bullet == NULL){
		return ((int (*)())ProcessMemory::MainAddr(0x4D7DB4))();
	}
	if(!Utils::isPlayerClass(bullet->mSender)){
		return ((int (*)())ProcessMemory::MainAddr(0x4D7DB4))();
	}
	Game::Player *player = (Game::Player*)bullet->mSender;
	if(player->isInSpecial() and player->mSpecialWeaponId == SUPERSHOT_SPECIAL_ID){
		return 0;
	}
	return ((int (*)())ProcessMemory::MainAddr(0x4D7DB4))();
}

int PlayerWeaponSuperShot::calcHokoDamage(Game::BulletGachihoko *bullet, int armortype, Cmn::Def::Team team, sead::Vector3<float> const& pos){
	int res = ((int (*)(Game::BulletGachihoko *, int, Cmn::Def::Team, sead::Vector3<float> const&))ProcessMemory::MainAddr(0x4DB3FC))(bullet, armortype, team, pos);
	if(!Utils::isPlayerClass(bullet->mSender)){
		return res;
	}
	Game::Player *player = (Game::Player*)bullet->mSender;
	if(player->isInSpecial() and player->mSpecialWeaponId == SUPERSHOT_SPECIAL_ID and res != 0){
		return DAMAGE;
	}
	return res;
}

void PlayerWeaponSuperShot::supershotJumpHook(){
	asm("CMP W20, #0");
	asm("B.EQ #8");
	asm("B _ZN3Cmn15PlayerCustomMgr27checkAndCreateSpecialWeaponEiNS_12PlayerCustom4KindEb_CC");
	asm("MOV X0, X19");
	asm("MOV X1, XZR");
	asm("BL _ZN2Lp3Sys5Actor6createIN3Cmn22PlayerWeaponShachihokoEEEPT_PS1_PN4sead4HeapE");
	asm("BL _ZN4Game21PlayerWeaponSuperShot15initWeaponXLinkEPN3Cmn12PlayerWeaponE");
	asm("B _ZN3Cmn15PlayerCustomMgr27checkAndCreateSpecialWeaponEiNS_12PlayerCustom4KindEb_1D4");
}

} // namespace Game
