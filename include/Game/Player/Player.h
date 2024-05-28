#pragma once

#include "types.h"
#include <math.h>

#include "sead/vector.h"
#include "Cmn/Actor.h"
#include "PlayerMotion.h"
#include "PlayerModel.h"
#include "Game/VictoryGoal.h"
#include "Game/VGoalOcta.h"
#include "Game/PlayerMgr.h"
#include "Game/Player/PlayerTrouble.h"
#include "Game/Player/PlayerInkAction.h"
#include "Game/Player/PlayerEffect.h"
#include "Game/Player/PlayerStepPaint.h"
#include "Game/Player/PlayerMove.h"
#include "Game/Player/PlayerSuperArmor.h"
#include "Game/Player/PlayerBehindCamera.h"
#include "Game/Player/PlayerCollision.h"
#include "Game/Player/PlayerCoopSeqRoundSetup.h"
#include "Game/Player/PlayerDokanWarp.h"
#include "Game/Player/PlayerInputVictoryClam.h"
#include "Game/Player/PlayerJetpack.h"
#include "Game/Player/PlayerSuperBall.h"
#include "Game/Player/PlayerComeOnNice.h"
#include "Game/Player/PlayerNetControl.h"
#include "Game/Player/PlayerSuperLanding.h"
#include "Cmn/PlayerCustomMgr.h"
#include "Cmn/PlayerInfo.h"
#include "Game/Gachihoko.h"
#include "Game/VictoryGoal.h"
#include "Game/VGoalOcta.h"
#include "Game/RivalMgr.h"
#include "agl/DrawContext.h"
#include "sead/color.h"
#include "Cmn/Mush/MushDataHolder.h"
#include "Cmn/PlayerWeapon.h"
#include "Game/Player/PlayerDamage.h"
#include "Game/Player/PlayerJointSquid.h"

#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

namespace Game {
	class pCollision{
		public:
		bool isCol;
	};
	class PlayerKingSquid{
		public:
	};
    class Player : public Cmn::Actor {
        public:
        enum ResultAnim{
            Win, Lose, CoopWin, CoopLose
        };
		enum MarkingType{
			m1, m2, m3, m4
		};

		class AccessorForSeqMgrMissionOcta{
			public:
			static void setPos(Game::Player*, sead::Vector3<float> const&);
		};
		
		void clearRequest_Shot();
		void startBarrier_Special();
		void startSpecial_AllMarking();
		void startSpecial_SuperArmor();
		void startSpecial_Barrier();
		void startSpecial_Jetpack();
		void startSpecial_SuperLanding();
		void startSpecial_RainCloud();
		void startSpecial_AquaBall();
		void startSpecial_Impl(int);
		void trigSpecial();
		void calcDraw_In4thCalc();
		void calcCameraInput(bool);
		void resetCalcVelGndOneMore();
		void informLookAt_Shot(sead::Vector3<float> const&);
		void startVGoal_ToDunkShoot_Impl(const Game::VictoryGoal *);
		void startOcta_VGoal_ToDunkShoot_Impl(const Game::VGoalOcta *);
		void resetVGoal_DunkShoot();
        void start_MissionAppear();
		void startMarkingOne_Impl(Game::Player*, Game::Player::MarkingType, int, int);
		void startVGoal(unsigned int, sead::Vector3<float> const&, int, sead::Vector3<float> const&, int);
        void change_DemoPlaceAnim(Game::Player::ResultAnim, int);
        void replaceTeamColor(sead::Color4f const&);
		void informGetWeapon_Impl_(int, int, int, bool);
		void fillInkMagazine();
		void startMarked_Common(int, int, int, Game::Player::MarkingType);
		void startMarked_Sender(int, int, Game::Player::MarkingType, int, unsigned int);
		bool isInSpecial() const;
		bool isInSuperArmor() const;
		bool IsInBarrier() const;
		bool isInSpecial_WaterCutter() const;
		bool isInTrouble_RespawnWait() const;
		bool isInTrouble_Dying() const;
		bool isInTrouble_Vanish() const;
		bool isInTrouble_AirFall() const;
		bool isInTrouble_WaterFall() const;
		bool isModelTypeHeroOcta() const;
		void isInDeviled();
		void onSleep();
		void onActivate();
		int getVClam_HoldingGoldenNum() const;
		int getVClam_HoldingNormalNum() const;
		bool isAerial() const;
		bool isAerial_NoHitGnd() const;
		bool isInSquid() const;
		bool isInSpecial_KingSquid_Impl(bool) const;
		bool isInSpecial_VGoal_Shachihoko() const;
		void informPlayerInfoChanged_Hair();
		void informWorld_ChangeGear(int);
		bool isCreateMantle() const;
		bool isInTrouble_Immovable() const;
		void resetDamage();
		void startDokanWarp_Block(sead::Vector3<float> const&, int, int, unsigned int);
		void fillPaintGauge();
		void dropHoldingClamAll_ForSpecial();
		void finish_DemoPlace();
		void informOcta_Respawn();
		sead::Vector3<float> getWaistPos() const;
		sead::Vector3<float> getHeadPos() const;
		Cmn::PlayerCustomPart *getTank() const;
		Game::PlayerBehindCamera *getBehindCamera_ForTV() const;
		void drawShotGuide();
		void setManualRespawnMode(bool);
		Game::PlayerDamage *selectDamage(bool) const;
		bool isInDokanWarp_Charge() const;
		void shootSuperBall();
		void registTwoshotAnims();
		void resetPaintGauge(bool,bool,bool,bool);
		bool isInSpecial_FreeBombs() const;
		bool isInSpecial_Launcher() const;
		void startMarked_Bomb_Direct(int, int, unsigned int);
		void informOcta_ChangeRivalType(bool);
		void updateMoveVel_WallHit(sead::Vector3<float> const&,sead::Vector3<float> const&);
		float calcPaintInk_RiseRt() const;
		bool isInSquid_Stealth_Move() const;
		void applyWeaponChanged(bool isWeapon);
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();

		bool isAlive(void){
			Game::PlayerTrouble *playerTrouble = this->mPlayerTrouble;
			if(playerTrouble != NULL){
				if(!this->isInTrouble_RespawnWait() and !this->isInTrouble_Dying() and !this->isInTrouble_Vanish() and !this->isInTrouble_AirFall() and !this->isInTrouble_WaterFall()){
					return 1;
				} else{
					return 0;
				}
			} else{
				return 0;
			}
		}

		bool isValidWeapon(Cmn::Def::WeaponKind type){
			Cmn::MushDataHolder* mushData = Cmn::MushDataHolder::sInstance;
			if(mushData != NULL){
				int id = -1;
				switch(type){
				case Cmn::Def::WeaponKind::cMain:
					id = this->mMainWeaponId;
					break;
				case Cmn::Def::WeaponKind::cSub:
					id = this->mSubWeaponId;
					break;
				case Cmn::Def::WeaponKind::cSpecial:
					id = this->mSpecialWeaponId;
					break;
				default:
					id = -1;
					break;
				};
				return mushData->mMushWeaponInfo->getById(type, id) != NULL;
			}
			return 0;
		}

		
		
		u64 _348;
		u32 mIsRemote;
		u32 _354;
		u32 mIsControlled;
		u32 mModelType;
        _BYTE _360[0x98];
		Game::RivalMgr *mRivalMgr;
		u64 *mRivalMgrOcta;
		_BYTE _408[0x40];
		u32 mMainWeaponId;
		u32 mSubWeaponId;
		u32 mSpecialWeaponId;
		_BYTE _454[0x18]; // 2c
		u32 mTankId;
		_BYTE _470[0x10];
        u32 mIndex;
        Cmn::PlayerInfo *mPlayerInfo;
        Game::PlayerMgr *mPlayerMgr;
        _BYTE _498[0x64];
		sead::Vector4<float> mCamRotation;
		u32 _50C;
		float mUpRot;
		_BYTE _514[0x20];
		sead::Vector3<float> mSomeRotation;
		_BYTE _540[0x28];
		u32 mDamage;
		_BYTE _56C[0x6C];
		sead::Vector3<float> mModelRotationDegree1;
		_BYTE _5E4[0x14];
		sead::Vector3<float> mRotationDegree;
		float mRotationDegree2;
		float mRotationUp;
		_BYTE _60C[0x68];
		sead::Vector3<float> mModelRotationDegree3;
		u32 _680;
		sead::Vector3<float> mShotRotation;
		_BYTE _690[0x38]; // B8
		Cmn::PlayerWeapon *mPlayerWeapon[2];
		u64 _6D8;
		Game::PlayerKingSquid *mPlayerKingSquid;
		_BYTE _6E8[0x60];
        sead::Vector3<float> mPosition;
		sead::Vector3<float> mModelRotationDegree4;
        _BYTE _754[0x1B0];
		float mJumpVel;
		_BYTE _914[0x14];
		sead::Vector3<float> mMoveVel;
        _BYTE _934[0x240];
		int mCarryingTornadoFrm;
		int _B78;
		int mBarrierEndFrm;
		int _B80;
		u32 mIsInBarrier; // + 14
		u32 _B88;
		u32 _B8C;
		u32 mIsInMarked;
		u32 _B94;
		u32 mMarkingType;
		_BYTE _B9C[0x50];
		u32 mArmorLevel; // beC ( + 18)
		_BYTE gap_BF0[0x6];
		bool mIsHaveShachihoko; // bf6
		_BYTE _BF7[0x29];
		u32 mGoldenIkuraNum;
		u32 mCoopSpecialNum;
		_BYTE _C28[0xC54 - 0xC28];
		bool mIsEmitDivePtcl;
		_BYTE _C55[0xC7C-0xC55];
		int mSwimAnimFrm;
		u32 _C80;
		u32 mWallActionUnk; // c84
		u32 _C88;
		u32 mAerialState;
		u32 mAerialStateProt; // C90 - c74 = 1C
		_BYTE _C94[0x1B0];
		u32  mVGoalLeftTime;
		bool mIsVGoal1; // +24 (E48 - E24)
		bool mIsVGoal2;
		_BYTE _E4A[0x6];
		Game::VictoryGoal *mVictoryGoal; // + 20
		_BYTE _E58[0x20];
		Game::VGoalOcta *mVictoryGoalOcta; // + 18
		_BYTE _E82[0x58];
		sead::Vector3<float> *mModelSize;
		_BYTE _EC8[0x20];
		Game::PlayerDokanWarp *mPlayerDokanWarp; // + 18
		_BYTE _EF0[0x10];
		Game::PlayerJetpack *mPlayerJetpack; // + 18
		Game::PlayerSuperArmor *mPlayerSuperArmor; // + 18
		Game::PlayerSuperLanding *mPlayerSuperLanding;
		u64 *_F30;
		Game::PlayerSuperBall *mPlayerSuperBall;
		_BYTE _F40[0x10];
		PlayerStepPaint *mPlayerStepPaint;
		PlayerMove *mPlayerMove;
		PlayerTrouble *mPlayerTrouble; // +20
		_BYTE _F68[0x10];
		Game::PlayerInputVictoryClam *mPlayerInputVictoryClam;
		Game::PlayerInkAction *mPlayerInkAction;
		Game::PlayerCollision *mPlayerCollision;
        Game::PlayerModel *mPlayerModel;
        __int64 mPlayerSilhouette; // Game::PlayerSilhouette *
        Game::PlayerMotion *mPlayerMotion;
		u64 _FA8;
		u64 _FB0;
		Game::PlayerEffect *mPlayerEffect;
		u64 _FC0;
		Game::PlayerJointSquid *mPlayerJointSquid;
		u64 _FD0;
		u64 _FD8;
		Cmn::PlayerCustomMgr *mPlayerCustomMgr;
		_BYTE _FE8[0x10];
		Game::PlayerNetControl *mPlayerNetControl;
		_BYTE _1000[0x20];
		Game::PlayerCoopSeqRoundSetup *mPlayerCoopSeqRoundSetup;
		_BYTE _1028[0x20]; // b8
		Game::PlayerComeOnNice *mPlayerComeOnNice;
		_BYTE _1050[0x90];
		u32 mUsingAi;
		u32 mCutsceneState; // + 28 (10e4)
		_BYTE _10E8[0xB8];
		float mPaintPoint; // +5c
		u32 _11A4; // 1148
		float mPaintGauge;
		u32 mSpecialLeftFrame;
		u32 mLayoutSpecialState;
		_BYTE _11B4[0x34];
		Game::PlayerBehindCamera *mPlayerBehindCamera; // +60
    };
};
#endif