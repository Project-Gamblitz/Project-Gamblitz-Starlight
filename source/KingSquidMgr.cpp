#include "xlink2/handle.hpp"
#include "flexlion/KingSquidMgr.hpp"
#include "flexlion/Utils.hpp"
#include "starlight/input.hpp"
#include "flexlion/DrawUtils.hpp"
using namespace starlight;

namespace Starlion{
    KingSquidMgr *KingSquidMgr::sInstance = NULL;
    PlayerKingSquid::PlayerKingSquid(Game::Player *player){
        KingSquidMgr::sInstance->mPlayerKingSquid[player->mIndex] = this;
        memset(this, 0, sizeof(PlayerKingSquid));
        mEffectManualHandle = new Cmn::EffectManualHandle();
        mEffectManualHandle->createReservationInfo(0x3C);
        mPlayer = player;
        mPlayerModel = mPlayer->mPlayerModel;
        mPlayerMotion = mPlayer->mPlayerMotion;
        mModel = mPlayerModel->mSquidModel;
        mPlayerJointSquid = mPlayer->mPlayerJointSquid;
        mSquidAnim = mPlayerMotion->mSquidAnimCtrlSet;
        memcpy(&mSquidMat, &mPlayerModel->mSquidModelDepthSilhoutteAccessKey, sizeof(Cmn::MaterialParamAccessKeySet));
        this->setupModel();
    }
    void PlayerKingSquid::setupModel(){
        if(!Utils::isValidWeapon(Cmn::Def::WeaponKind::cSpecial, 22)){
            return;
        }
        if(mModel == NULL){
            return;
        }
        Lp::Sys::ModelArc *modelArc = KingSquidMgr::sInstance->getArc(this);
        if(modelArc == NULL){
            return;
        }
        Lp::Sys::ModelArc *animArc = (Lp::Sys::ModelArc*)Game::PlayerMgr::sInstance->mModelResource.mSquidAnimArcs.mPtr[mPlayer->mModelType];
        Lp::Utl::ModelCreateArg modelCreateArg;
        Lp::Utl::AnimCreateArg animCreateArg;
        animCreateArg.mSklAnimSlotNum = 1;
        animCreateArg.mMatAnimSlotNum = 6;
        mKingSquidModel = Cmn::GfxUtl::createModelAndAnim(mPlayer->mTeam, sead::SafeStringBase<char>::create("P_Squid"), *modelArc, *animArc, modelCreateArg, animCreateArg, 1, NULL);
        if(mKingSquidModel == NULL){
            return;
        }
        if(mPlayerModel->mOcclusionQuery != NULL){
            mPlayerModel->mOcclusionQuery->add(mKingSquidModel);
        }
        mKingSquidModel->setCalcWeight(0xFFC, 1);
        mPlayerModel->mSquidModel = mKingSquidModel;
        mPlayerJointKingSquid = new Game::PlayerJointSquid((const Game::Player*)mPlayer);
        mPlayerJointKingSquid->reset();
        mPlayer->mPlayerJointSquid = mPlayerJointKingSquid;
        mKingSquidModel->mModelCallback = new Game::SquidModelCallback(mPlayerModel);
        mPlayerModel->mSquidModel = mModel;
        mPlayer->mPlayerJointSquid = mPlayerJointSquid;
        mKingSquidModel->bind(Game::MainMgr::sInstance->gfxMgr->modelScene);
        mKingSquidModel->resetRenderToDepthShadow(-1);
        Cmn::GfxUtl::setDepthSilhouetteColor(mKingSquidModel, *Cmn::GfxSetting::sInstance->mTeamColorMgr->getCurColor(mPlayer->mTeam, Cmn::TeamColorMgr::ColorVariation::COLOR_6));
        int zindx = mPlayer->mIndex + 0x80;
        for(int i = 0; i < mKingSquidModel->mChildren.mLength; i++){
            gsys::ModelUnit *child = *(gsys::ModelUnit**)mKingSquidModel->mChildren.mPtr[i];
            if(child != NULL){
                child->setXluZPrePassID(zindx);
            }
        }
        mKingSquidModel->mUpdateScale|=1;
        mKingSquidMat.setup(*mKingSquidModel, sead::SafeStringBase<char>::create("depth_silhouette_color"), NULL);
        mKingSquidAnim = new Game::PlayerAnimCtrlSet(mKingSquidModel, 0, *mPlayer->mPlayerInfo);
    }
    void PlayerKingSquid::enter(){
        //mEffectManualHandle->searchAndEmit("BiWpSpnrDPFulFlashOnetime", 0x168, 0);
        // GuWpSpnrHalfFlash, GuWpSpnrFulFlash, BiWpSpnrDPFulFlashOnetime (discarted), BiSwpStampThrow
        //mEffectManualHandle->setTeamColor(mPlayer->mTeam);
	}
    void PlayerKingSquid::calc(){
        Game::MainMgr *mainMgr = Game::MainMgr::sInstance;
        if(mainMgr == NULL){
            mIsRush = 0;
            mAerialFrm = 0;
            mBullet = NULL;
            return;
        }
        Game::BulletMgr *bulletMgr = mainMgr->mBulletMgr;
        if(bulletMgr == NULL){
            mIsRush = 0;
            mAerialFrm = 0;
            mBullet = NULL;
            return;
        }
        if(!Utils::isSceneLoaded() or mModel == NULL or mKingSquidModel == NULL or mKingSquidAnimCtrl == NULL or mSquidAnimCtrl == NULL or mPlayerJointKingSquid == NULL or mPlayerJointSquid == NULL){
            mIsRush = 0;
            mAerialFrm = 0;
            if(mBullet != NULL){
                mBullet->reserveSleepAll_(Lp::Sys::Actor::ListNodeKind::None);
                mBullet = NULL;
            }
            return;
        }
        if(!mEntered){
            this->enter();
            mEntered = 1;
        }
        //mPlayerMotion->startEventAnim((Game::PlayerMotion::AnimID)0x173, 0.0f, 1.0f);
        if(mPlayer->isInSpecial_KingSquid_Impl(0)){
            if(mPlayerModel->mSquidModel != mKingSquidModel){
                mEffectManualHandle->searchAndEmit("BuPlayerModeSp1Time", 0x168, 0);
                sead::Vector3<float> pos;
                pos.mX = mPlayer->mPosition.mX + (*(float*)(((u64)mPlayer)));
                pos.mY = mPlayer->mPosition.mY + (*(float*)(((u64)mPlayer)));
                pos.mZ = mPlayer->mPosition.mZ + (*(float*)(((u64)mPlayer)));
                sead::Matrix34<float> effMtx;
                memcpy(&effMtx, &mKingSquidModel->mtx, sizeof(sead::Matrix34<float>));
                Utils::setMtxPos(&effMtx, pos);
                mEffectManualHandle->setTeamColor(mPlayer->mTeam);
                mEffectManualHandle->emitParticle(effMtx, mPlayer);  
                mPlayerModel->mSquidModel = mKingSquidModel;
                mPlayer->mPlayerJointSquid = mPlayerJointKingSquid;
                mPlayerMotion->mSquidAnimCtrlSet = mKingSquidAnim;
                mPlayerMotion->mAnimSetCtrlSquid = mKingSquidAnimCtrl;
                memcpy(&mPlayerModel->mSquidModelDepthSilhoutteAccessKey, &mKingSquidMat, sizeof(Cmn::MaterialParamAccessKeySet));
                mKingSquidAnimCtrl->reset();
                mSquidAnimCtrl->reset();
                mPlayerMotion->reset(1, Game::PlayerMotion::ResetMode::cDefault);
                xlink2::Handle tmp;
                mPlayer->mXLink->searchAndEmitWrap("SpecialMode_00_00", false, &tmp);
                mPlayer->mXLink->searchAndPlayWrap("_Add_MissionLastBoss_00", false, &tmp);
                mPlayer->dropHoldingClamAll_ForSpecial();
                if(!mWasInSquid){
                    PlayerKingSquid::enter();
                    mPlayerMotion->startEventAnim((Game::PlayerMotion::AnimID)0x29D, 0.0f, 1.0f);
                    //BuPlayerModeFullChg, BuPlayerModeSP
                }
            }
        } else{
            if(mPlayerModel->mSquidModel != mModel){
                mPlayerModel->mSquidModel = mModel;
                mPlayer->mPlayerJointSquid = mPlayerJointSquid;
                mPlayerMotion->mSquidAnimCtrlSet = mSquidAnim;
                mPlayerMotion->mAnimSetCtrlSquid = mSquidAnimCtrl;
                memcpy(&mPlayerModel->mSquidModelDepthSilhoutteAccessKey, &mSquidMat, sizeof(Cmn::MaterialParamAccessKeySet));
                mKingSquidAnimCtrl->reset();
                mSquidAnimCtrl->reset();
                mPlayerMotion->reset(1, Game::PlayerMotion::ResetMode::cDefault);
            }
            if(mBullet != NULL){
                mBullet->reserveSleepAll_(Lp::Sys::Actor::ListNodeKind::None);
                mBullet = NULL;
            }
            mAerialFrm = 0;
            mIsRush = 0;
            mWasInSquid = mPlayer->isInSquid();
            return;
        }
        mWasInSquid = mPlayer->isInSquid();
        mPlayer->mSwimAnimFrm = 2 * int(!mIsRush);
        Game::PlayerInkAction *InkAction = mPlayer->mPlayerInkAction;
        Game::PlayerMgr *playerMgr = Game::PlayerMgr::sInstance;
        if(InkAction == NULL or playerMgr == NULL){
            mIsRush = 0;
            mAerialFrm = 0;
            if(mBullet != NULL){
                mBullet->reserveSleepAll_(Lp::Sys::Actor::ListNodeKind::None);
                mBullet = NULL;
            }
            return;
        }
        if(mIsRush and Prot::ObfLoad(&mPlayer->mAerialState) != 0){
            if(mBullet == NULL and mPlayer->mIsRemote == 0){
                mBullet = (Game::BulletSpAquaBall*)bulletMgr->activateOneCancelUnnecessary(0x68, 1, mPlayer->mPosition, sead::Vector3<float>::zero, -1);
                if(mBullet != NULL){
                    mBullet->initializeSender(mPlayer, mPlayer->mIndex, mPlayer->mIndex, mPlayer->mPosition);
                }
            }
        } else if(mBullet != NULL){
            mBullet->reserveSleepAll_(Lp::Sys::Actor::ListNodeKind::None);
            mBullet = NULL;
        }
        if(mBullet != NULL){
            sead::Vector3<float> pos;
            mBullet->mTeam = mPlayer->mTeam;
            pos.mX = mPlayer->mPosition.mX + (*(float*)(((u64)mPlayer) + 0x730)) * 3.0f;
            pos.mY = mPlayer->mPosition.mY + (*(float*)(((u64)mPlayer) + 0x734)) * 1.5f;
            pos.mZ = mPlayer->mPosition.mZ + (*(float*)(((u64)mPlayer) + 0x738)) * 3.0f;
            mBullet->setPos(pos);// 
        }
        InkAction->fillInkMagazine();
        if(Prot::ObfLoad(&mPlayer->mAerialState) != 0){
            mAerialFrm+=1;
        } else{
            mAerialFrm = 0;
        }
        if(mAerialFrm > 999){
            mAerialFrm = 999;
        }
        mIsRush = mAerialFrm > 3;
    }
    KingSquidMgr::KingSquidMgr(){
        sInstance = this;
        memset(this, 0, sizeof(KingSquidMgr));
        this->mKingSquidArcs[0] = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Wsp_KingSquid"), NULL, 0, NULL, NULL);
        this->mKingSquidArcs[1] = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Wsp_KingOcto"), NULL, 0, NULL, NULL);
    }
    Lp::Sys::ModelArc *KingSquidMgr::getArc(Starlion::PlayerKingSquid *kingSquid){
        if(kingSquid->mPlayer->mModelType == 2 or kingSquid->mPlayer->mModelType == 3){
            return this->mKingSquidArcs[1];
        }
        return this->mKingSquidArcs[0];
    }
    void KingSquidMgr::onCalc(){
        if(!Utils::isSceneLoaded()){
            return;
        }
        auto iterNode = Game::Player::getClassIterNodeStatic();
        for(Game::Player *ita = (Game::Player *)iterNode->derivedFrontActor(); ita != NULL; ita = (Game::Player *)iterNode->derivedNextActor(ita)){
            Game::PlayerStepPaint *StepPaint = ita->mPlayerStepPaint;
            if(StepPaint == NULL or !ita->isInSpecial_KingSquid_Impl(0)){
                continue;
            }
            StepPaint->mStepPaintType = 5;
        }
    }
};