#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/Actor.h"
#include "Game/Obj.h"
#include "Cmn/PlayerInfo.h"
#include "Game/Cmp/AnimSetController.h"
#include "Game/EnemyFunctions.h"
#include "Game/Cmp/EnemyPhysics.h"

namespace Game {
    class Player;
    class EnemyBaseVtable : public Cmn::ActorVtable{
        public:
        u64 onCreate_;
        u64 load_;
        u64 postLoad_;
        u64 enter_;
        u64 postEnter_;
        u64 prepareReset_;
        u64 reset_;
        u64 postReset_;
        u64 onActivate_;
        u64 onSleep_;
        u64 firstCalc_;
        u64 secondCalc_;
        u64 thirdCalc_;
        u64 fourthCalc_;
        u64 calcModelMtx;
        u64 calcRequestDraw;
        u64 onXLinkGetReservedAssetName;
        u64 informAndReactCollision;
        u64 informAndReactCollision_AUX;
        u64 receiveAttackEvent;
        u64 isActivateOnEnter;
        u64 isActivateOnMissionRespawn;
        u64 isChangeFirstStateOnActivate;
        u64 isCallOnGenerate_;
        u64 informCheckPoint;
        u64 storeExistOnSave;
        u64 informOctaFirstCheckPoint;
        u64 isAbleToBeCulled;
        u64 calcCheckCulling;
        u64 checkCulling_;
        u64 baseEnterCulling_;
        u64 baseExitCulling_;
        u64 enterCulling_;
        u64 exitCulling_;
        u64 isCalcModelMtxInCullingFirstFrame_;
        u64 isAbleToBeDrawCulled;
        u64 checkDrawCulling_;
        u64 baseEnterDemoStop_;
        u64 baseExitDemoStop_;
        u64 enterDemoStop_;
        u64 exitDemoStop_;
        u64 emitAndPlayHitEffectForSpectatorBehindCamera;
        u64 calcHitEffectPosForSpectatorBehindCamera_;
        u64 getArmorTypeForSpectatorBehindCamera;
        u64 getBaseDamageForSpectatorBehindCamera;
        u64 actorCBBeforeModelApplyAnimation;
        u64 actorCBBeforeModelUpdateWorldMatrix;
        u64 actorCBAfterModelUpdateWorldMatrix;
        u64 onPostCreateParams_;
        u64 onPrevModelLoad_;
        u64 onPostModelLoad_;
        u64 isAllowMuObjScale;
        u64 isSleepOnMissionRespawn;
        u64 onSetActorDBData_;
        u64 getActorName;
        u64 getResName;
        u64 getModelName;
        u64 getKColName;
        u64 getSLinkName;
        u64 setDead;
        u64 getObjPaintIndex;
        u64 createObjPaint_;
        u64 setSwitchStateOnEnter;
        u64 prepareTagOnEnter;
        u64 checkBulletGuideCollision;
        u64 isShotGuideTypeBrokenObjGndCol;
        u64 getBindBoneName;
        u64 getParamsFileBaseName;
        u64 createFamilyParams;
        u64 getDesignerParamsFileBaseName;
        u64 createDesignerParams;
        u64 isCreateDesignerParams_;
        u64 isCreateDesignerParamsImpl_;
        u64 createAnimInterpParams_;
        u64 createMergedParams_;
        u64 createParams_;
        u64 getFamilyParamsMgr_;
        u64 isCreateModel_;
        u64 createModelAndAnim_;
        u64 countAnimSlot_;
        u64 findExternalShaderArchive_;
        u64 createAnimSetCtrl_;
        u64 createActorBindable;
        u64 createBoneBindable;
        u64 createBlockKColHolder;
        u64 isActorReuseAbleSleep;
        u64 createPresetComponents_;
        u64 getComponentSkipFlagInCulling_;
        u64 getComponentSkipFlagInDemoStop_;
        u64 calcPrepareBindModel;
        u64 onSwitchCallback_;
        u64 onTagCallback_;
        u64 baseAddHIONode_;
        u64 addHIONodeFamily_;
        u64 addHIONode_;
        u64 onParamsChanged_;
        u64 postOnParamsChanged_;
        u64 hitEnemyNearbyAttack;
        u64 damage;
        u64 damageMotal;
        u64 getPlayerDamage;
        u64 interactCol;
        u64 unk0; // added post 3.1.0
        u64 getMapObjDamage;
        u64 isSearchBombTarget;
        u64 getSearchPos;
        u64 getSearchTargetPos;
        u64 getBombSplashNrm;
        u64 interactBomb;
        u64 isSuperMissileTarget;
        u64 getSuperMissileTargetPos;
        u64 setSlosherThroughFieldEnemy;
        u64 allowActivateFromWoodenBox;
        u64 interactDeathEvent;
        u64 unk1; // added post 3.1.0
        u64 isInformDeathEvent_;
        u64 calcPlayerCollision_;
        u64 preInformHitBullet_;
        u64 createEnemyPhysics_;
        u64 createDamageEffectable_;
        u64 createPixelMonitorDiskHolder_;
        u64 createDamagePaintAnimable_;
        u64 createItemDropable_;
        u64 createSilhouettableOcta_;
        u64 createEnemyParams_;
        u64 calcMaxLife_;
        u64 calcDamage_;
        u64 geLifeRatioForPaintAnim_;
        u64 baseDieSleep_;
        u64 getPaintDeathOffset_;
        u64 getDropMtx;
        u64 checkIsDropItem_;
        u64 isDropIkura_;
        u64 getDropIkuraBaseNum_;
        u64 firstCalc_InGenerate_;
        u64 isCureLife_;
        u64 isCallPlayerVoice_;
        u64 getAirBallAttachModel_;
    };
    class ModelCB;
    class EnemyParams{
        public:
        _BYTE _0[0xC0];
    };
    class EnemyBase : public Game::Obj{
        public:
        EnemyBaseVtable **getEnmVtable(){
            return (EnemyBaseVtable**)this;
        };
        sead::Vector3<float> getSearchPos() const;
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        void startGenerate();
        ModelCB *mModelCB;
        _BYTE _420[0x7D];
        bool _49D;
        _BYTE _49E[0x4D8 - 0x49E];
        Game::Cmp::AnimSetController *mAnimSetController;
        _BYTE _4E0[0x4F8 - 0x4E0];
        u32 mIsDead;
        _BYTE _4FC[0x78];
        u32 mHealth; // 574
        u32 mHealth2;
        _BYTE _57C[0x590 - 0x57c];
        Game::Cmp::EnemyPhysics *mEnemyPhysics;
        _BYTE _598[0x668 - 0x598];
        u32 _668;
        u32 _66C;
        bool _670;
        bool _671;
        bool mIsInSpawning;
        bool _673;
        _BYTE _674[0x4];
    };
    static_assert(sizeof(Game::EnemyBase) == 0x678, "sizeof(Game::EnemyBase) is invalid!");
    class EnemyBossBaseOcta : public Game::EnemyBase {
        public:
        void onActivate();
        void firstCalc();
    };
	class EnemyLastKing : public Game::EnemyBase {
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        void vtable();
        void calcMove_();
        void onActivate_();
        void firstCalc_();
        void secondCalc_();
        void fourthCalc_();
        void onCreate_();
        void endDemo();
    };
    class EnemyLastKingPunch : public Game::EnemyBase{
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
    class EnemyRollerKingOcta : public Game::EnemyBase {
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        virtual void vtable();
        void stateEnterPatrol_();
    };
    class EnemyFlyingHoheiOcta : public Game::EnemyBase {
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
    class EnemyTakopterOcta : public Game::EnemyBase {
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
    class EnemyTakopterBombOcta : public Game::EnemyBase {
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
    class RivalPlayerInfo{
        public:
        Game::Player *getPlayer() const;
    };
    class RivalSquad : public Game::Obj {
        public:
        void onDie(bool,bool);
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        _BYTE _414[0x164];
        Game::RivalPlayerInfo *mPlayerInfo;
        _BYTE _580[0x7D8];
        Lp::Utl::StateMachine mStateMachine;
    };
    class RivalSquadController{
        public:
        float getShotInitVel_() const;
        _BYTE _0[0x30];
        Game::RivalSquad *mSquad;
    };
    class RivalPlayerInfoOcta{
        public:
        Game::Player *getPlayer() const;
    };
    class RivalSquadOcta : public Game::Obj {
        public:
        void onDie(bool,bool);
        void customizePlayerInfo(Cmn::PlayerInfo *);
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        _BYTE _414[0x164];
        Game::RivalPlayerInfoOcta *mPlayerInfo;
        _BYTE _580[0x850];
        Lp::Utl::StateMachine mStateMachine;
    };
    class RivalSquadControllerOcta{
        public:
        float getShotInitVel_() const;
        _BYTE _0[0x30];
        Game::RivalSquadOcta *mSquad;
    };
    class EnemyHangedKing : public Game::EnemyBase {
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
};