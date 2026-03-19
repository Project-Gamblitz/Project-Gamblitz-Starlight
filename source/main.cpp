#include "main.hpp"
#include "flexlion/PlayerWeaponSuperShot.hpp"
#include "flexlion/PlayerWeaponTornado.hpp"
#include "flexlion/BigLaserModeMgr.hpp"
#include "Cui/MsgWindow.h"
#include "Game/BulletBombBase.h"

using namespace starlight;

extern "C" {
    extern void (*__preinit_array_start__[])(void) __attribute__((weak));
    extern void (*__preinit_array_end__[])(void) __attribute__((weak));
    extern void (*__init_array_start__[])(void) __attribute__((weak));
    extern void (*__init_array_end__[])(void) __attribute__((weak));
    extern void (*__fini_array_start__[])(void) __attribute__((weak));
    extern void (*__fini_array_end__[])(void) __attribute__((weak));
}

static void __run_init_array(void) {
    size_t count;
    size_t i;

    count = __preinit_array_end__ - __preinit_array_start__;
    for (i = 0; i < count; i++)
        __preinit_array_start__[i]();

    count = __init_array_end__ - __init_array_start__;
    for (i = 0; i < count; i++)
        __init_array_start__[i]();
}

static void __run_fini_array(void) {
    size_t count = __fini_array_end__ - __fini_array_start__;
    for (size_t i = count; i > 0; i--)
        __fini_array_start__[i - 1]();
}

// Needed on old versions of rtld that doesn't check for DT_INIT existance.
extern "C" void __custom_init(void) {
    __run_init_array();
}

// DT_FINI here for completeness.
extern "C" void __custom_fini(void) {
    __run_fini_array();
}

static agl::DrawContext *mDrawContext;
static sead::TextWriter *mTextWriter;
static sead::ExpHeap* mStarlightHeap;
static int mode;

extern "C" sead::ExpHeap *flexGetStarlightHeap(){
	return mStarlightHeap;
};

static Starlion::KingSquidMgr *kingSquidMgr = NULL;
static Cmn::CtrlChecker *ctrlChecker = NULL;
static Starlion::S1Inkstrike *mS1Inkstrike = NULL;
static Flexlion::InkstrikeMgr *tornadoMgr = NULL;
static Flexlion::BigLaserModeMgr *bigLaserModeMgr = NULL;
template<>
sead::Vector2<float> sead::Vector2<float>::zero = sead::Vector2<float>(0.0f, 0.0f);
template<>
sead::Vector3<float> sead::Vector3<float>::zero = sead::Vector3<float>(0.0f, 0.0f, 0.0f);
template<>
sead::Vector3<float> sead::Vector3<float>::ey = sead::Vector3<float>(0.0f, 1.0f, 0.0f);
namespace sead {
	void Random::init()
	{
		init(nn::os::GetSystemTick());
	}
	
	void Random::init(u32 seed)
	{
		const u32 mt_constant = 0x6C078965;
		mX = mt_constant * (seed ^ (seed >> 30u)) + 1;
		mY = mt_constant * (mX ^ (mX >> 30u)) + 2;
		mZ = mt_constant * (mY ^ (mY >> 30u)) + 3;
		mW = mt_constant * (mZ ^ (mZ >> 30u)) + 4;
	}
	
	void Random::init(u32 seed_x, u32 seed_y, u32 seed_z, u32 seed_w)
	{
		if ((seed_x | seed_y | seed_z | seed_w) == 0)
		{
			seed_w = 0x48077044;
			seed_z = 0x714ACB41;
			seed_y = 0x6C078967;
			seed_x = 1;
		}
		mX = seed_x;
		mY = seed_y;
		mZ = seed_z;
		mW = seed_w;
	}
	
	u32 Random::getU32()
	{
		u32 x = mX ^ (mX << 11u);
		mX = mY;
		mY = mZ;
		mZ = mW;
		mW = mW ^ (mW >> 19u) ^ x ^ (x >> 8u);
		return mW;
	}
	
	u64 Random::getU64()
	{
		return u64(getU32()) << 32u | getU32();
	}
	
	void Random::getContext(u32* x, u32* y, u32* z, u32* w) const
	{
		*x = mX;
		*y = mY;
		*z = mZ;
		*w = mW;
	}
}
bool Game::Utl::ActorFactoryBase::isNoActor() const{
	return false;
}

static bool tsthk = 0;

static int custommgrjpt[27];

static void (*miniMapCamCalcImpl)(Game::MiniMapCamera *_this);
static xlink2::UserInstanceSLink *(*startSkill_DeathMarkingImpl)(Game::Player*, unsigned int, char);
static void (*startAllMarking_ImplOrig)(Game::Player*, int);
static void (*updateCursorEffectOrig)(Game::MiniMap*);
void miniMapCamCalcHook(Game::MiniMapCamera *_this){
	miniMapCamCalcImpl(_this);
	float anim = tornadoMgr->cameraanim;
	u8 *base = (u8*)_this;
	sead::Projection *perspProj = *(sead::Projection **)(base + 0x80);
	sead::Projection *orthoProj = *(sead::Projection **)(base + 0x140);
	if(anim > 0.0f){
		float orig = 1.0f - anim;
		// Top-down: position directly above the at point, north-up
		float topX = _this->mAt.mX;
		float topY = _this->mAt.mY + tornadoMgr->cameraheight;
		float topZ = _this->mAt.mZ;
		if(anim >= 1.0f){
			// Snap to exact top-down to avoid float drift
			_this->mPosition.mX = topX;
			_this->mPosition.mY = topY;
			_this->mPosition.mZ = topZ;
			_this->mAt.mY = 0.0f;
			_this->mUp.mX = 0.0f;
			_this->mUp.mY = 0.0f;
			_this->mUp.mZ = -1.0f;
		} else {
			_this->mPosition.mX = _this->mPosition.mX * orig + topX * anim;
			_this->mPosition.mY = _this->mPosition.mY * orig + topY * anim;
			_this->mPosition.mZ = _this->mPosition.mZ * orig + topZ * anim;
			_this->mAt.mY *= orig;
			_this->mUp.mX *= orig;
			_this->mUp.mY *= orig;
			_this->mUp.mZ = _this->mUp.mZ * orig + (-1.0f) * anim;
		}
		// Switch to perspective projection
		if(perspProj != NULL && perspProj != orthoProj){
			*(sead::Projection **)(base + 0x78) = perspProj;
			((Cmn::CameraBase*)_this)->setFovy(tornadoMgr->camerafovy);
		}
	} else {
		// Restore ortho projection when not in inkstrike
		if(orthoProj != NULL){
			*(sead::Projection **)(base + 0x78) = orthoProj;
		}
	}
}

static void (*handleBulletCloneEventImpl)(Game::BulletCloneHandle *cloneHandle, Game::Player *player, Game::BulletCloneEvent *event, int clonefrm);
void handleBulletCloneEventHook(Game::BulletCloneHandle *cloneHandle, Game::Player *player, Game::BulletCloneEvent *event, int clonefrm){
	int paintfrm = (*((int (**)(Game::SeqMgrBase*, uint))((*(u64*)Game::MainMgr::sInstance->seqMgr) + (78 * 8))))(Game::MainMgr::sInstance->seqMgr, clonefrm); // im lazy to define vtable
	if(event->mType == Game::BulletCloneEvent::Type::BulletTypeInkstrike){
		// Defer the launch — store pending dest and let cShootPrepare handle the delay
		int id = player->mIndex;
		if(id >= 0 && id < 10){
			tornadoMgr->mPendingDest[id] = event->mVel;
			tornadoMgr->mRemoteShotPending[id] = true;
		}
	}
	//FsLogger::LogFormatDefaultDirect("\nEvent:\nType: %i\nPos: %.2f %.2f %.2f\nVel: %.2f %.2f %.2f\nPlayerId: %i\n_unk: %i\n\n", 
	//event->mType, event->mPos.mX, event->mPos.mY, event->mPos.mZ, event->mVel.mX, event->mVel.mY, event->mVel.mZ, event->mPlayerId, event->_unk);
	handleBulletCloneEventImpl(cloneHandle, player, event, clonefrm);
}

// Barrier network sync — unpackStateEvent hook
// The compiler optimized out 'this' (PlayerCloneHandle*) from the calling convention
// because unpackStateEvent never uses it. Actual ABI: (Player*, event*, gameFrame).
static void (*unpackStateEventOriginal)(Game::Player *player, Game::PlayerStateCloneEvent *event, u32 gameFrame);

// Packet types must fit in 6 bits (0-63) — pack() serializes byte 32 as 6 bits.
// Game uses 1-54; we use 55 (StartBarrier) and 56 (EndBarrier).
// Integer data must go in DWORD[6] (bytes 24-27) and DWORD[7] (bytes 28-31)
// because bytes 0-23 are serialized as lossy compressed floats by pack().
#define PACKET_START_BARRIER 55
#define PACKET_END_BARRIER   56
#define PACKET_ALL_MARKING   57

void receiveEndBarrier_Net_Reimpl(Game::Player *player){
	player->mBarrierEndFrm = 0;
	player->_B80 = 0;
	player->mIsInBarrier = 0;
}

// Reimplementation of Game::Player::receiveStartBarrier_Net (3.1.0: 0x7100e45d68)
// Delegates to startBarrier_Common which handles all field setting, effects, and infect.
void receiveStartBarrier_Net_Reimpl(Game::Player *player, int duration, int sourcePlayerIdx){
	int currentFrame = (int)Game::MainMgr::sInstance->mPaintGameFrame;
	if(player->mBarrierEndFrm > currentFrame) return;
	int barrierEndFrm = currentFrame + duration;
	// Clamp clone's endFrm to never exceed the local player's endFrm.
	// Without this, network timing drift makes checkBarrier_CopyFrom's '>' check
	// re-trigger startBarrier_Common on the local player every frame (visual re-infect).
	Game::Player *localPlayer = Game::PlayerMgr::sInstance->getControlledPerformer();
	if(localPlayer != NULL && localPlayer->mBarrierEndFrm > currentFrame){
		if(barrierEndFrm > localPlayer->mBarrierEndFrm)
			barrierEndFrm = localPlayer->mBarrierEndFrm;
	}
	player->startBarrier_Common(barrierEndFrm, sourcePlayerIdx);
}

// Replaces stubbed sendEvent_StartBarrier tail-call inside startBarrier_Common.
// Follows the same pattern as sendStateEvent_StartJetpack (3.1.0: 0x7100e78670):
// construct event, set packet type, store ints at DWORD[6]/DWORD[7], check offline, push.
// startBarrier_Common already guards with !mIsRemote before calling this.
static u32 sBarrierSentExpiry = 0;
void sendEvent_StartBarrierHook(Game::PlayerNetControl *netCtrl, int barrierEndFrm, int sourcePlayerIdx){
	if(netCtrl == NULL || netCtrl->mCloneHandle == NULL) return;
	// Don't re-send while a previous barrier send is still active.
	// Prevents infect loop: network timing drift can make remote clone's mBarrierEndFrm
	// higher than local player's, causing secondCalc's checkBarrier_CopyFrom (which uses >)
	// to re-trigger startBarrier_Common on the local player every frame.
	u32 currentFrame = (u32)Game::MainMgr::sInstance->mPaintGameFrame;
	if(currentFrame < sBarrierSentExpiry) return;
	sBarrierSentExpiry = (u32)barrierEndFrm;
	Game::PlayerCloneHandle *handle = netCtrl->mCloneHandle;
	if(handle->mCloneObjMgr->mIsOfflineScene) return;
	Game::PlayerStateCloneEvent event;
	memset(&event, 0, sizeof(event));
	event._data[32] = PACKET_START_BARRIER;
	// Duration and sourcePlayerIdx at DWORD[6] and DWORD[7] (bytes 24-31)
	// These are transmitted as raw U32 by pack(), unlike bytes 0-23 (lossy float compression)
	int duration = barrierEndFrm - (int)currentFrame;
	*(int*)(event._data + 24) = duration;
	*(int*)(event._data + 28) = sourcePlayerIdx;
	handle->mPlayerCloneObj->pushPlayerStateEvent(event);
}

void unpackStateEventHook(Game::Player *player, Game::PlayerStateCloneEvent *event, u32 gameFrame){
	u8 packetValue = event->_data[32];
	if(packetValue == PACKET_END_BARRIER){
		receiveEndBarrier_Net_Reimpl(player);
		return;
	}
	if(packetValue == PACKET_START_BARRIER){
		int duration = *(int*)(event->_data + 24);
		int sourcePlayerIdx = *(int*)(event->_data + 28);
		receiveStartBarrier_Net_Reimpl(player, duration, sourcePlayerIdx);
		return;
	}
	if(packetValue == PACKET_ALL_MARKING){
		int markingGameFrame = *(int*)(event->_data + 24);
		player->receiveAllMarking(markingGameFrame);
		return;
	}
	unpackStateEventOriginal(player, event, gameFrame);
}

// Debug counters for barrier hit hooks
static int dbgBarrierHitCount = 0;
static int dbgStealthHookCount = 0;
static int dbgDmgVoiceHookCount = 0;

// Play barrier hit effect+SFX using the game's own emitAndPlay_BarrierHit
static void emitBarrierHitSfx(Game::Player *player){
	dbgBarrierHitCount++;
	player->emitAndPlay_BarrierHit(player->mPosition, false);
}

// Hook for playDamageVoiceAndRumble — plays barrier hit effect+SFX when player is in Bubbler
void playDamageVoiceAndRumbleHook(Game::Player *player, Game::DamageReason const &reason, bool isOneTimeDamage){
	dbgDmgVoiceHookCount++;
	if(player->isInBarrier()){
		emitBarrierHitSfx(player);
		return;
	}
	player->playDamageVoiceAndRumble(reason, isOneTimeDamage);
}

// Hook for emitAndPlay_StealthDamage — plays barrier hit effect+SFX
// Fires on both ATTACKER's console (processDamageFromBullet_) and VICTIM's console (receiveAttackEventImpl)
void emitAndPlay_StealthDamageHook(Game::Player *player, int attackerIdx, Cmn::Def::DMG dmg, Game::DamageReason const &reason){
	dbgStealthHookCount++;
	if(player->isInBarrier()){
		emitBarrierHitSfx(player);
	}
	player->emitAndPlay_StealthDamage(attackerIdx, dmg, reason);
}

static void (*playerFirstCalcOg)(Game::Player*);
static void (*playerThirdCalcOg)(Game::Player*);
static void (*playerFourthCalcOg)(Game::Player*);
void playerFirstCalcHook(Game::Player *player){
	playerFirstCalcOg(player);
	tornadoMgr->playerFirstCalc(player);
	Game::PlayerWeaponTornado::sInstance->playerFirstCalc(player);
	Game::PlayerWeaponSuperShot::sInstance->playerFirstCalc(player);

}

// Hook for the isInSpecial() call inside PlayerInkAction::getFlagDrawShooterGuide.
// Vanilla bug: Barrier and FreeBombs aren't in the special allow-list, so the shot guide
// is denied even though these specials let the player keep shooting.
// Patched at 010B2014 (replaces BL to isInSpecial).
bool isInSpecialForShotGuideHook(Game::Player *player){
	if(!player->isInSpecial())
		return false;
	if(player->isInBarrier() || player->isInSpecial_FreeBombs())
		return false;
	return true;
}

// Barrier eats enemy bombs on collision, matching RespawnPoint behavior.
// Iterates all active BulletBombBase instances (covers all sub-weapon bomb types),
// checks distance to barrier center, and calls informAndReactCollision with
// reaction type 11 (sleepBarrier) to destroy the bomb.
static void barrierEatBombs(Game::Player *player){
	sead::Vector3<float> barrierPos;
	player->calcBarrier_CenterPos(&barrierPos);

	int playerTeam = (int)player->mTeam;
	const float radiusSq = 14.0f * 14.0f; // mBarrier_Radius = 25.0

	auto iterNode = Game::BulletBombBase::getClassIterNodeStatic();
	for (Lp::Sys::Actor *actor = iterNode->derivedFrontActiveActor();
		 actor != NULL; )
	{
		// Get next before potentially sleeping current actor
		Lp::Sys::Actor *next = iterNode->derivedNextActiveActor(actor);

		Cmn::Actor *bomb = (Cmn::Actor *)actor;
		if ((int)bomb->mTeam != playerTeam) {
			// Get bomb position via vtable[95] (byte offset 760)
			u64 vtable = *(u64 *)bomb;
			typedef float* (*GetPosFunc)(void*);
			float *pos = ((GetPosFunc)(*(u64 *)(vtable + 760)))(bomb);
			if (pos) {
				float dx = pos[0] - barrierPos.mX;
				float dy = pos[1] - barrierPos.mY;
				float dz = pos[2] - barrierPos.mZ;
				float distSq = dx*dx + dy*dy + dz*dz;

				if (distSq < radiusSq) {
					// informAndReactCollision (vtable[93], byte offset 744)
					// reaction type 11 = sleepBarrier
					typedef void (*InformFunc)(void*, int, int*, sead::Vector3<float>*, int, int, int, int, int);
					int outResult = 0;
					((InformFunc)(*(u64 *)(vtable + 744)))(bomb, 0, &outResult, &barrierPos, 11, playerTeam, 0, 0, 0);

					// Play barrier hit VFX/SFX at bomb position
					sead::Vector3<float> hitPos = {pos[0], pos[1], pos[2]};
					player->emitAndPlay_BarrierHit(hitPos, false);
				}
			}
		}
		actor = next;
	}
}

void playerThirdCalcHook(Game::Player *player){
	bool wasInBarrier = player->isInBarrier();

	// Eat enemy bombs while barrier is active (before thirdCalcOg may expire it)
	if(wasInBarrier)
		barrierEatBombs(player);

	playerThirdCalcOg(player);
	// Play BarrierEnd SFX when barrier expires during thirdCalc
	if(wasInBarrier && !player->isInBarrier()){
		xlink2::Handle handle;
		player->mXLink->searchAndPlayWrap("BarrierEnd", false, &handle);
	}
	tornadoMgr->playerThirdCalc(player);
}

void playerFourthCalcHook(Game::Player *player){
	// Fix vanilla bug: shot guide disappears during Barrier and FreeBombs specials.
	// These specials allow the player to keep shooting, so the guide should stay visible.
	// drawShotGuide is called inside fourthCalc (via calcDraw_In4thCalc), so overrides
	// must be applied BEFORE the original call. drawChargerGuide requires mCutsceneState==5,
	// the hide flag cleared, and the ShotGuide enable bit set.
	bool fixShotGuide = player->isInBarrier() || player->isInSpecial_FreeBombs();
	u32 savedCutsceneState = 0;
	if(fixShotGuide){
		savedCutsceneState = player->mCutsceneState;
		player->mCutsceneState = 5;
		*((_BYTE*)player + 0x10E0) = 0;
		u8 *inkAction = *(u8**)((u8*)player + 0xF80);
		if(inkAction){
			u8 *shotGuide = *(u8**)(inkAction + 0x298);
			if(shotGuide){
				*(shotGuide + 0x61C) |= 1;
				// Prevent drawChargerGuide LABEL_574 from killing shooter/spinner sites.
				// 3.1.0 ShotGuide[0x5D8], 5.5.2 ShotGuide[0x638]. Must be 5 to keep sites alive.
				*(u32*)(shotGuide + 0x638) = 5;
			}
		}
	}

	playerFourthCalcOg(player);
	tornadoMgr->playerFourthCalc(player);

	if(fixShotGuide){
		player->mCutsceneState = savedCutsceneState;
	}
}

void handleDisplayVersion(nn::oe::DisplayVersion *ver){
	if(ver != NULL){
		nn::oe::GetDisplayVersion(ver);
		char buf[64];
		snprintf(buf, sizeof(buf), "%s | %s", ver->name, MOD_VER);
		size_t stLen = strlen(buf);
		memcpy(ver->name, buf, stLen + 1);
	}
}

xlink2::PropertyDefinition::~PropertyDefinition(){};

void renderEntrypoint(agl::DrawContext *drawContext, sead::TextWriter *textWriter)
{
	mDrawContext = drawContext;
	mTextWriter = textWriter;
	DrawUtils::setDrawContext(mDrawContext);
	DrawUtils::setColor(sead::Color4f::cWhite);
	mTextWriter->mColor = sead::Color4f::cWhite;
	
	sead::Heap *oldHeap;

	Collector::init();
	Collector::collect();

	if(mStarlightHeap == NULL){
		allocHeap();
	} 
	if(mStarlightHeap != NULL){
		oldHeap = Collector::mHeapMgr->setCurrentHeap_(mStarlightHeap);
	} else{
		return;
	}
	Utils::setStarlightHeap(mStarlightHeap);
	static bool init = false;
	if(!init){
		init_starlion();
		init = true;
	}

	static bool initcustommushtable;
	if(!initcustommushtable){
		if(Collector::mMushDataHolder != NULL){
			Starlion::Sp1WeaponLookup::Initialize();
			custommgrjptHook();
			Game::PlayerWeaponSuperShot::sInstance->initialize();
			if(Utils::isValidWeapon(Cmn::Def::WeaponKind::cSpecial, 22)){
				*(u64*)ProcessMemory::MainAddr(0x2A3F910) = (u64)&calcAquaBallDamageHook;
			}
			initcustommushtable = 1;
		}
	}
	ctrlChecker->calc();
	Game::PlayerWeaponSuperShot::sInstance->onCalc();
	agentThreeHandle();
	kingSquidMgr->onCalc();
	tornadoMgr->onCalc();
	if(!Utils::isSceneLoaded() && bigLaserModeMgr != NULL){
		bigLaserModeMgr->reset();
	}

	// Debug: show collision attribute under Inkstrike cursor
	if(mTextWriter != NULL){
		static const char *sMaterialNames[] = {
			"Stone00", "Stone01", "Grass00", "Soil00", "Glass00",
			"Wood00", "Plastic00", "Mesh00", "Metal00", "Metal00_SuperShotThrough",
			"Rubber00", "None", "ExFallWater", "Cloth00", "ExKeepOutP",
			"ExKeepOutE", "ExKeepOutR", "Mesh01", "EnemyShield00", "EnemyShield01",
			"ExKeepOutC", "ExFillUpP", "ExFallEnemyInk", "Vinyl00", "ExKeepOutN",
			"Dirt00", "ExKeepOutP_C", "ExKeepOutC_WallClimb", "Carpet00", "EnemyInkShower",
			"ExFillUpE", "Metal01", "ExPaint", "ExKeepOutE2", "ExKeepOutP_JetPack",
		};
		Game::Player *ctrlPlayer = Utils::getControlledPerformer();
		if(ctrlPlayer != NULL && tornadoMgr->playerState[ctrlPlayer->mIndex] == Flexlion::TornadoState::cAim && IS_DEV){
			u16 attr = tornadoMgr->mDbgColAttr;
			if(attr == 0xFFFF){
				mTextWriter->printf("NO_COL [INVALID]\n");
			} else {
				int mat = attr & 0x3F;
				const char *matName = (mat <= 34) ? sMaterialNames[mat] : "???";
				mTextWriter->printf("COL_%X %s%s %s\n", attr, matName, tornadoMgr->mDbgColIsWall ? " (wall)" : "", tornadoMgr->mAimValid[ctrlPlayer->mIndex] ? "" : "[INVALID]");
			}
		}
	}
	if(IS_DEV){
		size_t freeBytes = Collector::mHeapMgr->getCurrentHeap()->getFreeSize();
		float freeMB = (float)freeBytes / (1024.0f * 1024.0f);

		// Box dimensions (pixel coords)
		float boxX = 975.0f, boxY = 575.0f;
		float boxW = 295.0f, boxH = 100.0f;

		// Draw box border using drawLineImm in NDC space
		{
			sead::Matrix34<float> viewMtx = sead::Matrix34<float>::ident;
			sead::Matrix44<float> projMtx = sead::Matrix44<float>::ident;
			agl::utl::DevTools::beginDrawImm(mDrawContext, viewMtx, projMtx);

			// Convert pixel coords to NDC (-1 to 1), Y flipped (screen Y-down, NDC Y-up)
			float x0 = -1.0f + 2.0f * boxX / SCREEN_W;
			float y0 =  1.0f - 2.0f * boxY / SCREEN_H;
			float x1 = -1.0f + 2.0f * (boxX + boxW) / SCREEN_W;
			float y1 =  1.0f - 2.0f * (boxY + boxH) / SCREEN_H;

			sead::Color4f lineCol = sead::Color4f::cWhite;
			float lw = 2.0f;
			agl::utl::DevTools::drawLineImm(mDrawContext, {x0, y0, 0}, {x1, y0, 0}, lineCol, lw); // top
			agl::utl::DevTools::drawLineImm(mDrawContext, {x0, y1, 0}, {x1, y1, 0}, lineCol, lw); // bottom
			agl::utl::DevTools::drawLineImm(mDrawContext, {x0, y0, 0}, {x0, y1, 0}, lineCol, lw); // left
			agl::utl::DevTools::drawLineImm(mDrawContext, {x1, y0, 0}, {x1, y1, 0}, lineCol, lw); // right
		}

		// Text inside box
		sead::Vector2<float> textPos = {boxX + 5.0f, boxY + 5.0f};
		mTextWriter->setCursorFromTopLeft(textPos);
		mTextWriter->printf("[Gamblitz Development Build]\n");
		mTextWriter->printf("Git SHA[%s] v%s\n", DEV_VER, MOD_VER);
		mTextWriter->printf("------------------------\n");
		mTextWriter->printf("[Memory]\n");
		mTextWriter->printf("Free: %.1f mb\n", freeMB);
		mTextWriter->printf("------------------------\n");
	}
	static int renderstate = 0;
	static int renderctr = 0;
	static bool msgWindowShown = false;
	char *scenename = Lp::Utl::getCurSceneName();
	switch(renderstate){
	case 0:
		if(scenename != NULL and strcmp(scenename, "Boot") == 0){
			renderctr+=1;
			if(renderctr > 60){
				renderstate+=1;
			}
		}
		break;
	case 1:
		// renderstate+=DrawUtils::drawLogo(strcmp(scenename, "Boot") != 0);
		break;
	case 2:
		// Show custom MsgWindow after logo on Boot scene (same pattern as Boot::NetworkExe::stateProductPlacement)
		if(Cui::SystemPageMgr::sInstance != NULL && !msgWindowShown){
			Cui::MsgWindowPageHandler *handler = Cui::SystemPageMgr::sInstance->mMsgWindowHandler;
			Cui::MsgArg arg;
			arg.readCommonMsgAttr(sead::SafeStringBase<char>::create("CuiMsgWin"), sead::SafeStringBase<char>::create("Msg_Product_Placement"));
			handler->in(arg);
			msgWindowShown = true;
		}
		if(msgWindowShown){
			Cui::MsgWindowPageHandler *handler = Cui::SystemPageMgr::sInstance->mMsgWindowHandler;
			if(handler->isAbleIn()){
				renderstate+=1;
			}
		}
		break;
	case 3:
		break;
	}
	static int mControlState = 0;
	Collector::mController.update();
	Collector::mControllerDebug.update();
	Collector::mHeapMgr->setCurrentHeap_(oldHeap);
	Utils::onLeaveRender();
}

void agentThreeHandle(){
	Game::PlayerMgr *playerMgr = Collector::mPlayerMgrInstance;
	Cmn::Def::Mode mode = Cmn::Def::getCurMode();
	Cmn::PlayerInfo::setInfoType setType = Cmn::PlayerInfo::cAgent3;
	if(playerMgr != NULL){
		Game::Player* player = playerMgr->getControlledPerformer();
		if(player != NULL and isWorld00()){
			Cmn::PlayerInfo *ctrlPlayerInfo = player->mPlayerInfo;
			Cmn::PlayerCustomMgr *ctrlPlayerCustomMgr = player->mPlayerCustomMgr;
			player->mPlayerInfo->setInfoByPlayerType(0, setType, mode);
		}
	}
}

bool isWorld00(){
	if(Cmn::StaticMem::sInstance) return (Cmn::Def::getCurMode() == Cmn::Def::Mode::cWorld and strcmp(Cmn::StaticMem::sInstance->mMapFileName1.mCharPtr, "Fld_World00_Wld") == 0);
	return 0;
}

int msnArmorHook(Cmn::MushGearInfo *gearInfo, Cmn::Def::GearKind kind, int armorLv){
	if(isWorld00()) return Utils::getGearIdByName(kind, "MSN000");
	return gearInfo->searchIdArmor(kind, armorLv);
}

void healPlayerSuperLandingHook(Game::Player *player){
    player->resetDamage();
    player->dropHoldingClamAll_ForSpecial();
}

u64 specialSetupWithoutModelHook(){
	return 0xa582438000; // byte array for which specials mush model is created
}

int *custommgrjptHook(){
	custommgrjpt[0] = ((u64)&Game::PlayerWeaponSuperShot::supershotJumpHook) - ((u64)custommgrjpt);
	int *oldJptable = (int*)ProcessMemory::MainAddr(0x24BE358);
	for(int i = 1; i < 27; i++){
		custommgrjpt[i] = oldJptable[i - 1];
		if(custommgrjpt[i] != 0){
			custommgrjpt[i]+=((u64)oldJptable) - ((u64)custommgrjpt);
		}
	}
	custommgrjpt[1] = ((u64)&Flexlion::BigLaserModeMgr::bigLaserJumpHook) - ((u64)custommgrjpt);
	custommgrjpt[2] = ((u64)&Game::PlayerWeaponTornado::tornadoJumpHook) - ((u64)custommgrjpt);
	return custommgrjpt;
}

CURLcode curl_easy_perform_hook(CURL *curl){
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
	curl_easy_setopt(curl, CURLOPT_USE_SSL, 0L);
	return curl_easy_perform(curl);
}

void registKingSquidAnimHook(Game::PlayerAnimCtrlSet *animCtrlSet, int a1,char const*a2,bool a3,bool a4){
	register Game::PlayerMotion *motion asm("x19");
	Starlion::PlayerKingSquid* kingsquid = ((Starlion::PlayerKingSquid*)motion->mPlayer->mPlayerKingSquid);

	// Replace BigLaserP animation names with non-P S1 Killer Wail versions
	// e.g. "Shoot_BigLaserP" -> "Shoot_BigLaser", "Sp_Start_BigLaserP_Ed" -> "Sp_Start_BigLaser_Ed"
	char s1Name[64];
	const char *useName = a2;
	const char *pPos = (a2 != NULL) ? strstr(a2, "BigLaserP") : NULL;
	if(pPos != NULL) {
		int prefixLen = (int)(pPos - a2) + 8; // includes "BigLaser" (8 chars)
		memcpy(s1Name, a2, prefixLen);
		strcpy(s1Name + prefixLen, pPos + 9); // skip the P, copy rest including null
		useName = s1Name;
	}

	animCtrlSet->registAnim(a1, useName, a3, a4);
	if(kingsquid == NULL){
		return;
	}
	if(kingsquid->mKingSquidAnim == NULL){
		return;
	}
	kingsquid->mKingSquidAnim->registAnim(a1, useName, a3, a4);
}

void setupKingSquidAnimHook(Game::PlayerAnimCtrlSet *animCtrlSet, Game::PlayerJoint *joint){
	animCtrlSet->setup(joint);
	Starlion::PlayerKingSquid* kingsquid = ((Starlion::PlayerKingSquid*)joint->mPlayer->mPlayerKingSquid);
	if(kingsquid == NULL){
		return;
	}
	if(kingsquid->mKingSquidAnim == NULL or kingsquid->mPlayerJointKingSquid == NULL){
		return;
	}
	kingsquid->mKingSquidAnim->setup(kingsquid->mPlayerJointKingSquid);
}

void kingSquidAnimSetControllerHook(Game::AnimSetController *anim, gsys::PartialSkeletalAnm const* cool){
	anim->setPartialSkeletalAnm(cool);
	register Game::PlayerMotion *motion asm("x19");
	Starlion::PlayerKingSquid* kingsquid = ((Starlion::PlayerKingSquid*)motion->mPlayer->mPlayerKingSquid);
	if(kingsquid == NULL){
		return;
	}
	if(kingsquid->mKingSquidAnim == NULL){
		return;
	}
	kingsquid->mSquidAnimCtrl = anim;
	kingsquid->mKingSquidAnimCtrl = new Game::AnimSetController(3, NULL);
	kingsquid->mKingSquidAnimCtrl->_4 = 1;
	kingsquid->mKingSquidAnimCtrl->mModel = kingsquid->mKingSquidModel;
	//*(u64*)(0x69) = 0x69;
	kingsquid->mKingSquidAnimCtrl->setSkelSlot(0);
	kingsquid->mKingSquidAnimCtrl->setMatSlot(1);
	kingsquid->mKingSquidAnimCtrl->load();
	kingsquid->mKingSquidAnimCtrl->setPartialSkeletalAnm(cool);
}

void init_starlion(){
	exl::util::impl::InitMemLayout();
	FsLogger::Initialize();
	FsLogger::LogFormatDefaultDirect("[Gamblitz] Initialized funcs/vars!\n");
	if(IS_DEV){
		FsLogger::LogFormatDefaultDirect("[Gamblitz Development Build]\nDev Build v.%s ", DEV_VER);
		FsLogger::LogFormatDefaultDirect("Mod Version: %s\n", MOD_VER);
	}
	Utils::rotateMtxY = (rotateMtxFunc)(ProcessMemory::MainAddr(0x159904));
	Utils::rotateMtxX = (rotateMtxFunc)(ProcessMemory::MainAddr(0x1599B4));
	miniMapCamCalcImpl = *(void (**)(Game::MiniMapCamera*))ProcessMemory::MainAddr(0x2AE8518);
	*(void (**)(Game::MiniMapCamera*))ProcessMemory::MainAddr(0x2AE8518) = miniMapCamCalcHook;
	Cmn::ActorVtable *playerVtable = (Cmn::ActorVtable*)ProcessMemory::MainAddr(0x2C0BAD8);
	playerFirstCalcOg = (void (*)(Game::Player*))playerVtable->firstCalc;
	playerThirdCalcOg = (void (*)(Game::Player*))playerVtable->thirdCalc;
	playerFourthCalcOg = (void (*)(Game::Player*))playerVtable->fourthCalc;
	playerVtable->firstCalc = (u64)playerFirstCalcHook;
	playerVtable->thirdCalc = (u64)playerThirdCalcHook;
	playerVtable->fourthCalc = (u64)playerFourthCalcHook;
	handleBulletCloneEventImpl = (void (*)(Game::BulletCloneHandle *, Game::Player *, Game::BulletCloneEvent *, int))ProcessMemory::MainAddr(0x4CF54C);
	/* TODO: fill in 5.5.2 offset for unpackStateEvent (3.1.0 IDA: 0x7100e79b30) */
	unpackStateEventOriginal = (void (*)(Game::Player *, Game::PlayerStateCloneEvent *, u32))ProcessMemory::MainAddr(0x0104CAA8);
	// startBarrierCommon no longer needed — receive side sets barrier fields directly
	// DrawUtils::makeTudou();
	FsLogger::LogFormatDefaultDirect("[Gamblitz] Initialized DrawUtils, 0x%x free RAM.\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());
	kingSquidMgr = new Starlion::KingSquidMgr();
	FsLogger::LogFormatDefaultDirect("[Gamblitz] Created KingSquidMgr, 0x%x free RAM.\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());
	mS1Inkstrike = new Starlion::S1Inkstrike();
	new Game::PlayerWeaponSuperShot();
	new Game::PlayerWeaponTornado();
	bigLaserModeMgr = new Flexlion::BigLaserModeMgr();
	Flexlion::BigLaserModeMgr::initHook();
	Flexlion::BigLaserModeMgr::initModelHook();
	tornadoMgr = new Flexlion::InkstrikeMgr();
	FsLogger::LogFormatDefaultDirect("[Gamblitz] Created InkstrikeMgr, 0x%x free RAM.\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());
	_BYTE randomBuf[0x28];
	memset(randomBuf, 0, sizeof(randomBuf));
	sead::TaskConstructArg arg;
	memset(&arg, 0, sizeof(arg));
	arg._0 = (u64)(&randomBuf);
	ctrlChecker = new Cmn::CtrlChecker(arg);
	FsLogger::LogFormatDefaultDirect("[Gamblitz] Created Cmn::CtrlChecker, 0x%x free RAM.\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());
	ctrlChecker->calc();
	FsLogger::LogFormatDefaultDirect("[Gamblitz] Initialized, 0x%x free RAM.\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());

	startSkill_DeathMarkingImpl = (xlink2::UserInstanceSLink *(*)(Game::Player*, unsigned int, char))
		ProcessMemory::MainAddr(0x01011268);

	startAllMarking_ImplOrig = (void (*)(Game::Player*, int))
		ProcessMemory::MainAddr(0x010197F0);

	updateCursorEffectOrig = (void (*)(Game::MiniMap*))
		ProcessMemory::MainAddr(0x00A146E4);

}

void playerModelSetupHook(Game::PlayerModel *pmodel){
	pmodel->setup();
	pmodel->mPlayer->mPlayerKingSquid = new Starlion::PlayerKingSquid(pmodel->mPlayer);
	tornadoMgr->registerPlayer(pmodel->mPlayer);
	Game::PlayerWeaponSuperShot::sInstance->registerPlayer(pmodel->mPlayer);
	int idx = pmodel->mPlayer->mIndex;
}

int calcAquaBallDamageHook(Game::BulletSpAquaBall *bullet, int armortype, Cmn::Def::Team team, sead::Vector3<float> const& pos){
	int res = ((int (*)(Game::BulletSpAquaBall *, int, Cmn::Def::Team, sead::Vector3<float> const&))ProcessMemory::MainAddr(0x53FF70))(bullet, armortype, team, pos);
	if(!Utils::isPlayerClass(bullet->mSender)){
		return res;
	}
	Game::Player *player = (Game::Player*)bullet->mSender;
	if(player->mPlayerKingSquid == NULL){
		return res;
	}
	Starlion::PlayerKingSquid *kingsquid = (Starlion::PlayerKingSquid*)player->mPlayerKingSquid;
	if(kingsquid->mBullet == bullet and team != bullet->mTeam){
		return 1600;
	}
	return res;
}

void playerKingSquidCalcHook(Game::Player *player){
	Starlion::PlayerKingSquid *kingSquid = ((Starlion::PlayerKingSquid*)player->mPlayerKingSquid);
	if(kingSquid != NULL){
		kingSquid->calc();
	}
	Game::MainMgr *mainMgr;
	Game::Player *tmp;
	tmp = player;
	asm("MOV X19, X0");
	mainMgr = Game::MainMgr::sInstance;
	asm("MOV X8, X0");
}

// Reimplementation of Game::SighterTarget::calcMarkingEffectPos_ (removed in 5.5.2)
// Returns the marking effect position: direction * modelScale + offset
// Offsets confirmed via 5.5.2 calcMarked_ (sub_7100B8A45C)
sead::Vector3<float> Game::SighterTarget_calcMarkingEffectPos(Game::SighterTarget *sighterTarget) {
	u8 *st = (u8 *)sighterTarget;

	// Get merged params -> family params -> sub-object -> vtable call for model scale
	// this + 0x4D0 -> [+0x10] + 0x5A0 -> vtable[0xD8/8=27]() returns float*
	u64 mergedParams = *(u64 *)(st + 0x4D0);
	u64 familyParams = *(u64 *)(mergedParams + 0x10);
	u64 subObj = familyParams + 0x5A0;
	float *scalePtr = ((float *(*)(u64))(*(u64 *)(*(u64 *)subObj + 0xD8)))(subObj);
	float scale = *scalePtr;

	// direction: bytes 900-908 (0x384-0x38C)
	// offset pos: bytes 924-932 (0x39C-0x3A4)
	sead::Vector3<float> result;
	result.mX = scale * *(float *)(st + 0x384) + *(float *)(st + 0x39C);
	result.mY = scale * *(float *)(st + 0x388) + *(float *)(st + 0x3A0);
	result.mZ = scale * *(float *)(st + 0x38C) + *(float *)(st + 0x3A4);
	return result;
}

// Reimplementation of Game::SighterTarget::startAllMarking(int, int) (removed in 5.5.2)
// Offsets confirmed via 5.5.2 calcMarked_ (sub_7100B8A45C)
void Game::SighterTarget_startAllMarking(Game::SighterTarget *sighterTarget, int a2, int a3) {
	u8 *st = (u8 *)sighterTarget;

	// Check state (DWORD 356 = 0x590) - skip if in states 2, 3, or 4
	u32 state = *(u32 *)(st + 0x590);
	if ((state - 2) < 3)
		return;

	// Marking icon duration after appearing: 0x21C = 540 frames (9s), matching player marking
	*(u32 *)(st + 0x66C) = 0x21C;

	// Get controlled player
	Game::Player *player = Game::PlayerMgr::sInstance->getControlledPerformer();

	// Calculate marking effect position
	sead::Vector3<float> effectPos = Game::SighterTarget_calcMarkingEffectPos(sighterTarget);

	// Store effect position at bytes 1720-1728 (0x6B8-0x6C0)
	*(sead::Vector3<float> *)(st + 0x6B8) = effectPos;

	// Get player barrier center position, store at bytes 1708-1716 (0x6AC-0x6B4)
	sead::Vector3<float> barrierPos;
	player->calcBarrier_CenterPos(&barrierPos);
	*(sead::Vector3<float> *)(st + 0x6AC) = barrierPos;

	// Calculate distance between barrier center and effect position
	float dx = barrierPos.mX - effectPos.mX;
	float dy = barrierPos.mY - effectPos.mY;
	float dz = barrierPos.mZ - effectPos.mZ;
	float dist = sqrtf(dx * dx + dy * dy + dz * dz);

	// Map distance to search line arc height: [0..600] -> [2.0..5.0]
	float searchLineValue;
	if (dist <= 0.0f) {
		searchLineValue = 2.0f;
	} else if (dist >= 600.0f) {
		searchLineValue = 5.0f;
	} else {
		searchLineValue = (dist * 3.0f / 600.0f) + 2.0f;
	}

	// Store search line value at byte 1732 (0x6C4)
	*(float *)(st + 0x6C4) = searchLineValue;

	// Distance-based countdown matching player startMarkingOne_Impl formula:
	// Maps distance [0..600] -> countdown [20..60] frames.
	int countdown;
	if (dist <= 0.0f) {
		countdown = 20;
	} else if (dist >= 600.0f) {
		countdown = 60;
	} else {
		countdown = (int)((dist * 40.0f / 600.0f) + 20.0f);
	}
	// Set both SearchLine travel countdown AND marking start countdown to the same
	// distance-based value, so the "Marking" effects appear exactly when the
	// SearchLine visual reaches the target (calcMarked_ decrements both each frame).
	*(u32 *)(st + 0x670) = countdown;  // marking start countdown
	*(u32 *)(st + 0x6A8) = countdown;  // SearchLine travel countdown

	// Emit "SearchLine" XLink effect
	xlink2::Handle handle;
	sighterTarget->mXLink->searchAndEmitWrap("SearchLine", false, &handle);

	// Store handle at bytes 1736/1744 (0x6C8/0x6D0)
	*(u64 *)(st + 0x6C8) = (u64)handle.mEvent;
	*(u32 *)(st + 0x6D0) = handle.mEventId;
}

// Reimplementation of Game::PlayerEffect::emitAndPlay_SuperArmorInvoke but for AllMarking
void emitAndPlay_AllMarkingInvoke(Game::PlayerEffect *effect) {
	if(effect == NULL || effect->mPlayer == NULL || effect->mPlayer->mXLink == NULL) return;
	xlink2::Handle handle;
	effect->mPlayer->mXLink->searchAndEmitWrap("SWpAllMarking", false, &handle);
	effect->mPlayer->mXLink->searchAndPlayWrap("AllMarkingStartCtrl", false, &handle);
}

// Hook for Game::Player::startAllMarking_Impl to add SighterTarget marking
void startAllMarking_ImplHook(Game::Player *player, int a1) {
	// If this is the local player activating, send the network event
	if(!player->mIsRemote){
		sendEvent_AllMarking(player->mPlayerNetControl, a1);
	}

	// Call AllMarking invoke effect
	if(player->mPlayerEffect != NULL){
		emitAndPlay_AllMarkingInvoke(player->mPlayerEffect);
	}

	// Call original startAllMarking_Impl
	player->startAllMarking_Impl(a1);

	// Iterate all active SighterTargets and call startAllMarking on enemy ones
	Cmn::Def::Team myTeam = player->mTeam;
	auto iterNode = Game::SighterTarget::getClassIterNodeStatic();
	for (Game::SighterTarget *st = (Game::SighterTarget *)iterNode->derivedFrontActiveActor();
		 st != NULL;
		 st = (Game::SighterTarget *)iterNode->derivedNextActiveActor(st))
	{
		// Only mark enemy SighterTargets (different team)
		if (st->mTeam != myTeam) {
			// a2 = marking ID, a3 = countdown frames (~1.5s at 60fps)
			Game::SighterTarget_startAllMarking(st, a1 + 0x21C, 90);
		}
	}
}

// Sends AllMarking activation event to remote consoles via state event system.
// Uses packet type 57 with game frame at DWORD[6] (bytes 24-27).
void sendEvent_AllMarking(Game::PlayerNetControl *netCtrl, int gameFrame){
	if(netCtrl == NULL || netCtrl->mCloneHandle == NULL) return;
	Game::PlayerCloneHandle *handle = netCtrl->mCloneHandle;
	if(handle->mCloneObjMgr->mIsOfflineScene) return;
	Game::PlayerStateCloneEvent event;
	memset(&event, 0, sizeof(event));
	event._data[32] = PACKET_ALL_MARKING;
	*(int*)(event._data + 24) = gameFrame;
	handle->mPlayerCloneObj->pushPlayerStateEvent(event);
}

// Reimplementation of Game::Player::startSpecial_AllMarking (removed in 5.5.2)
// Replaces stubbed sendSignal_AllMarking with state event send.
void Game::Player::startSpecial_AllMarking() {
	int gameFrame = Game::MainMgr::sInstance->mPaintGameFrame;
	sendEvent_AllMarking(this->mPlayerNetControl, gameFrame);
	startAllMarking_ImplHook(this, gameFrame);
}

// Reimplementation of Game::Player::receiveAllMarking (removed in 5.5.2)
// Matches 3.1.0 behavior: calls startAllMarking_Impl directly for remote clones.
// The hook's extra effects (SighterTarget marking, AllMarking invoke VFX) are only
// for the local activation path — remote clones just need the core marking logic.
void Game::Player::receiveAllMarking(int a2) {
	if (this->mIsRemote) {
		emitAndPlay_AllMarkingInvoke(this->mPlayerEffect);
		this->startAllMarking_Impl(a2);
	}
}

xlink2::UserInstanceSLink *startSkill_DeathMarkingHook(Game::Player *player, unsigned int a2, char a3) {
    xlink2::Handle v8;

    Game::Player *PerformerAt = Game::PlayerMgr::sInstance->getPerformerAt(a2);

    // ShowDL passes gameFrame-based args to startMarkingOne_Impl:
    //   m2 (default): arg3 = gameFrame + 0x21C, arg4 = gameFrame
    //   m3 (a3 & 1):  arg3 = gameFrame + 0x78,  arg4 = gameFrame
    // Without these, the marking duration is zero and the timer is never set.
    int gameFrame = (int)Game::MainMgr::sInstance->mPaintGameFrame;
    Game::Player::MarkingType markType = (a3 & 1) ? Game::Player::MarkingType::m3
                                                   : Game::Player::MarkingType::m2;
    int frameOffset = (a3 & 1) ? 0x78 : 0x21C;
    player->startMarkingOne_Impl(PerformerAt, markType, gameFrame + frameOffset, gameFrame);

    if (!player->mIsRemote) {
        int *netCtrlField = (int *)((u8 *)player->mPlayerNetControl + 132);
        *netCtrlField |= 0x20;
    }

    player->mXLink->searchAndPlayWrap("MarkingDeathAttackStart", false, &v8);
    return (xlink2::UserInstanceSLink *)v8.mEvent;
}


static xlink2::Event *gLaserIconEvent = NULL;
static u32 gLaserIconEventId = 0;
static xlink2::Event *gArtIconSndEvent = NULL;
static u32 gArtIconSndEventId = 0;

void updateCursorEffectHook(Game::MiniMap *miniMap) {
	Game::Player *player = Game::PlayerMgr::sInstance->getControlledPerformer();
	bool useArtillery = (player != NULL && tornadoMgr->playerState[player->mIndex] == Flexlion::TornadoState::cAim);
	bool aHeld = (player != NULL) && Lp::Utl::getCtrl(0)->isHoldContinue(starlight::Controller::Buttons::A, 1);
	bool aimValid = useArtillery && tornadoMgr->mAimValid[player->mIndex] && Utils::isShowMinimap() && aHeld;
	bool laserValid = (gLaserIconEvent != NULL) && (*(u32*)((u8*)gLaserIconEvent + 32) == gLaserIconEventId);
	bool sndValid = (gArtIconSndEvent != NULL) && (*(u32*)((u8*)gArtIconSndEvent + 32) == gArtIconSndEventId);

	// Fade Icon and ArtilleryIcon sound when leaving artillery or when aim becomes invalid
	if(!aimValid && laserValid) {
		gLaserIconEvent->fade(-1);
		gLaserIconEvent = NULL;
		gLaserIconEventId = 0;
		laserValid = false;
	}
	if(!aimValid && sndValid) {
		gArtIconSndEvent->fade(-1);
		gArtIconSndEvent = NULL;
		gArtIconSndEventId = 0;
		sndValid = false;
	}

	// When not in artillery, or aim is invalid, let the original run (shows DoronCursor)
	if(!useArtillery || !aimValid) {
		updateCursorEffectOrig(miniMap);
		return;
	}

	// Aim is valid: suppress DoronCursor, show Icon instead
	u8 *self = (u8*)miniMap;
	u64 doronEvent = *(u64*)(self + 3776);
	if(doronEvent != 0 && *(u32*)(doronEvent + 32) == *(u32*)(self + 3784)) {
		((xlink2::Event*)doronEvent)->fade(-1);
	}
	*(u64*)(self + 3776) = 0;
	*(u32*)(self + 3784) = 0;

	if(!laserValid) {
		Flexlion::BulletSuperArtillery *bsa = tornadoMgr->bullets[player->mIndex];
		Lp::Sys::XLink *xlink = (bsa != NULL) ? bsa->getXLink() : NULL;
		xlink2::Handle handle;
		if(xlink != NULL) xlink->searchAndEmitWrap("Icon", false, &handle);
		gLaserIconEvent = handle.mEvent;
		gLaserIconEventId = handle.mEventId;
		laserValid = (gLaserIconEvent != NULL) && (*(u32*)((u8*)gLaserIconEvent + 32) == gLaserIconEventId);
	}
	if(!sndValid) {
		Lp::Sys::XLink *mapXLink = *(Lp::Sys::XLink **)((u8*)miniMap + 0x320);
		if(mapXLink != NULL) {
			xlink2::Handle sndHandle;
			mapXLink->searchAndPlayWrap("ArtilleryIcon", false, &sndHandle);
			gArtIconSndEvent = sndHandle.mEvent;
			gArtIconSndEventId = sndHandle.mEventId;
		}
	}
	if(laserValid) {
		Flexlion::BulletSuperArtillery *bsa = tornadoMgr->bullets[player->mIndex];
		if(bsa != NULL) {
			const float halfCanvas = 360.0f;
			float halfFovyRad = tornadoMgr->camerafovy * 0.5f * MATH_PI / 180.0f;
			float tanHalfFovy = sinf(halfFovyRad) / cosf(halfFovyRad);
			float worldPerCanvas = tornadoMgr->cameraheight * tanHalfFovy / halfCanvas;
			sead::Vector3<float> camAt = miniMap->mMiniMapCamera->mAt;
			bsa->mXLinkMtx = {{
				1.0f, 0.0f, 0.0f, camAt.mX + miniMap->mCursorPos.mX * worldPerCanvas,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, camAt.mZ - miniMap->mCursorPos.mY * worldPerCanvas
			}};
		}
	}
}

void inkstrikeNetHook(u64 *x0, u32 w1, u64 *x2, u64 *x3, u32 w4, u32 w5){
	if(!tornadoMgr->isShot) ((void (*)(u64*, u32, u64*, u64*, u32, u32))ProcessMemory::MainAddr(0x4CEE6C))(x0, w1, x2, x3, w4, w5);
}
sead::Vector3<float> inkstrikeBombVelHook(Game::PlayerInkAction *inkAction){
	sead::Vector3<float> vec = inkAction->getShotVel_BombStd();
	if(tornadoMgr->isShot){
		vec.mX=0.0f;
		vec.mY=-5.0f;
		vec.mZ=0.0f;
	}
	return vec;
}

void inkstrikeShotHook(Game::BulletSpSuperBall *ball, Game::Player *sender, int senderId, int senderId2, sead::Vector3<float> *startpos, Game::SuperBallShotArg2 *arg2, int poop){
	Flexlion::BulletSuperArtillery *bullet = tornadoMgr->bullets[sender->mIndex];
	if(bullet and tornadoMgr->isShot) *startpos = bullet->mPos;
	ball->shot(sender, senderId, senderId2, startpos, arg2, poop);
}

bool isInInkstrikeCarryHook(Game::Player *player){
	return (player->isInSpecial() and player->mSpecialWeaponId == TORNADO_SPECIAL_ID) and tornadoMgr->playerState[player->mIndex] != Flexlion::TornadoState::cNone;
}

sead::Heap *npcHeapHook(sead::HeapMgr *a1, sead::Heap *a2){
	sead::Heap *heap = a1->setCurrentHeap_(a2);
	FsLogger::LogFormatDefaultDirect("[Dbg] Npc Creation, Free Heap Space: %i\n", a2->getFreeSize());
	return heap;
}

sead::HeapMgr *npcHeapFix(sead::HeapMgr *a1, sead::Heap *a2){
	if(a2 != NULL){
		if(a2->getFreeSize() < 20000000){
			a2 = NULL;
		}
	}
	sead::Heap *poop;
	poop = a2;
	asm("MOV X1, X0");
	return a1;
}
 
void hooks_init(){
    autoMatchTest();
    rivalOctohook(Cmn::Def::PlayerModelType::Rival);
	weaponFixHook(NULL, sead::SafeStringBase<char>::create("bruh"));
	playerModelDrawHook(NULL, NULL);
	krakenDiveHook(0);
	isInKingSquidHook(NULL);
	renderEntrypoint(NULL, NULL);
	extraBigLaserBulletHook(NULL);
	bulletSuperLaserShotHook(NULL, NULL, 0, 0, NULL, NULL, 0);
	bigLaserItemPickupHook(NULL, 0);
	bigLaserSetupWithModelHook(NULL);
	weaponModelPreRegHook(NULL, 0, 0, 0);
	bulletSuperLaserGetClassNameOverride();
	jetPackJetHook(0);
	curl_easy_perform_hook(NULL);
	inkstrikeBombVelHook(NULL);
	inkstrikeShotHook(NULL, NULL, 0, 0, NULL, NULL, 0);
	bcatHook(0);
	npcHeapFix(NULL, NULL);
	isInLauncherHook(NULL);
	Game::SighterTarget_startAllMarking(NULL, 0, 0);
	startAllMarking_ImplHook(NULL, 0);
	sendEvent_AllMarking(NULL, 0);
	updateCursorEffectHook(NULL);
	startSkill_DeathMarkingHook(NULL, 0, 0);
	inkstrikeNetHook(NULL, 0, NULL, NULL, 0, 0);
	playerModelSetupHook(NULL);
	playerKingSquidCalcHook(NULL);
	handleDisplayVersion(NULL);
	registKingSquidAnimHook(NULL, 0, NULL, 0, 0);
	setupKingSquidAnimHook(NULL, NULL);
	kingSquidAnimSetControllerHook(NULL, NULL);
	actorDbHook(NULL, NULL, NULL);
	Game::PlayerWeaponSuperShot::supershotJumpHook();
	Flexlion::BigLaserModeMgr::bigLaserJumpHook();
	custommgrjptHook();
	specialSetupWithoutModelHook();
	stepPaintTypeHook(NULL);
	fixEffHook(NULL);
	playerModelResourceLoadHook(NULL, NULL);
	createPlayerModelHook(Cmn::Def::Team::Alpha, sead::SafeStringBase<char>::create("test"), *(Lp::Sys::ModelArc*)NULL, *(Lp::Utl::ModelCreateArg*)NULL, NULL);
	damageReasonHook(NULL, *(Game::DamageReason*)NULL, NULL, 0, 0, 0);
	GetCharKindHook(NULL, 0);
	isInInkstrikeCarryHook(NULL);
	handleBulletCloneEventHook(NULL, NULL, NULL, 0);
	unpackStateEventHook(NULL, NULL, 0);
	sendEvent_StartBarrierHook(NULL, 0, 0);
	playDamageVoiceAndRumbleHook(NULL, *(Game::DamageReason*)NULL, 0);
	emitAndPlay_StealthDamageHook(NULL, 0, (Cmn::Def::DMG)0, *(Game::DamageReason*)NULL);
	PlaySuperArmorUse();
	healPlayerSuperLandingHook(NULL);
	createHumanModelHook(sead::SafeStringBase<char>::create("test"), Cmn::Def::Team::Alpha, *(Game::PlayerModelResource*)NULL,  *(Lp::Utl::ModelCreateArg*)NULL,  *(Lp::Utl::AnimCreateArg*)NULL, Cmn::Def::PlayerModelType::InkGirl, *(sead::RingBuffer<int>*)NULL);
	msnArmorHook(0, Cmn::Def::GearKind::cHead, 0);
	CustomizeSeqBaseLoadHook(NULL, NULL);
	CustomizeAmiiboCbHelperFunc();
	LobbyRivalFixHook(NULL);
	LobbyRivalGetPlayerTypeHook(NULL);
	isInSpecialForShotGuideHook(NULL);
}

int LobbyRivalGetPlayerTypeHook(Cmn::SaveDataCmn *saveDataCmn){
	int r = saveDataCmn->getSquidOrOctaPlayerModelType();
	return (r == 6 ? 4 : r);
}

void LobbyRivalFixHook(Lobby::MainMgr *mainMgr){
	mainMgr->createActor();
	mainMgr->mRivalLobbyPlayer = Lp::Sys::Actor::create<Lobby::Player>((Lp::Sys::Actor*)mainMgr, NULL);
	memset(mainMgr->mRivalLobbyPlayer->_480, 0, sizeof(mainMgr->mRivalLobbyPlayer->_480));
	mainMgr->mRivalLobbyPlayer->mGearHeap = mainMgr->mGearHeapForEachModel;
	mainMgr->mRivalLobbyPlayer->mModelType = Cmn::Def::PlayerModelType::Rival;
}
void CustomizeSeqBaseLoadHook(Cmn::CustomizeSeqBase *seqBase, sead::Heap *heap){
	seqBase->mRivalCustomizePlayer = Lp::Sys::Actor::create<Cmn::CustomizePlayer>((Lp::Sys::Actor*)seqBase, heap);
	memset(seqBase->mRivalCustomizePlayer->_480, 0, sizeof(seqBase->mRivalCustomizePlayer->_480));
	seqBase->mRivalCustomizePlayer->mGearHeap = seqBase->mGearHeapForEachModel;
	seqBase->mRivalCustomizePlayer->mModelType = Cmn::Def::PlayerModelType::Rival;
	seqBase->mRivalCustomizePlayer->mCustomizeSeqBase = seqBase;
}

void CustomizeAmiiboCbHelperFunc(){
	asm("LDR W8, [X19, #0x3EC]");
	asm("CMP W8, #0x6");
	asm("BNE #0xC");
	asm("MOV W8, #0x4");
	asm("STR W8, [X19, #0x3EC]");
	asm("LDR W8, [X19,#0x414]");
	asm("RET");
}

ushort GetCharKindHook(uintptr_t _this, ushort charKind){
	switch(charKind){
	case 0x32:
		return 0xE087;
	case 0x33:
		return 0xE089;
	case 0x34:
		return 0xE0B5;    
	case 0x35:
		return 0xE0C0;
	case 0x36:
		return 0xE0C1;
	case 0x37:
		return 0xE0B6;
	};
	return ((ushort (*)(uintptr_t, ushort))ProcessMemory::MainAddr(0x423FF4))(_this, charKind);
}

void damageReasonHook(Game::VersusBeatenPage *page, Game::DamageReason &reason, Cmn::PlayerInfo const*attackerInfo, int dieFrm, int uiAppearFrm, bool iwannaslep){
	Cmn::Def::WeaponKind kind = Cmn::Def::WeaponKind(reason.mClassType << 26 >> 26); // clear unnecessary bytes
	if(attackerInfo != NULL and (kind == Cmn::Def::WeaponKind::cMain or reason.mClassType == 9 or reason.mClassType == 12 or reason.mClassType == 4 or reason.mClassType == 6)){
		Game::Player *player = Utils::getPerformerById(attackerInfo->mPlayerIndex);
		if(player != NULL){
			Starlion::Sp1WeaponLookup::getLookupRefId(&reason.mWeaponId, player->mMainWeaponId, reason.mWeaponId);
		}
	}
	page->start(reason, attackerInfo, dieFrm, uiAppearFrm, iwannaslep);
}

static Lp::Sys::ModelArc *inkGirlHalfNewArc = NULL;
static Lp::Sys::ModelArc *rivalHalfArc = NULL;
static Lp::Sys::ModelArc *rivalFullArc = NULL;

void playerModelResourceLoadHook(Game::PlayerModelResource *res, sead::Heap *heap){
	inkGirlHalfNewArc = NULL;
	rivalHalfArc = NULL;
	rivalFullArc = NULL;
	res->load(heap);
	inkGirlHalfNewArc = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Player00_Hlf"), heap, 0, NULL, NULL);
	rivalHalfArc = (Lp::Sys::ModelArc *)res->mHalfArcs.mPtr[6];
	if(rivalHalfArc == NULL) rivalHalfArc = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Rival00_Hlf"), heap, 0, NULL, NULL);
	rivalFullArc = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Player04"), heap, 0, NULL, NULL);
}

gsys::Model *createHumanModelHook(sead::SafeStringBase<char> const& name, Cmn::Def::Team team, Game::PlayerModelResource &res, const Lp::Utl::ModelCreateArg &arg, Lp::Utl::AnimCreateArg const&animarg, Cmn::Def::PlayerModelType modeltype, sead::RingBuffer<int> const&hairinfo){
	register Game::PlayerModel *x19 asm("x19");
	u64 tmparc = res.mHumanArcs.mPtr[2];
	if(modeltype == Cmn::Def::PlayerModelType::OctGirl and x19->mPlayer->mPlayerInfo->mModelId == 54){
		res.mHumanArcs.mPtr[2] = (u64)rivalFullArc;
	}
	gsys::Model *model = Cmn::PlayerCustomUtl::createPlayerModelHuman(name, team, res, arg, animarg, modeltype, hairinfo);
	res.mHumanArcs.mPtr[2] = tmparc;
	return model;
}

gsys::Model *createPlayerModelHook(Cmn::Def::Team team,sead::SafeStringBase<char> const& name,Lp::Sys::ModelArc &arc,Lp::Utl::ModelCreateArg const&createArg,sead::Heap *heap){
	Game::PlayerModelResource *res = &Game::PlayerMgr::sInstance->mModelResource;
	Lp::Sys::ModelArc *useArc = &arc;
	register Game::PlayerModel *x19 asm("x19");
	if(((Lp::Sys::ModelArc *)res->mHalfArcs.mPtr[0]) == useArc and x19->mPlayer->mPlayerInfo->mModelId == 0){
		useArc = inkGirlHalfNewArc;
	} else if(((Lp::Sys::ModelArc *)res->mHalfArcs.mPtr[2]) == useArc and x19->mPlayer->mPlayerInfo->mModelId == 54){ // nacho asked to add this for his hair
		useArc = rivalHalfArc;
	}
	gsys::Model *model = Cmn::GfxUtl::createModel(team, name, *useArc, createArg, heap);
	return model;
}

int stepPaintTypeHook(Game::PlayerStepPaint *step){
	Game::Player *player = step->mPlayer;
	if(player->isInSpecial_KingSquid_Impl(0) and player->mPlayerKingSquid != NULL){
		Starlion::PlayerKingSquid *kingSquid = (Starlion::PlayerKingSquid *)player->mPlayerKingSquid;
		return !kingSquid->mIsRush;
	}
	return step->mStepPaintType;
}

bool fixEffHook(Game::Player *player){
	return (player->isInSquid_Stealth_Move() and !player->isInSpecial_KingSquid_Impl(0));
}

void actorDbHook(Lp::Utl::ByamlIter *iter, const char **a, const char *b){
	iter->tryGetStringByKey(a, b);
	if(strcmp(*a, "TestObj_Lift_Ikkaku112") == 0){
		memcpy((void*)(*a), "Obj_Box00S", strlen("Obj_Box00S") + 1);
	}
}

void bcatHook(int *res){
	int poop;
	register int w8 asm("w8");
	int p2 = *res;
	if(-1 >= 0){
		p2 = 2;
	}
	poop = p2;
	asm("MOV W8, W0");
}

void allocHeap() {
    Lp::Sys::HeapGroup* heapGroup = Lp::Sys::HeapGroup::sInstance;
    if(heapGroup != NULL){
        sead::ExpHeap* othersHeap = heapGroup->mHeaps[Lp::Sys::HeapGroup::Group::cOthers];
        Collector::mHeapMgr->setCurrentHeap_(othersHeap);
        sead::Heap* currentHeap = Collector::mHeapMgr->getCurrentHeap();

        if(mStarlightHeap == NULL){
            sead::SafeStringBase<char> str;
            str = (char*)"StarlightHeap";

            mStarlightHeap = sead::ExpHeap::create(0, str, currentHeap, 4, sead::Heap::HeapDirection::TAIL, true);
        }
    }
}

Game::BulletMgr *extraBigLaserBulletHook(Game::BulletMgr *mgr){
	register u64 x1 asm("x1");
	Cmn::Def::Mode mode = Cmn::Def::getCurMode();
	if(mode == Cmn::Def::Mode::cPlaza or mode == Cmn::Def::Mode::cPlayerMake or mode == Cmn::Def::Mode::cStaffRoll or mode == Cmn::Def::Mode::cStaffRollOcta or mode == Cmn::Def::Mode::cTwoShot or mode == Cmn::Def::Mode::cCustomize){
		x1 = 0;
		return mgr;
	}
	Cmn::PlayerInfoAry *ary = Cmn::StaticMem::sInstance->mPlayerInfoAry;
	Flexlion::BigLaserModeMgr::resetBulletPools();
	Flexlion::BigLaserModeMgr::resetWeaponTracking();
	if(mode != Cmn::Def::Mode::cVersus){
		// Split pool: Killer Wail bullets (BulletOldSuperLaser XLink + OldBigLaser model)
		// Model swap happens in bulletLoadHook (vtable hook on BulletSuperLaser::load)
		Flexlion::BigLaserModeMgr::sCreateAsPrincessCannon = false;
		for(int i = 0; i < ary->getValidInfoNum(); i++){
			void *b = (void*)Lp::Sys::Actor::create<Game::BulletSuperLaser>(mgr->getBulletParent(), NULL);
			Flexlion::BigLaserModeMgr::registerBullet(b, false);
		}
		// Split pool: Princess Cannon bullets (BulletSuperLaser XLink + BigLaser model)
		Flexlion::BigLaserModeMgr::sCreateAsPrincessCannon = true;
		for(int i = 0; i < ary->getValidInfoNum(); i++){
			void *b = (void*)Lp::Sys::Actor::create<Game::BulletSuperLaser>(mgr->getBulletParent(), NULL);
			Flexlion::BigLaserModeMgr::registerBullet(b, true);
		}
		Flexlion::BigLaserModeMgr::sCreateAsPrincessCannon = false;
		if((mode != Cmn::Def::Mode::cMission or strcmp(Cmn::StaticMem::sInstance->mMapFileName1.mCharPtr, "Fld_BossLastKing_Msn") != 0) and mode != Cmn::Def::Mode::cMissionOcta){
			for(int i = 0; i < 16; i++){
				Lp::Sys::Actor::create<Game::BulletGachihoko>(mgr->getBulletParent(), NULL);
			}
			for(int i = 0; i < 16 * 20; i++){
				Lp::Sys::Actor::create<Game::BulletGachihokoSplash>(mgr->getBulletParent(), NULL);
			}
		}
		if(mode == Cmn::Def::Mode::cMission){
			for(int i = 0; i < ary->getValidInfoNum(); i++){
				Lp::Sys::Actor::create<Game::BulletSpJetpackJet>(mgr->getBulletParent(), NULL);
				Lp::Sys::Actor::create<Game::BulletSpSuperBall>(mgr->getBulletParent(), NULL);
			}
		}
	} else{
		// Split pool: KW and PC bullets for versus
		// Model swap happens in bulletLoadHook (vtable hook on BulletSuperLaser::load)
		Flexlion::BigLaserModeMgr::sCreateAsPrincessCannon = false;
		for(int i = 0; i < ary->getValidInfoNum(); i++){
			void *b = (void*)Lp::Sys::Actor::create<Game::BulletSuperLaser>(mgr->getBulletParent(), NULL);
			Flexlion::BigLaserModeMgr::registerBullet(b, false);
		}
		Flexlion::BigLaserModeMgr::sCreateAsPrincessCannon = true;
		for(int i = 0; i < ary->getValidInfoNum(); i++){
			void *b = (void*)Lp::Sys::Actor::create<Game::BulletSuperLaser>(mgr->getBulletParent(), NULL);
			Flexlion::BigLaserModeMgr::registerBullet(b, true);
		}
		Flexlion::BigLaserModeMgr::sCreateAsPrincessCannon = false;
		for(int i = 0; i < ary->getValidInfoNum(); i++){
			Lp::Sys::Actor::create<Game::BulletSpAquaBall>(mgr->getBulletParent(), NULL);
			for(int j = 0; j < 2; j++){
				Lp::Sys::Actor::create<Game::BulletSlosherBase>(mgr->getBulletParent(), NULL);
			}
			for(int j = 0; j < 8; j++){
				Lp::Sys::Actor::create<Game::BulletSlosherSplash>(mgr->getBulletParent(), NULL);
			}
			Lp::Sys::Actor::create<Game::BulletSpJetpackJet>(mgr->getBulletParent(), NULL);
			Lp::Sys::Actor::create<Game::BulletSpSuperBall>(mgr->getBulletParent(), NULL);
		}
		for(int i = 0; i < 8; i++){
			Lp::Sys::Actor::create<Game::BulletGachihoko>(mgr->getBulletParent(), NULL);
		}
		for(int i = 0; i < 8 * 20; i++){
			Lp::Sys::Actor::create<Game::BulletGachihokoSplash>(mgr->getBulletParent(), NULL);
		}
	}
	x1 = 0;
	return mgr;
}


bool jetPackJetHook(Game::Player *player){
	if(player != NULL){
		return ((player->isInSpecial_KingSquid_Impl(0) and Prot::ObfLoad(&player->mAerialState) == 0) or player->mPlayerJetpack->isInAirWalk());
	}
	return 0;
}

int weaponFixHook(gsys::Model *model, sead::SafeStringBase<char> lol){
	if(model != NULL){
		return model->searchBone(lol);
	} else{
		return 0xFFFFFFFF;
	}
}

void playerModelDrawHook(Cmn::PlayerWeapon *playerWeapon, sead::Matrix34<float> *mtx){
	// Model re-setup is now done in bigLaserItemPickupHook (game logic phase).
	// Calling sBaseSetupWithModel during draw phase crashes (NULL model from heap/render state mismatch).
	playerWeapon->getRootBoneMtx(mtx);
}

bool clamCalcSleepHook(Game::VictoryClamHolding *clam){
	if(clam->clamDirector != NULL){
		return (*(int (**)(Game::VictoryClamHolding *clam))(*(u64*)(clam) + (0x5B8)))(clam);
	}
	return 0;
}

void playReportHook(nn::prepo::PlayReport *playReport, nn::account::Uid const& uid){
	if(playReport != NULL){
		playReport->Save(uid);
	}
}

static int mRushEndFrame = 0;

bool isInKingSquidHook(Game::Player *player){
	return player->isInSpecial_KingSquid_Impl(0) or krakenDiveHook(player);
}

bool krakenDiveHook(Game::Player *player){
	Starlion::PlayerKingSquid *kingSquid = ((Starlion::PlayerKingSquid*)player->mPlayerKingSquid);
	if(kingSquid == NULL){
		return 0;
	}
	return kingSquid->mIsRush and Prot::ObfLoad(&player->mAerialState) != 0 and player->isInSpecial_KingSquid_Impl(0);
}

void handlePlayerControl(){
    Cmn::PlayerCtrl *playerCtrl = Collector::mPlayerCtrlInstance;
    Game::PlayerMgr *playerMgr = Collector::mPlayerMgrInstance;
    if(playerMgr == NULL) return; // Null check
    
    Game::Player *player = playerMgr->getControlledPerformer();
    if (player == NULL) return;

    Game::PlayerBehindCamera *playerCamera = player->mPlayerBehindCamera;
    if (playerCamera == NULL) return;

    Game::PlayerGamePadData::FrameInput input;
}

int main(int arg, char **argv){

}