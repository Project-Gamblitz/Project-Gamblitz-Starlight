#include "flexlion/InkstrikeMgr.hpp"
const int startflightdelay = 40; // delay from when a point is chosen to when the tornado is actually launched
const int playerdelay = 100; // 40 frames Shoot_Tornado_St + 60 frames Shoot_Tornado (was 60 + 60)
const float tornadoTankZOffset = -3.0f;
namespace Flexlion{
    InkstrikeMgr *InkstrikeMgr::sInstance = NULL;
    InkstrikeMgr::InkstrikeMgr(){
        sInstance = this;
        memset(this, 0, sizeof(InkstrikeMgr));
        mTornadoArc = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Wsp_Tornado"), NULL, 0, NULL, NULL);
        mTornadoMonitorArc = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Wsp_Tornado_Monitor"), NULL, 0, NULL, NULL);
        cameraheight = 300.0f;
        cameraanim = 0.0f;
        for(int i = 0; i < 10; i++){
            bullets[i] = NULL;
            mTankRootBoneIdx[i] = -1;
        }
    }
    void InkstrikeMgr::informShotInkstrike(Game::Player *player, sead::Vector3<float> pos, sead::Vector3<float> dest, int paintgamefrm){
        if(bullets[player->mIndex] != NULL && !bullets[player->mIndex]->mFlightActive){
            // BSA is already active from prepare(), transition it to flight
            bullets[player->mIndex]->launch(pos, dest, paintgamefrm);
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
        // Handle remote players: flight started via network while still in cAim
        if(bullets[id] != NULL && bullets[id]->mFlightActive && playerState[id] == TornadoState::cAim){
            mShootFrm[id] = Game::MainMgr::sInstance->mPaintGameFrame;
            playerState[id] = TornadoState::cShoot;
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
        this->detectChangeState(player);
        float camdst = float(playerState[id] == TornadoState::cAim);
        if(isCtrlPerformer) cameraanim+=(camdst - cameraanim) * 0.2f;
        if(abs(camdst - cameraanim) < 0.05f) cameraanim = camdst;
        switch(playerState[id]){
        case TornadoState::cNone:
            break;
        case TornadoState::cAim:
            if(player->isInTrouble_Dying()){
                // Player died without choosing a spot — cancel special
                playerState[id] = TornadoState::cNone;
                if(bullets[id] != NULL && bullets[id]->isActive()){
                    bullets[id]->cancel();
                }
                if(isCtrlPerformer){
                    Game::MiniMap *mMap = Utils::getMinimap();
                    if(mMap != NULL) mMap->setVisible(true);
                }
                break;
            }
            if(!player->isInSpecial()){
                // Special ran out without choosing a spot — shoot at player position
                sead::Vector3<float> fallbackDest = player->mPosition;
                fallbackDest.mY += cameraheight;
                fallbackDest = Utils::calcGroundPos(player, fallbackDest);
                if(isOnline) bulletCloneHandle->sendEvent_Shot(player->mIndex, player->mPosition, fallbackDest, Game::BulletCloneEvent::Type::BulletTypeInkstrike, 0);
                player->resetPaintGauge(0, 0, 0, 0);
                isAppliedWeapon[id] = 0;
                mPendingDest[id] = fallbackDest;
                mShootPrepareFrm[id] = Game::MainMgr::sInstance->mPaintGameFrame;
                playerState[id] = TornadoState::cShootPrepare;
                if(bullets[id] != NULL) bullets[id]->mStateMachine.changeState(BSAState::cState_Wait);
                Game::MiniMap *mMap = Utils::getMinimap();
                if(mMap != NULL){
                    mMap->setVisible(false);
                    mMap->fadeAllEffect();
                }
                break;
            }

            if(isCtrlPerformer and Utils::isShowMinimap() and Lp::Utl::getCtrl(0)->isHoldContinue(starlight::Controller::Buttons::A, 1) and cameraanim > 0.95f){
                const float distmul = 2.0f;
                float dist = sqrtf(miniMap->mCursorPos.mX * miniMap->mCursorPos.mX + miniMap->mCursorPos.mY * miniMap->mCursorPos.mY) * distmul;
                float deg = atan2f(miniMap->mCursorPos.mY, miniMap->mCursorPos.mX) + MATH_PI * 0.25f;
                miniMapAt.mX = cosf(deg) * dist;
                miniMapAt.mY = cameraheight;
                miniMapAt.mZ = sinf(deg) * dist * -1;
                miniMapAt = Utils::calcGroundPos(player, miniMapAt);
                if(isOnline) bulletCloneHandle->sendEvent_Shot(player->mIndex, player->mPosition, miniMapAt, Game::BulletCloneEvent::Type::BulletTypeInkstrike, 0);
                player->resetPaintGauge(0, 0, 0, 0);
                isAppliedWeapon[id] = 0;
                mPendingDest[id] = miniMapAt;
                mShootPrepareFrm[id] = Game::MainMgr::sInstance->mPaintGameFrame;
                playerState[id] = TornadoState::cShootPrepare;
                if(bullets[id] != NULL) bullets[id]->mStateMachine.changeState(BSAState::cState_Wait);
                Game::MiniMap *mMap = Utils::getMinimap();
                if(mMap != NULL){
                    Lp::Sys::XLink *mapXLink = *(Lp::Sys::XLink **)((u8*)mMap + 0x320);
                    if(mapXLink != NULL){
                        xlink2::Handle decideHandle;
                        mapXLink->searchAndPlayWrap("Decide", false, &decideHandle);
                    }
                    mMap->setVisible(false);
                    mMap->fadeAllEffect();
                }
            }
            break;
        case TornadoState::cShootPrepare:
        {
            Prot::ObfStore(&player->mSpecialLeftFrame, startflightdelay);
            int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mShootPrepareFrm[id];
            if(elapsed >= startflightdelay || player->isInTrouble_Dying()){
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
			int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mShootFrm[id];
			if(elapsed >= flightDelay){
				Prot::ObfStore(&player->mSpecialLeftFrame, 0);
				playerState[id] = TornadoState::cNone;
				player->informGetWeapon_Impl_(player->mMainWeaponId, player->mSubWeaponId, player->mSpecialWeaponId, 0);
				if(isCtrlPerformer){
					Game::MiniMap *mMap = Utils::getMinimap();
					if(mMap != NULL) mMap->setVisible(true);
				}
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
        bool bulletActive = bullets[id] != NULL && bullets[id]->isActive();
        switch(playerState[id]){
        case TornadoState::cNone:
            break;
        case TornadoState::cAim:
        case TornadoState::cShootPrepare:
        {
            // Tornado model rendering is handled by BSA's calcTankBone via vtFourthCalc
            // Attach Wsp_Tornado_Monitor to the weapon bone (replacing current weapon)
            Cmn::PlayerWeapon *weapon = player->mPlayerWeapon[0];
            if(weapon != NULL){
                sead::Matrix34<float> weaponBoneMtx;
                weapon->getRootBoneMtx(&weaponBoneMtx);
                weapon->setVisible(false);
                mTornadoMonitorModel[id]->mtx = weaponBoneMtx;
                mTornadoMonitorModel[id]->mUpdateScale|=1;
                mTornadoMonitorModel[id]->updateAnimationWorldMatrix_(3);
                mTornadoMonitorModel[id]->requestDraw();
            }
            break;
        }
        case TornadoState::cShoot:
        {
            // Keep monitor visible during shoot animation
            Cmn::PlayerWeapon *weapon = player->mPlayerWeapon[0];
            if(weapon != NULL){
                sead::Matrix34<float> weaponBoneMtx;
                weapon->getRootBoneMtx(&weaponBoneMtx);
                weapon->setVisible(false);
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
