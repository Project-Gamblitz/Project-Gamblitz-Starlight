#include "flexlion/InkstrikeMgr.hpp"
const int flighttime = 150;
const int startflightdelay = 60; // delay from when a point is chosen to when the tornado is actually launched. (Spot is selected in the map -> wait 60 frames -> launch tornado)
const int playerdelay = 120; // 60 frames Shoot_Tornado_St + 60 frames Shoot_Tornado
const float flightheight = 300.0f;
const float carryingOffset = -5.0f;
namespace Flexlion{
    BulletTornado::BulletTornado(){
        this->reset();
    }
    void BulletTornado::reset(){
        startfrm = 0;
        sender = NULL;
        model = NULL;
        isactive = 0;
        from = sead::Vector3<float>::zero;
        to = sead::Vector3<float>::zero;
        rot = sead::Vector3<float>::zero;
        pos = sead::Vector3<float>::zero;
        superball = NULL;
        isShot = 0;
    }
    void BulletTornado::onActivate(Game::Player *Sender, gsys::Model *Model, sead::Vector3<float> src, sead::Vector3<float> dst, int paintgamefrm){
        this->reset();
        startfrm = paintgamefrm;
        sender = Sender;
        model = Model;
        from = src;
        to = dst;
        rot.mX = 0.0f;
        rot.mZ = 0.0f;
        rot.mY = atan2f(from.mX - to.mX, from.mZ - to.mZ) + MATH_PI; 
        if(rot.mY > MATH_PI * 2.0f) rot.mY -= MATH_PI * 2.0f;
        isactive = 1;
    }
    void BulletTornado::onRender(){
        if(!isactive) return;
        model->mtx = {{ 
            cosf(rot.mY) * cosf(rot.mZ),
            sinf(rot.mX) * sinf(rot.mY) * cosf(rot.mZ) - sinf(rot.mZ) * cosf(rot.mX),
            cosf(rot.mX) * sinf(rot.mY) * cosf(rot.mZ) + sinf(rot.mZ) * sinf(rot.mX),
            pos.mX,
            sinf(rot.mZ) * cosf(rot.mY),
            sinf(rot.mX) * sinf(rot.mY) * sinf(rot.mZ) + cosf(rot.mZ) * cosf(rot.mX),
            cosf(rot.mX) * sinf(rot.mY) * sinf(rot.mZ) - cosf(rot.mZ) * sinf(rot.mX),
            pos.mY,
            -sinf(rot.mY),
            sinf(rot.mX) * cosf(rot.mY),
            cosf(rot.mX) * cosf(rot.mY),
            pos.mZ
        }};
        model->mUpdateScale|=1;
        model->updateAnimationWorldMatrix_(3);
        model->requestDraw();
    }
    void BulletTornado::calcBurst(){
        if(superball == NULL) return;
        auto ballIter = Game::BulletSpSuperBall::getClassIterNodeStatic();
        bool s = 0;
        for(Game::BulletSpSuperBall *bullet = (Game::BulletSpSuperBall *)ballIter->derivedFrontActiveActor(); bullet != NULL; bullet = (Game::BulletSpSuperBall *)ballIter->derivedNextActiveActor(bullet)){
            if(superball == bullet){
                s = 1;
                break;
            }
        }
        if(!s){
            this->reset();
            return;
        }
        if(superball->mIsHitGnd and superball->mCore != NULL){
        sead::Matrix34<float> *matrix = &superball->mCore->mMatrix;
        *matrix = {{
            0.7f,		       0.0f,       		 0.0f, 				matrix->matrix[0][3],
            0.0f,    		 3.0f,      		 0.0f,				matrix->matrix[1][3],
            0.0f,    		 0.0f,       		 0.7f,				matrix->matrix[2][3]
        }};
        }
    }
    void BulletTornado::burst(){
        Game::PlayerSuperBall *playerSuperBall = sender->mPlayerSuperBall;
        Game::PlayerInkAction *InkAction = sender->mPlayerInkAction;
        Game::BulletMgr *bulletMgr = Game::MainMgr::sInstance->mBulletMgr;
        if(playerSuperBall != NULL and InkAction != NULL and bulletMgr != NULL){
            if(playerSuperBall->mBullet == NULL){
                playerSuperBall->mBullet = (Game::BulletSpSuperBall*)bulletMgr->activateOneCancelUnnecessary(0x76, sender->mIsRemote == 0, pos, sead::Vector3<float>::zero, -1);
                superball = playerSuperBall->mBullet;
            }
            if(playerSuperBall->mBullet != NULL){
                playerSuperBall->mBullet->Initialize(sender->mIndex, &pos);
                InkstrikeMgr::sInstance->isShot = 1;
                InkAction->shotSuperBall();
                InkstrikeMgr::sInstance->isShot = 0;
                playerSuperBall->reset();
                playerSuperBall->mBullet = NULL;
            }
        }
        isactive = 0;
    }
    void BulletTornado::onCalc(){
        this->calcBurst();
        if(!isactive) return;
        if(Game::MainMgr::sInstance->mPaintGameFrame - startfrm >= flighttime){
            this->burst();
            return;
        }
        float anim = float(Game::MainMgr::sInstance->mPaintGameFrame - startfrm) / float(flighttime);
        sead::Vector3<float> respos;
        respos.mY = from.mY * (1.0f - anim) + to.mY * anim + sinf(anim * MATH_PI) * flightheight;
        respos.mX = from.mX * (1.0f - anim) + to.mX * anim;
        respos.mZ = from.mZ * (1.0f - anim) + to.mZ * anim;
        rot.mX = atan2f(sqrtf(powf(abs(respos.mX - pos.mX), 2) + powf(abs(respos.mZ - pos.mZ), 2)), respos.mY - pos.mY);
        pos = respos;
    }
    InkstrikeMgr *InkstrikeMgr::sInstance = NULL;
    InkstrikeMgr::InkstrikeMgr(){
        sInstance = this;
        memset(this, 0, sizeof(InkstrikeMgr));
        mTornadoArc = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Wsp_Tornado"), NULL, 0, NULL, NULL);
        cameraheight = 300.0f; // TODO: Make a thing that you give edge coordinates for each map and it calculates this
        cameraanim = 0.0f;
        for(int i = 0; i < 10; i++) bullets[i] = new BulletTornado();
    }
    void InkstrikeMgr::informShotInkstrike(Game::Player *player, sead::Vector3<float> pos, sead::Vector3<float> dest, int paintgamefrm){
        if(!bullets[player->mIndex]->isactive){
            bullets[player->mIndex]->onActivate(player, mTornadoModel[player->mIndex], pos, dest, paintgamefrm);
            isAppliedWeapon[player->mIndex] = 0;
            Prot::ObfStore(&player->mSpecialLeftFrame, 0);
            playerState[player->mIndex] = TornadoState::cShoot;
        }
    }
    void InkstrikeMgr::onCalc(){
        if(Game::MainMgr::sInstance == NULL or !Utils::isSceneLoaded()){
            if(!isBulletDeinit){
                for(int i = 0; i < 10; i++) bullets[i]->reset();
                isBulletDeinit = 1;
            }
            return;
        }
        isBulletDeinit = 0;
        for(int i = 0; i < 10; i++) bullets[i]->onCalc();
    }
    void InkstrikeMgr::detectChangeState(Game::Player *player){
        int id = player->mIndex;
        if(player->isInSpecial() and player->mSpecialWeaponId == TORNADO_SPECIAL_ID and playerState[id] == TornadoState::cNone){
            playerState[id] = TornadoState::cAim;
            isAppliedWeapon[id] = 0;
        }
        if(!bullets[id]->isactive and playerState[id] == TornadoState::cShoot){
            isAppliedWeapon[id] = 0;
            playerState[id] = TornadoState::cNone;
        } else if(bullets[id]->isactive and playerState[id] == TornadoState::cAim){
            isAppliedWeapon[id] = 0;
            Prot::ObfStore(&player->mSpecialLeftFrame, 0);
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
                const float distmul = 2.0f;
                float dist = sqrtf(miniMap->mCursorPos.mX * miniMap->mCursorPos.mX + miniMap->mCursorPos.mY * miniMap->mCursorPos.mY) * distmul;
                float deg = atan2f(miniMap->mCursorPos.mY, miniMap->mCursorPos.mX) + MATH_PI * 0.25f; // some maps also need an additional + MATH_PI * 0.25f (90 degrees), needs to be somehow accessible
                miniMapAt.mX = cosf(deg) * dist;
                miniMapAt.mY = cameraheight;
                miniMapAt.mZ = sinf(deg) * dist * -1;
                miniMapAt = Utils::calcGroundPos(player, miniMapAt);
                this->informShotInkstrike(player, player->mPosition, miniMapAt, Game::MainMgr::sInstance->mPaintGameFrame);
                if(isOnline) bulletCloneHandle->sendEvent_Shot(player->mIndex, player->mPosition, miniMapAt, Game::BulletCloneEvent::Type::BulletTypeInkstrike, 0);
                player->resetPaintGauge(0, 0, 0, 0); // reset gauge when tornado shot
                isAppliedWeapon[id] = 0;
                Prot::ObfStore(&player->mSpecialLeftFrame, 0);
                playerState[id] = TornadoState::cShoot;
                player->informGetWeapon_Impl_(player->mMainWeaponId, player->mSubWeaponId, player->mSpecialWeaponId, 0);
            }
            break;
        case TornadoState::cShoot:
            isAppliedWeapon[id] = 0;
            break;
        };
    }
    void InkstrikeMgr::registerPlayer(Game::Player *player){
        int id = player->mIndex;
        Lp::Utl::ModelCreateArg arg;
        mTornadoModel[id] = Cmn::GfxUtl::createModel(player->mTeam, sead::SafeStringBase<char>::create("Tornado"), *mTornadoArc, arg, NULL);
        mTornadoModel[id]->bind(Game::MainMgr::sInstance->gfxMgr->modelScene);
    }
    void InkstrikeMgr::playerFourthCalc(Game::Player *player){
        int id = player->mIndex;
        if(mTornadoModel[id] == NULL || player->mPlayerCustomMgr == NULL){
            return;
        }
        Cmn::PlayerCustomPart *tank = player->getTank();
        if(tank == NULL){
            tank = player->mPlayerCustomMgr->getMantle();
        }
        if(bullets[id]->isactive){
            bullets[id]->onRender();
            return;
        }
        sead::Vector3<float> mtxrot = sead::Vector3<float>::zero;
        sead::Vector3<float> mtxpos = sead::Vector3<float>::zero;
        switch(playerState[id]){
        case TornadoState::cNone:
            break;
        case TornadoState::cAim:
            if(tank == NULL){
                break;
            }
            if(tank->mActorFullModel == NULL){
                break;
            }
            mTornadoModel[id]->mtx = tank->mActorFullModel->mtx;
            mtxrot = Utils::getRotFromMtx(mTornadoModel[id]->mtx);
            mtxpos = Utils::getMtxPos(&mTornadoModel[id]->mtx);
            mtxpos.mX += sinf(mtxrot.mY) * carryingOffset;
            mtxpos.mZ += cosf(mtxrot.mY) * carryingOffset;
            Utils::setMtxPos(&mTornadoModel[id]->mtx, mtxpos);
            mTornadoModel[id]->mUpdateScale|=1;
            mTornadoModel[id]->updateAnimationWorldMatrix_(3);
            mTornadoModel[id]->requestDraw();
            break;
        case TornadoState::cShoot:
            // Shouldn't be entered at all?
            mTornadoModel[id]->updateAnimationWorldMatrix_(3);
            mTornadoModel[id]->requestDraw();
            break; 
        };
    }
}