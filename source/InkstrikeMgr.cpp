#include "flexlion/InkstrikeMgr.hpp"
#include "flexlion/BigLaserModeMgr.hpp"
#include "flexlion/BulletSuperArtillery.hpp"
#include "Game/RespawnPoint.h"
// #include "flexlion/FsLogger.hpp"
const int startflightdelay = 40; // delay from when a point is chosen to when the tornado is actually launched
const int playerdelay = 60; // 60 frames Shoot_Tornado waiting for the player to transition from tornado cShoot to cNone, matches Splatoon 1 timing
const float tornadoTankZOffset = -3.0f;

extern "C" {
    extern u8 _ZTVN4Game27MessagePlayerPerformSpecialE[];
    extern void *_ZN3Cmn18MessageBroadcaster9sInstanceE;
    void _ZN3Cmn17MessageDispatcher15dispatchMessageERKNS_7MessageE(void *, const void *);
}
#define MessagePlayerPerformSpecialVtable _ZTVN4Game27MessagePlayerPerformSpecialE
#define MessageBroadcasterInstance _ZN3Cmn18MessageBroadcaster9sInstanceE
#define dispatchMessage _ZN3Cmn17MessageDispatcher15dispatchMessageERKNS_7MessageE


// Send PerformSpecial network event (type 22) so remote consoles also count the special.
static void sendPerformSpecialNet(Game::Player *player) {
    if (!player->mPlayerNetControl) return;
    Game::PlayerCloneHandle *handle = player->mPlayerNetControl->mCloneHandle;
    if (!handle) return;
    bool isOffline = !Game::MainMgr::sInstance || Game::MainMgr::sInstance->cloneObjMgr->mIsOfflineScene;
    if (isOffline) return;
    Game::PlayerCloneObj *cloneObj = handle->mPlayerCloneObj;
    if (!cloneObj) return;
    Game::PlayerStateCloneEvent event;
    memset(&event, 0, sizeof(event));
    event._data[32] = 22; // PerformSpecial event type
    cloneObj->pushPlayerStateEvent(event);
}


static void informPerformSpecial(Game::Player *player) {
    u64 msg[2];
    msg[0] = (u64)(MessagePlayerPerformSpecialVtable + 0x10);
    msg[1] = (u64)player;
    if (MessageBroadcasterInstance)
        dispatchMessage(MessageBroadcasterInstance, msg);
    sendPerformSpecialNet(player);
    player->informStartSpecialToLayout(0x11);
}

// Offsets verified from CameraVersusTopView::onCalc (SP 0x710117AB60).
// On MP 5.5.2 stripped, offsets in the class are identical — only the entry
// point addresses differ, and we only use offsets from already-loaded pointers.
static constexpr int kOff_TopView_CameraParam  = 0x178;  // (u64*)topView[47]
static constexpr int kOff_TopView_BravoInvType = 0x200;  // (int*)topView[128]
static constexpr int kOff_CamParam_Pos         = 0x98;
static constexpr int kOff_CamParam_At          = 0xD0;
static constexpr int kOff_CamParam_Up          = 0x108;
// ref_() lives at vtable slot 0xD8; returns (self + 0x2C) as Vector3<float>*
static constexpr int kOff_ParamVec3Body        = 0x2C;
static constexpr int kVtSlot_ParamVec3_Ref     = 0xD8;

// Returns a pointer to the Vector3<float> data inside a Param<Vector3> sub-object.
// Equivalent to calling the virtual ref_() but avoids indirect call and skips
// anything the vtable might do. Works because ref_() is a trivial "return this + 0x2C".
static inline const sead::Vector3<float>* getParamVec3Body(u64 paramNodeAddr) {
    return (const sead::Vector3<float>*)(paramNodeAddr + kOff_ParamVec3Body);
}

// Safely walks the SeqMgr → CameraHolder → TopView chain.
// Returns nullptr if we're not in a versus match or the chain isn't set up yet.
static u64 getTopViewCameraRaw() {
    u64 mainMgr = (u64)Game::MainMgr::sInstance;
    if (!mainMgr) return 0;
    u64 seqMgr = *(u64*)(mainMgr + 912);
    if (!seqMgr) return 0;
    u64 camHolder = *(u64*)(seqMgr + 1104);   // SeqMgrVersus + 0x450
    if (!camHolder) return 0;
    u64 topView = *(u64*)(camHolder + 0x30);  // CameraVersusTopView at index 6
    return topView;
}

// Returns centroid of RespawnPoints for the given team. outFound is set true if
// at least one was found.
static sead::Vector3<float> findTeamSpawnCentroid(int team, bool *outFound) {
    sead::Vector3<float> sum = sead::Vector3<float>::zero;
    int count = 0;
    auto iterNode = Game::RespawnPoint::getClassIterNodeStatic();
    for (Cmn::Actor *obj = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
         obj != NULL;
         obj = (Cmn::Actor *)iterNode->derivedNextActiveActor(obj))
    {
        if (*(int *)((u8 *)obj + 0x328) == team) {
            float *pos = (float *)((u8 *)obj + 0x39C);
            sum.mX += pos[0];
            sum.mY += pos[1];
            sum.mZ += pos[2];
            count++;
        }
    }
    if (outFound) *outFound = (count > 0);
    if (count > 0) {
        sum.mX /= (float)count;
        sum.mY /= (float)count;
        sum.mZ /= (float)count;
    }
    return sum;
}

// Returns the current stage's internal name (e.g. "Fld_Quarry00"). May return nullptr.
static const char* getCurrentMapName() {
    u64 mainMgr = (u64)Game::MainMgr::sInstance;
    if (!mainMgr) return nullptr;
    u64 staticMem = *(u64*)(mainMgr + 920);
    if (!staticMem) return nullptr;
    return *(const char**)(staticMem + 936);
}

// Case-sensitive prefix match helper
static bool strStartsWith(const char* s, const char* prefix) {
    if (!s || !prefix) return false;
    while (*prefix) {
        if (*s++ != *prefix++) return false;
    }
    return true;
}

namespace Flexlion{
    InkstrikeMgr *InkstrikeMgr::sInstance = NULL;
    InkstrikeMgr::InkstrikeMgr(){
        sInstance = this;
        memset(this, 0, sizeof(InkstrikeMgr));
        mTornadoArc = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Wsp_Tornado"), NULL, 0, NULL, NULL);
        mTornadoMonitorArc = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Wsp_Tornado_Monitor"), NULL, 0, NULL, NULL);
        cameraheight = 1500.0f;
        cameraanim = 0.0f;
        camerafovy = 60.0f;
		mSpawnY = 3000.0f;  // safe default
		mSpawnYCaptured = false;
		mCamUpX = -1.0f;  // default to vanilla-style if snapshot fails
		mCamUpZ = 0.0f;
		mNeedCamUpSnapshot = false;
        for(int i = 0; i < 10; i++){
			bullets[i][0] = NULL;
			bullets[i][1] = NULL;
			mActiveSlot[i] = -1;
            mTankRootBoneIdx[i] = -1;
            mAimValid[i] = false;
			mWasAHeld[i] = false;
			mMapOpen[i] = false;
        }
    }
	int InkstrikeMgr::pickFreeSlot(int playerId) {
    // Prefer an inactive slot
    for(int slot = 0; slot < 2; slot++){
        if(bullets[playerId][slot] != NULL && !bullets[playerId][slot]->isActive()){
            return slot;
        }
    }
    // Both active — shouldn't happen in normal play, but fall back to slot 0
    return 0;
	}
	
	BulletSuperArtillery *InkstrikeMgr::getActiveBullet(int playerId) {
		if(mActiveSlot[playerId] < 0) return NULL;
		return bullets[playerId][mActiveSlot[playerId]];
	}
	
	// Align Minimap horizontally
	void InkstrikeMgr::snapshotCamUp(sead::Vector3<float> camAt) {
		if (!mNeedCamUpSnapshot) return;
		
		// FsLogger::LogFormatDefaultDirect("[InkOri] enter, needSnap=%d\n",
		//     (int)mNeedCamUpSnapshot);
	
		bool alphaFound = false;
		bool bravoFound = false;
		sead::Vector3<float> alphaSpawn = findTeamSpawnCentroid(0, &alphaFound);
		sead::Vector3<float> bravoSpawn = findTeamSpawnCentroid(1, &bravoFound);
		// default to vanilla minimap orientation and skip the rest of the logic.
		if (!alphaFound || !bravoFound) {
			mCamUpX = 0.0f;
			mCamUpZ = -1.0f;  // vanilla-style default
			mNeedCamUpSnapshot = false;
			return;
		}
		// FsLogger::LogFormatDefaultDirect("[InkOri] alphaSpawn=(%d,%d,%d) found=%d\n",
		//     (int)alphaSpawn.mX, (int)alphaSpawn.mY, (int)alphaSpawn.mZ, (int)alphaFound);
		// FsLogger::LogFormatDefaultDirect("[InkOri] bravoSpawn=(%d,%d,%d) found=%d\n",
		//     (int)bravoSpawn.mX, (int)bravoSpawn.mY, (int)bravoSpawn.mZ, (int)bravoFound);
	
		float resultX = -1.0f;
		float resultZ = 0.0f;
	
		Game::Player *ctrlPlayer = Game::PlayerMgr::sInstance->getControlledPerformer();
		int bravoInvType = 0;
		u64 topView = getTopViewCameraRaw();
		u64 camParam = 0;
		if (topView != 0) {
			bravoInvType = *(int*)(topView + kOff_TopView_BravoInvType);
			camParam = *(u64*)(topView + kOff_TopView_CameraParam);
		}
		int team = (ctrlPlayer != NULL) ? (int)ctrlPlayer->mTeam : -1;
	
		// if (camParam != 0) {
		//     const sead::Vector3<float>* pos = getParamVec3Body(camParam + kOff_CamParam_Pos);
		//     const sead::Vector3<float>* at  = getParamVec3Body(camParam + kOff_CamParam_At);
		//     FsLogger::LogFormatDefaultDirect("[InkOri] cam pos=(%d,%d,%d) at=(%d,%d,%d)\n",
		//         (int)pos->mX, (int)pos->mY, (int)pos->mZ,
		//         (int)at->mX, (int)at->mY, (int)at->mZ);
		// }
	
		if (bravoInvType == 1 || bravoInvType == 2) {
			// Mirror map logic — same orientation for both teams
			if (alphaFound && bravoFound) {
				bool xShared = (alphaSpawn.mX >= 0.0f) == (bravoSpawn.mX >= 0.0f);
				bool zShared = (alphaSpawn.mZ >= 0.0f) == (bravoSpawn.mZ >= 0.0f);
	
				if (xShared && !zShared) {
					resultX = (alphaSpawn.mX >= 0.0f) ? -1.0f : 1.0f;
					resultZ = 0.0f;
				} else if (zShared && !xShared) {
					resultX = 0.0f;
					resultZ = (alphaSpawn.mZ >= 0.0f) ? -1.0f : 1.0f;
				}
				// else: ambiguous (both or neither axes shared) — fall back to default
	
				// If spawns are very close together (narrow/long map like First Deli),
				// rotate the orientation 90° clockwise so the long axis becomes horizontal.
				float dx = alphaSpawn.mX - bravoSpawn.mX;
				float dz = alphaSpawn.mZ - bravoSpawn.mZ;
				float spawnDist = sqrtf(dx * dx + dz * dz);
	
				// FsLogger::LogFormatDefaultDirect("[InkOri] spawnDist=%d\n", (int)spawnDist);
	
				if (spawnDist < 200.0f) {
					float newX = resultZ;
					float newZ = -resultX;
					resultX = newX;
					resultZ = newZ;
				}
			}
		} else {
			// Type 0 logic — axis dominance based on Alpha spawn
			if (alphaFound) {
				const char* mapName = getCurrentMapName();
				// Fld_Quarry is laid out inconsistent with other Z-dominant maps;
				// force Z-axis output to get correct orientation.
				bool forceZAxis = strStartsWith(mapName, "Fld_Quarry");
	
				// FsLogger::LogFormatDefaultDirect("[InkOri] map=%s forceZ=%d\n",
				//     mapName ? mapName : "(null)", (int)forceZAxis);
	
				if (forceZAxis) {
					resultX = 0.0f;
					resultZ = (alphaSpawn.mX >= 0.0f) ? 1.0f : -1.0f;
				} else if (fabsf(alphaSpawn.mX) > fabsf(alphaSpawn.mZ)) {
					// X-axis dominant: orient along Z
					resultX = 0.0f;
					resultZ = (alphaSpawn.mX >= 0.0f) ? 1.0f : -1.0f;
				} else {
					// Z-axis dominant: orient along X
					resultX = (alphaSpawn.mZ >= 0.0f) ? -1.0f : 1.0f;
					resultZ = 0.0f;
				}
	
				// Type 0 Bravo: 180° rotation
				if (team == 1) {
					resultX = -resultX;
					resultZ = -resultZ;
				}
			}
		}
	
		mCamUpX = resultX;
		mCamUpZ = resultZ;
	
		// FsLogger::LogFormatDefaultDirect(
		//     "[InkOri] FINAL team=%d bravoInv=%d mUp=(%d,0,%d)\n",
		//     team, bravoInvType,
		//     (int)mCamUpX, (int)mCamUpZ);
		
		mNeedCamUpSnapshot = false;
	}
    void InkstrikeMgr::informShotInkstrike(Game::Player *player, sead::Vector3<float> pos, sead::Vector3<float> dest, int paintgamefrm){
		int id = player->mIndex;
		if(mActiveSlot[id] < 0) return;
		BulletSuperArtillery *b = bullets[id][mActiveSlot[id]];
        if(b != NULL && !b->mFlightActive){
            // BSA is already active from prepare(), transition it to flight
            b->launch(pos, dest, paintgamefrm, mMatchEnding);
			isAppliedWeapon[id] = 0;
        }
    }
	static sead::Vector3<float> cursorToWorldXZ(
    const sead::Vector3<float>& camAt,
    const sead::Vector2<float>& cursorPos,
    float worldPerCanvas)
	{
		float Xu = InkstrikeMgr::sInstance->mCamUpX;
		float Zu = InkstrikeMgr::sInstance->mCamUpZ;
		// With camera looking -Y and up = (Xu, 0, Zu):
		//   screen_right = (-Zu, 0, Xu)   // canvas X-positive → world delta in this dir
		//   screen_up    = (Xu,  0, Zu)   // canvas Y-positive → world delta in this dir
		sead::Vector3<float> result;
		result.mX = camAt.mX + cursorPos.mX * (-Zu) * worldPerCanvas
							+ cursorPos.mY * (Xu)  * worldPerCanvas;
		result.mZ = camAt.mZ + cursorPos.mX * (Xu)  * worldPerCanvas
							+ cursorPos.mY * (Zu)  * worldPerCanvas;
		result.mY = 0.0f;
		return result;
	}
    void InkstrikeMgr::onCalc(){
        if(Game::MainMgr::sInstance == NULL or !Utils::isSceneLoaded()){
            if(!isBulletDeinit){
				for(int i = 0; i < 10; i++){
					for(int slot = 0; slot < 2; slot++){
						if(bullets[i][slot] != NULL){
							bullets[i][slot]->reset();
						}
					}
					mActiveSlot[i] = -1;
				}
                resetBSAStatics();
                isBulletDeinit = 1;
				mMatchEnding = false;
				mSpawnYCaptured = false;
				mSpawnY = 3000.0f;
            }
            return;
        }
        isBulletDeinit = 0;
    }
    void InkstrikeMgr::detectChangeState(Game::Player *player){
        int id = player->mIndex;
        if(player->isInSpecial() and player->mSpecialWeaponId == TORNADO_SPECIAL_ID and playerState[id] == TornadoState::cNone){
            playerState[id] = TornadoState::cAim;
            isAppliedWeapon[id] = 0;
			mRemoteShotPending[id] = false;
			mNeedCamUpSnapshot = true;
			mRemoteShotPending[id] = false;  // clear stale flag from previous activation
			// Pick a free BSA slot for this new activation
			mActiveSlot[id] = pickFreeSlot(id);
            // Activate BSA so xlink cState_Wait effects play during aiming
            if(bullets[id][mActiveSlot[id]] != NULL && !bullets[id][mActiveSlot[id]]->isActive()){
                bullets[id][mActiveSlot[id]]->prepare(player);
            }
        }
        // Handle remote players: network event received, defer launch through cShootPrepare
        // so the prepare animation plays before the BSA actually launches.
        if(mRemoteShotPending[id] && playerState[id] == TornadoState::cAim){
			mShootPrepareFrm[id] = Game::MainMgr::sInstance->mPaintGameFrame;
			playerState[id] = TornadoState::cShootPrepare;
			// Mirror local commit — transition BSA to cState_Wait so the
			// "position confirmed" xlink sound plays on remote consoles too
			if(mActiveSlot[id] >= 0 && bullets[id][mActiveSlot[id]] != NULL){
				bullets[id][mActiveSlot[id]]->mStateMachine.changeState(BSAState::cState_Wait);
			}
        }
    }
    void InkstrikeMgr::playerFirstCalc(Game::Player *player){
        bool isCtrlPerformer = Game::PlayerMgr::sInstance->mCurrentPlayerIndex == player->mIndex;
        int id = player->mIndex;
        if(mTornadoModel[id] == NULL){
            return;
        }
        Game::BulletMgr *bulletMgr = Game::MainMgr::sInstance->mBulletMgr;
        if (!mSpawnYCaptured && player->isAlive() && isCtrlPerformer && player->mPosition.mY > 10.0f) {
			mSpawnY = player->mPosition.mY + 110.0f;
			mSpawnYCaptured = true;
		}
        Game::PlayerInkAction *InkAction = player->mPlayerInkAction;
        Game::MiniMap *miniMap = Utils::getMinimap();
        sead::Vector3<float> miniMapAt = sead::Vector3<float>::zero;
        bool isOnline = !Game::MainMgr::sInstance->cloneObjMgr->mIsOfflineScene;
        auto bulletCloneHandle = player->mPlayerNetControl->mCloneHandle->mBulletCloneHandle;
		// Check if Princess Cannon was picked up - Cancel special
		Flexlion::BigLaserMode checkMode = Flexlion::BigLaserModeMgr::sInstance->getMode(player->mIndex);
        this->detectChangeState(player);
        float camdst = float(playerState[id] == TornadoState::cAim);
        if(isCtrlPerformer) cameraanim+=(camdst - cameraanim) * 0.2f;
        if(abs(camdst - cameraanim) < 0.05f) cameraanim = camdst;
        switch(playerState[id]){
        case TornadoState::cNone:
            break;
        case TornadoState::cAim:
            if(player->isInTrouble_Dying() || player->isInTrouble_WaterFall() || !player->isAlive() || checkMode == Flexlion::cPrincessCannon || (!player->isInSpecial() && player->mSpecialWeaponId != TORNADO_SPECIAL_ID)){
                // Player died without choosing a spot or picked up princess cannon or changed weapon in shooting range — cancel special
                playerState[id] = TornadoState::cNone;
				mWasAHeld[id] = false;
				mRemoteShotPending[id] = false;
				if(mActiveSlot[id] >= 0 && bullets[id][mActiveSlot[id]] != NULL && bullets[id][mActiveSlot[id]]->isActive()){
					bullets[id][mActiveSlot[id]]->cancel();
				}
				mActiveSlot[id] = -1;
                if(isCtrlPerformer){
                    Game::MiniMap *mMap = Utils::getMinimap();
                    if(mMap != NULL) mMap->setVisible(true);
                }
                break;
            }
            if((!player->isInSpecial() || mMatchEnding) && isCtrlPerformer){
				sead::Vector3<float> autoDest;
					if(mMatchEnding){
						// Match ended — use player position.
						autoDest = player->mPosition;
						autoDest.mY = InkstrikeMgr::sInstance->mSpawnY;
						autoDest = Utils::calcGroundPos(player, autoDest);
					} else if(isCtrlPerformer && mAimValid[id] && Lp::Utl::getCtrl(0)->isHoldContinue(starlight::Controller::Buttons::A, 1)){
					// Special ran out or match ended while aiming at valid position — use cursor position
					const float halfCanvas = 360.0f;
					float halfFovyRad = camerafovy * 0.5f * MATH_PI / 180.0f;
					float tanHalfFovy = sinf(halfFovyRad) / cosf(halfFovyRad);
					float worldPerCanvas = cameraheight * tanHalfFovy / halfCanvas;
					sead::Vector3<float> camAt = miniMap->mMiniMapCamera->mAt;
					autoDest.mX = camAt.mX + miniMap->mCursorPos.mX * worldPerCanvas;
					autoDest.mY = InkstrikeMgr::sInstance->mSpawnY;
					autoDest.mZ = camAt.mZ - miniMap->mCursorPos.mY * worldPerCanvas;
					autoDest = Utils::calcGroundPos(player, autoDest);
				} 
				else {
					// No valid aim — fall back to player position
					autoDest = player->mPosition;
					autoDest.mY = InkstrikeMgr::sInstance->mSpawnY;
					autoDest = Utils::calcGroundPos(player, autoDest);
				}
				if(isOnline) bulletCloneHandle->sendEvent_Shot(player->mIndex, autoDest, player->mPosition, Game::BulletCloneEvent::Type::BulletTypeInkstrike, 0);
				isAppliedWeapon[id] = 0;
				mPendingDest[id] = autoDest;
				mShootPrepareFrm[id] = Game::MainMgr::sInstance->mPaintGameFrame;
				Game::MiniMap *mMap = Utils::getMinimap();
				if(mMap != NULL){
					Lp::Sys::XLink *mapXLink = *(Lp::Sys::XLink **)((u8*)mMap + 0x320);
					if(mapXLink != NULL){
						xlink2::Handle decideHandle;
						mapXLink->searchAndPlayWrap("Pronounce", false, &decideHandle);
					}
				}
				playerState[id] = TornadoState::cShootPrepare;
				player->mPlayerInkAction->mNoControlPtr = (u64)bullets[id][mActiveSlot[id]];
				player->resetPaintGauge(0, 0, 0, 0);
				Prot::ObfStore(&player->mSpecialLeftFrame, 0);
				informPerformSpecial(player);
				mWasAHeld[id] = false;
				if(mActiveSlot[id] >= 0 && bullets[id][mActiveSlot[id]] != NULL) 
						bullets[id][mActiveSlot[id]]->mStateMachine.changeState(BSAState::cState_Wait);
				if(isCtrlPerformer){
					Game::MiniMap *mMap = Utils::getMinimap();
					if(mMap != NULL){
						mMap->setVisible(false);
						mMap->fadeAllEffect();
					}
				}
				break;
			}


            // Continuously validate aim position for the controlled player
            if(isCtrlPerformer and Utils::isShowMinimap() and cameraanim > 0.95f){
				const float halfCanvas = 360.0f;
				float halfFovyRad = camerafovy * 0.5f * MATH_PI / 180.0f;
				float tanHalfFovy = sinf(halfFovyRad) / cosf(halfFovyRad);
				float worldPerCanvas = cameraheight * tanHalfFovy / halfCanvas;
				sead::Vector3<float> camAt = miniMap->mMiniMapCamera->mAt;
				sead::Vector3<float> aimPos = cursorToWorldXZ(camAt, miniMap->mCursorPos, worldPerCanvas);
				aimPos.mY = InkstrikeMgr::sInstance->mSpawnY;
				bool groundFound = false;
				Utils::calcGroundPos(player, aimPos, &groundFound);
				mAimValid[id] = groundFound;
			}
			
			// Play open sound on the frame the minimap becomes visible
			if(isCtrlPerformer){
				bool mapVisible = Utils::isShowMinimap();
				if(mapVisible && !mMapOpen[id]){
					// Map just became visible — play sound once
					Game::MiniMap *mMap = Utils::getMinimap();
					if(mMap != NULL){
						Lp::Sys::XLink *mapXLink = *(Lp::Sys::XLink **)((u8*)mMap + 0x320);
						if(mapXLink != NULL){
							xlink2::Handle openHandle;
							mapXLink->searchAndPlayWrap("OpenArtillery", false, &openHandle);
						}
					}
					mMapOpen[id] = true;
				} 
				else 
					if(!mapVisible){
					mMapOpen[id] = false; // will let openartillery play again after closing map
					mWasAHeld[id] = false; // ensures tornado doesn't shoot if map is closed while holding A
				}
			}


            if(isCtrlPerformer and Utils::isShowMinimap() and cameraanim > 0.95f){
				bool aHeld = Lp::Utl::getCtrl(0)->isHoldContinue(starlight::Controller::Buttons::A, 1);
				bool aFirstFrame = Lp::Utl::getCtrl(0)->isHoldContinue(starlight::Controller::Buttons::A, 1) && !mWasAHeld[id];
			
				if(aHeld && !mAimValid[id]){
					// Invalid aim — play rejection sound on first frame of press
					if(aFirstFrame){
						Lp::Sys::XLink *mapXLink = *(Lp::Sys::XLink **)((u8*)miniMap + 0x320);
						if(mapXLink != NULL){
							xlink2::Handle abtnHandle;
							mapXLink->searchAndPlayWrap("AButton", false, &abtnHandle);
						}
					}
				}
			
				if(!aHeld && mWasAHeld[id] && mAimValid[id]){
					// Play Pronounce on release
					Game::MiniMap *mMap = Utils::getMinimap();
					if(mMap != NULL){
						Lp::Sys::XLink *mapXLink = *(Lp::Sys::XLink **)((u8*)mMap + 0x320);
						if(mapXLink != NULL){
							xlink2::Handle decideHandle;
							mapXLink->searchAndPlayWrap("Pronounce", false, &decideHandle);
						}
					}
					// A was released on valid aim — commit shot
					const float halfCanvas = 360.0f;
					float halfFovyRad = camerafovy * 0.5f * MATH_PI / 180.0f;
					float tanHalfFovy = sinf(halfFovyRad) / cosf(halfFovyRad);
					float worldPerCanvas = cameraheight * tanHalfFovy / halfCanvas;
					sead::Vector3<float> camAt = miniMap->mMiniMapCamera->mAt;
					miniMapAt = cursorToWorldXZ(camAt, miniMap->mCursorPos, worldPerCanvas);
					miniMapAt.mY = InkstrikeMgr::sInstance->mSpawnY;
					miniMapAt = Utils::calcGroundPos(player, miniMapAt);
					if(isOnline) bulletCloneHandle->sendEvent_Shot(player->mIndex, miniMapAt, player->mPosition, Game::BulletCloneEvent::Type::BulletTypeInkstrike, 0);
					isAppliedWeapon[id] = 0;
					mPendingDest[id] = miniMapAt;
					mShootPrepareFrm[id] = Game::MainMgr::sInstance->mPaintGameFrame;
					playerState[id] = TornadoState::cShootPrepare;
					player->mPlayerInkAction->mNoControlPtr = (u64)bullets[id][mActiveSlot[id]];
					player->resetPaintGauge(0, 0, 0, 0);
					Prot::ObfStore(&player->mSpecialLeftFrame, 0);
					informPerformSpecial(player);
					if(mActiveSlot[id] >= 0 && bullets[id][mActiveSlot[id]] != NULL) 
						bullets[id][mActiveSlot[id]]->mStateMachine.changeState(BSAState::cState_Wait);
					if(mMap != NULL){
						mMap->setVisible(false);
						mMap->fadeAllEffect();
					}
				}
			
				mWasAHeld[id] = aHeld;
			}
            break;
        case TornadoState::cShootPrepare:
        {
			if(isCtrlPerformer){
				Game::MiniMap *mMap = Utils::getMinimap();
				if(mMap != NULL) mMap->setVisible(true);
			}
            int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mShootPrepareFrm[id];
            if(elapsed >= startflightdelay || player->isInTrouble_Dying() || player->isInTrouble_AirFall() || player->isInTrouble_WaterFall() || mMatchEnding){
                // Get launch position from ink tank bone
                sead::Vector3<float> launchPos = player->mPosition;
                Cmn::PlayerCustomPart *tank = player->getTank();
                if(tank == NULL) tank = player->mPlayerCustomMgr->getMantle();
                if(tank != NULL){
                    sead::Matrix34<float> tankBoneMtx;
                    tank->getRootBoneMtx(&tankBoneMtx);
                    launchPos.mX = tankBoneMtx.matrix[0][3];
                    launchPos.mY = tankBoneMtx.matrix[1][3];
                    launchPos.mZ = tankBoneMtx.matrix[2][3];
                }
                this->informShotInkstrike(player, launchPos, mPendingDest[id], Game::MainMgr::sInstance->mPaintGameFrame);
                mShootFrm[id] = Game::MainMgr::sInstance->mPaintGameFrame;
                playerState[id] = TornadoState::cShoot;
            }
            break;
        }
        case TornadoState::cShoot:
		{
			isAppliedWeapon[id] = 0;
			int flightDelay = player->isInTrouble_Dying() || player->isInTrouble_WaterFall() ? 10 : startflightdelay;
			int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mShootFrm[id];
			if(elapsed >= playerdelay){
				// Release NoControl restriction
				player->mPlayerInkAction->mNoControlPtr = 0;
				playerState[id] = TornadoState::cNone;
				player->mPlayerMotion->animSeq_3C = -1;
				player->informGetWeapon_Impl_(player->mMainWeaponId, player->mSubWeaponId, player->mSpecialWeaponId, 0);
			}
			break;
		}
        };
    }
    void InkstrikeMgr::playerThirdCalc(Game::Player *player){
        int id = player->mIndex;
        if(player->isInTrouble_Dying() || player->isInTrouble_WaterFall()){
            // Clear shoot animations when dying
            if(player->mPlayerMotion->animSeq_3C == 46 || player->mPlayerMotion->animSeq_3C == 47){
                player->mPlayerMotion->animSeq_3C = -1;
            }
            return;
        }
        int desiredAnim = -1;
        switch(playerState[id]){
        case TornadoState::cShootPrepare:
            desiredAnim = 46;
            break;
        case TornadoState::cShoot:
            desiredAnim = 47;
            break;
        default:
            break;
        }
        if(desiredAnim != -1 && player->mPlayerMotion->animSeq_3C != desiredAnim){
            player->mPlayerMotion->startOneTime_Insert((Game::PlayerMotion::AnimID)desiredAnim);
        }
    }
    void InkstrikeMgr::registerPlayer(Game::Player *player){
        int id = player->mIndex;
        Lp::Utl::ModelCreateArg arg;
        mTornadoModel[id] = Cmn::GfxUtl::createModel(player->mTeam, sead::SafeStringBase<char>::create("Tornado"), *mTornadoArc, arg, NULL);
        mTornadoModel[id]->bind(Game::MainMgr::sInstance->gfxMgr->modelScene);
        mTornadoMonitorModel[id] = Cmn::GfxUtl::createModel(player->mTeam, sead::SafeStringBase<char>::create("Tornado_Monitor"), *mTornadoMonitorArc, arg, NULL);
        mTornadoMonitorModel[id]->bind(Game::MainMgr::sInstance->gfxMgr->modelScene);
        gsys::Model *fullModel = *player->mPlayerModel->mFullModel;
        if(fullModel != NULL){
            mTankRootBoneIdx[id] = fullModel->searchBone(sead::SafeStringBase<char>::create("tank_root"));
        }
        // Create the BulletSuperArtillery actor for this player
        for(int slot = 0; slot < 2; slot++){
			bullets[id][slot] = BulletSuperArtillery::create(
				(Lp::Sys::Actor *)player,
				mTornadoModel[id],
				player->mTeam
			);
		}
	}
    void InkstrikeMgr::playerFourthCalc(Game::Player *player){
        int id = player->mIndex;
        if(mTornadoModel[id] == NULL || mTornadoMonitorModel[id] == NULL || player->mPlayerCustomMgr == NULL){
            return;
        }
        // Don't render special models when player is dead
        if(!player->isAlive()){
            return;
        }
        switch(playerState[id]){
        case TornadoState::cNone:
        {
            // Restore weapon visibility only once when leaving tornado special
            if(mWeaponHidden[id]){
                Cmn::PlayerWeapon *weapon = player->mPlayerWeapon[0];
                if(weapon != NULL){
                    weapon->setVisible(true);
                }
                mWeaponHidden[id] = false;
            }
            break;
        }
        case TornadoState::cAim:
        case TornadoState::cShootPrepare:
        case TornadoState::cShoot:
        {
            // Attach Wsp_Tornado_Monitor to the weapon bone (replacing current weapon)
            Cmn::PlayerWeapon *weapon = player->mPlayerWeapon[0];
            if(weapon != NULL){
                sead::Matrix34<float> weaponBoneMtx;
                weapon->getRootBoneMtx(&weaponBoneMtx);
                weapon->setVisible(false);
                mWeaponHidden[id] = true;
                mTornadoMonitorModel[id]->mtx = weaponBoneMtx;
                mTornadoMonitorModel[id]->mUpdateScale|=1;
                mTornadoMonitorModel[id]->updateAnimationWorldMatrix_(3);
                mTornadoMonitorModel[id]->requestDraw();
            }
            break;
        }
        };
    }
}
