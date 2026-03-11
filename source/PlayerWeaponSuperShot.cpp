#include "flexlion/PlayerWeaponSuperShot.hpp"
#include "flexlion/BulletSuperShot.hpp"
#include "starlight/collector.hpp"
#include "Game/Player/Player.h"

using namespace starlight;

namespace Game {

PlayerWeaponSuperShot *PlayerWeaponSuperShot::sInstance = NULL;

// --- PlayerWeapon vtable patch (xlink name) ---
static u64 sWeaponVtable[128];
static bool sWeaponVtablePatched = false;
static void nopSetLinkUserName(void *, const void *) {}

Cmn::PlayerWeapon* PlayerWeaponSuperShot::initWeaponXLink(Cmn::PlayerWeapon *weapon) {
	weapon->setLinkUserName(sead::SafeStringBase<char>::create("SuperShot"));
	if (!sWeaponVtablePatched) {
		u64 *origVtable = *(u64 **)weapon;
		memcpy(sWeaponVtable, origVtable, sizeof(sWeaponVtable));
		sWeaponVtable[87] = (u64)&nopSetLinkUserName;
		sWeaponVtablePatched = true;
	}
	*(u64 **)weapon = sWeaponVtable;
	return weapon;
}

PlayerWeaponSuperShot::PlayerWeaponSuperShot(){
	sInstance = this;
	memset(mXlinkSet, 0, sizeof(mXlinkSet));
	memset(mFiredBullet, 0, sizeof(mFiredBullet));
	memset(mBullet, 0, sizeof(mBullet));
	mInitialized = false;
}

void PlayerWeaponSuperShot::initialize(){
	if(mInitialized) return;
	if(!Utils::isValidWeapon(Cmn::Def::WeaponKind::cSpecial, SUPERSHOT_SPECIAL_ID)) return;
	mInitialized = true;
}

void PlayerWeaponSuperShot::sleepGachihokoBullet(Game::BulletGachihoko *bullet) {
	// Immediately sleep the game-created BulletGachihoko so it doesn't do anything
	Lp::Sys::Actor *actor = (Lp::Sys::Actor *)bullet;
	actor->reserveSleepAll_(Lp::Sys::Actor::ListNodeKind::None);
}

void PlayerWeaponSuperShot::launchBullet(Game::Player *player) {
	int id = player->mIndex;

	// Create BulletSuperShot on first use
	if (mBullet[id] == NULL) {
		mBullet[id] = Game::BulletSuperShot::create((Lp::Sys::Actor *)player, player->mTeam);
	}

	// If previous bullet is still active, sleep it first
	if (mBullet[id]->mActive) {
		mBullet[id]->doSleep();
	}

	// Compute launch position and velocity from player aim
	sead::Vector3<float> pos = player->mPosition;
	pos.mY += 10.0f; // Raise above feet

	float yaw = player->mShotRotation.mY;
	float pitch = player->mShotRotation.mX;
	float cp = cosf(pitch);
	sead::Vector3<float> vel;
	vel.mX = sinf(yaw) * cp * BulletSuperShot::LAUNCH_SPEED;
	vel.mY = sinf(pitch) * BulletSuperShot::LAUNCH_SPEED;
	vel.mZ = cosf(yaw) * cp * BulletSuperShot::LAUNCH_SPEED;

	mBullet[id]->launch(player, pos, vel);
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

	// Detect fire events by scanning for BulletGachihoko created by SuperShot players.
	// We immediately sleep the Gachihoko bullet and launch our own BulletSuperShot instead.
	bool hasBullet[10] = {};
	Game::BulletGachihoko *toSleep[10] = {};
	auto iterNode = Game::BulletGachihoko::getClassIterNodeStatic();
	for(Game::BulletGachihoko *ita = (Game::BulletGachihoko*)iterNode->derivedFrontActiveActor(); ita != NULL; ita = (Game::BulletGachihoko*)iterNode->derivedNextActiveActor(ita)){
		if(!Utils::isPlayerClass(ita->mSender)){
			continue;
		}
		Game::Player *bulletPlayer = (Game::Player*)ita->mSender;
		int id = bulletPlayer->mIndex;
		if(bulletPlayer->isInSpecial() && bulletPlayer->mSpecialWeaponId == SUPERSHOT_SPECIAL_ID){
			hasBullet[id] = true;
			if(!mFiredBullet[id]){
				// Mark for sleep after iteration (can't sleep during iteration)
				toSleep[id] = ita;

				// Launch our standalone bullet
				launchBullet(bulletPlayer);

				// Trigger weapon xlink fire effect
				Cmn::PlayerWeapon *weapon = bulletPlayer->mPlayerCustomMgr->getWeapon(Cmn::PlayerCustom::Kind_Wpn_Special, SUPERSHOT_SPECIAL_ID);
				if(weapon != NULL){
					weapon->setLinkAction(Cmn::PlayerWeapon::cFireImpact, false);
				}
				mFiredBullet[id] = true;
			}
		}
	}
	// Sleep BulletGachihoko actors after iteration to avoid invalidating the iterator
	for(int i = 0; i < 10; i++){
		if(toSleep[i] != NULL){
			sleepGachihokoBullet(toSleep[i]);
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
		// Sleep any active bullet when special ends
		if(mBullet[id] != NULL && mBullet[id]->mActive){
			mBullet[id]->doSleep();
		}
		mXlinkSet[id] = false;
		mFiredBullet[id] = false;
	}
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
