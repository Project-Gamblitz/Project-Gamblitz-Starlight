#include "flexlion/InkstrikeMgr.hpp"
#include "flexlion/BigLaserModeMgr.hpp"
const int startflightdelay = 40; // delay from when a point is chosen to when the tornado is actually launched
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
        for(int i = 0; i < 10; i++){
            bullets[i] = NULL;
            mTankRootBoneIdx[i] = -1;
            mAimValid[i] = false;
			mWasAHeld[i] = false;
			mMapOpen[i] = false;
        }
    }
    void InkstrikeMgr::informShotInkstrike(Game::Player *player, sead::Vector3<float> pos, sead::Vector3<float> dest, int paintgamefrm){
        if(bullets[player->mIndex] != NULL && !bullets[player->mIndex]->mFlightActive){
            // BSA is already active from prepare(), transition it to flight
            bullets[player->mIndex]->launch(pos, dest, paintgamefrm, mMatchEnding);
            isAppliedWeapon[player->mIndex] = 0;
        }
    }
    void InkstrikeMgr::onCalc(){
        if(Game::MainMgr::sInstance == NULL or !Utils::isSceneLoaded()){
            if(!isBulletDeinit){
                for(int i = 0; i < 10; i++){
                    if(bullets[i] != NULL){
                        bullets[i]->reset();
                    }
                }
                resetBSAStatics();
                isBulletDeinit = 1;
				mMatchEnding = false;
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
            // Activate BSA so xlink cState_Wait effects play during aiming
            if(bullets[id] != NULL && !bullets[id]->isActive()){
                bullets[id]->prepare(player);
            }
        }
        // Handle remote players: network event received, defer launch through cShootPrepare
        // so the prepare animation plays before the BSA actually launches.
        if(mRemoteShotPending[id] && playerState[id] == TornadoState::cAim){
            mShootPrepareFrm[id] = Game::MainMgr::sInstance->mPaintGameFrame;
            playerState[id] = TornadoState::cShootPrepare;
            mRemoteShotPending[id] = false;
        }
    }
    void InkstrikeMgr::playerFirstCalc(Game::Player *player){
        bool isCtrlPerformer = Game::PlayerMgr::sInstance->mCurrentPlayerIndex == player->mIndex;
        int id = player->mIndex;
        if(mTornadoModel[id] == NULL){
            return;
        }
        Game::BulletMgr *bulletMgr = Game::MainMgr::sInstance->mBulletMgr;
        Game::PlayerSuperBall *playerSuperBall = player->mPlayerSuperBall;
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
            if(player->isInTrouble_Dying() || !player->isAlive() || checkMode == Flexlion::cPrincessCannon){
                // Player died without choosing a spot or picked up princess cannon — cancel special
                playerState[id] = TornadoState::cNone;
				mWasAHeld[id] = false;
                if(bullets[id] != NULL && bullets[id]->isActive()){
                    bullets[id]->cancel();
                }
                if(isCtrlPerformer){
                    Game::MiniMap *mMap = Utils::getMinimap();
                    if(mMap != NULL) mMap->setVisible(true);
                }
                break;
            }
            if(!player->isInSpecial() || mMatchEnding){
				sead::Vector3<float> autoDest;
					if(mMatchEnding){
						// Match ended — use player position.
						autoDest = player->mPosition;
						autoDest.mY = 3000.0f;
						autoDest = Utils::calcGroundPos(player, autoDest);
					} else if(isCtrlPerformer && mAimValid[id] && Lp::Utl::getCtrl(0)->isHoldContinue(starlight::Controller::Buttons::A, 1)){
					// Special ran out or match ended while aiming at valid position — use cursor position
					const float halfCanvas = 360.0f;
					float halfFovyRad = camerafovy * 0.5f * MATH_PI / 180.0f;
					float tanHalfFovy = sinf(halfFovyRad) / cosf(halfFovyRad);
					float worldPerCanvas = cameraheight * tanHalfFovy / halfCanvas;
					sead::Vector3<float> camAt = miniMap->mMiniMapCamera->mAt;
					autoDest.mX = camAt.mX + miniMap->mCursorPos.mX * worldPerCanvas;
					autoDest.mY = 3000.0f;
					autoDest.mZ = camAt.mZ - miniMap->mCursorPos.mY * worldPerCanvas;
					autoDest = Utils::calcGroundPos(player, autoDest);
				} 
				else {
					// No valid aim — fall back to player position
					autoDest = player->mPosition;
					autoDest.mY = 3000.0f;
					autoDest = Utils::calcGroundPos(player, autoDest);
				}
				if(isOnline) bulletCloneHandle->sendEvent_Shot(player->mIndex, player->mPosition, autoDest, Game::BulletCloneEvent::Type::BulletTypeInkstrike, 0);
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
				player->resetPaintGauge(0, 0, 0, 0);
				playerState[id] = TornadoState::cShootPrepare;
				informPerformSpecial(player);
				mWasAHeld[id] = false;
				if(bullets[id] != NULL) bullets[id]->mStateMachine.changeState(BSAState::cState_Wait);
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
                sead::Vector3<float> aimPos;
                aimPos.mX = camAt.mX + miniMap->mCursorPos.mX * worldPerCanvas;
                aimPos.mY = 3000.0f;
                aimPos.mZ = camAt.mZ - miniMap->mCursorPos.mY * worldPerCanvas;
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
					miniMapAt.mX = camAt.mX + miniMap->mCursorPos.mX * worldPerCanvas;
					miniMapAt.mY = 3000.0f;
					miniMapAt.mZ = camAt.mZ - miniMap->mCursorPos.mY * worldPerCanvas;
					miniMapAt = Utils::calcGroundPos(player, miniMapAt);
					if(isOnline) bulletCloneHandle->sendEvent_Shot(player->mIndex, player->mPosition, miniMapAt, Game::BulletCloneEvent::Type::BulletTypeInkstrike, 0);
					player->resetPaintGauge(0, 0, 0, 0);
					isAppliedWeapon[id] = 0;
					mPendingDest[id] = miniMapAt;
					mShootPrepareFrm[id] = Game::MainMgr::sInstance->mPaintGameFrame;
					playerState[id] = TornadoState::cShootPrepare;
					informPerformSpecial(player);
					if(bullets[id] != NULL) bullets[id]->mStateMachine.changeState(BSAState::cState_Wait);
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
            Prot::ObfStore(&player->mSpecialLeftFrame, startflightdelay);
            Prot::ObfStore(&player->mLayoutSpecialState, -1); // negative total → gauge shows 0%
            int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mShootPrepareFrm[id];
            if(elapsed >= startflightdelay || player->isInTrouble_Dying() || mMatchEnding){
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
			int flightDelay = player->isInTrouble_Dying() ? 10 : startflightdelay;
			Prot::ObfStore(&player->mSpecialLeftFrame, flightDelay);
			Prot::ObfStore(&player->mLayoutSpecialState, -1); // negative total → gauge shows 0%
			int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mShootFrm[id];
			if(elapsed >= flightDelay){
				Prot::ObfStore(&player->mSpecialLeftFrame, 0);
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
        if(player->isInTrouble_Dying()){
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
        bullets[id] = BulletSuperArtillery::create(
            (Lp::Sys::Actor *)player,
            mTornadoModel[id],
            player->mTeam
        );
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
