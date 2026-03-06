#include "flexlion/InkstrikeMgr.hpp"
const int startflightdelay = 60; // delay from when a point is chosen to when the tornado is actually launched
const int playerdelay = 120; // 60 frames Shoot_Tornado_St + 60 frames Shoot_Tornado
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
        if(bullets[player->mIndex] != NULL && !bullets[player->mIndex]->isActive()){
            bullets[player->mIndex]->launch(player, pos, dest, paintgamefrm);
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
        }
        // Handle remote players: bullet activated via network while still in cAim
        if(bullets[id] != NULL && bullets[id]->isActive() && playerState[id] == TornadoState::cAim){
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
            if(!player->isInSpecial()){
                playerState[id] = TornadoState::cNone;
                break;
            }

            if(isCtrlPerformer and Utils::isShowMinimap() and Lp::Utl::getCtrl(0)->isHoldContinue(starlight::Controller::Buttons::A, 1) and cameraanim > 0.95f){
                // Convert cursor canvas position to world coordinates using the
                // minimap camera's viewport unproject (canvas → screen → camera → world)
                Game::MiniMapCamera *cam = miniMap->mMiniMapCamera;
                sead::Viewport *viewport = (sead::Viewport*)((u8*)cam->cameraMgr + 0x5B8);
                sead::Projection *proj = cam->mProjection;
                sead::Camera *seadCam = cam->getCamera();

                sead::Vector2<float> cursorCanvas = miniMap->mCursorPos;
                viewport->unproject(&miniMapAt, cursorCanvas, *proj, *seadCam);

                // Snap to actual terrain height
                miniMapAt.mY = cameraheight;
                miniMapAt = Utils::calcGroundPos(player, miniMapAt);
                if(isOnline) bulletCloneHandle->sendEvent_Shot(player->mIndex, player->mPosition, miniMapAt, Game::BulletCloneEvent::Type::BulletTypeInkstrike, 0);
                player->resetPaintGauge(0, 0, 0, 0);
                isAppliedWeapon[id] = 0;
                mPendingDest[id] = miniMapAt;
                mShootPrepareFrm[id] = Game::MainMgr::sInstance->mPaintGameFrame;
                playerState[id] = TornadoState::cShootPrepare;
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
            if(elapsed >= startflightdelay){
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
            Prot::ObfStore(&player->mSpecialLeftFrame, startflightdelay);
            int elapsed = Game::MainMgr::sInstance->mPaintGameFrame - mShootFrm[id];
            if(elapsed >= startflightdelay){
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
        // When the bullet actor is active, it handles model rendering via its own fourthCalc
        if(bullets[id] != NULL && bullets[id]->isActive()){
            return;
        }
        switch(playerState[id]){
        case TornadoState::cNone:
            break;
        case TornadoState::cAim:
        case TornadoState::cShootPrepare:
        {
            // Attach Wsp_Tornado to the tank_root bone on the player model
            Cmn::PlayerCustomPart *tank = player->getTank();
            if(tank == NULL) tank = player->mPlayerCustomMgr->getMantle();
            if(tank != NULL){
                sead::Matrix34<float> tankBoneMtx;
                tank->getRootBoneMtx(&tankBoneMtx);
                // Normalize rotation columns to strip bone scale, keep only position + rotation
                for(int col = 0; col < 3; col++){
                    float len = sqrtf(
                        tankBoneMtx.matrix[0][col] * tankBoneMtx.matrix[0][col] +
                        tankBoneMtx.matrix[1][col] * tankBoneMtx.matrix[1][col] +
                        tankBoneMtx.matrix[2][col] * tankBoneMtx.matrix[2][col]
                    );
                    if(len > 0.0f){
                        tankBoneMtx.matrix[0][col] /= len;
                        tankBoneMtx.matrix[1][col] /= len;
                        tankBoneMtx.matrix[2][col] /= len;
                    }
                }
                // Apply Z axis offset along the bone's local Z direction
                tankBoneMtx.matrix[0][3] += tankBoneMtx.matrix[0][2] * tornadoTankZOffset;
                tankBoneMtx.matrix[1][3] += tankBoneMtx.matrix[1][2] * tornadoTankZOffset;
                tankBoneMtx.matrix[2][3] += tankBoneMtx.matrix[2][2] * tornadoTankZOffset;
                mTornadoModel[id]->mtx = tankBoneMtx;
                mTornadoModel[id]->mUpdateScale|=1;
                mTornadoModel[id]->updateAnimationWorldMatrix_(3);
                mTornadoModel[id]->requestDraw();
            }
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
            // During shoot phase, the BSA actor handles the model
            break;
        };
    }
}
