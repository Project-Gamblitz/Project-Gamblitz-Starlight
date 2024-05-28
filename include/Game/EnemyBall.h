#pragma once

#include "types.h"
#include "Cmn/Def/ResetType.h"
#include "Cmn/Actor.h"
#include "Game/Enemy.h"
#include "Cmn/GfxUtl.h"
#include "Game/Cmp/Movable.h"
#include "Game/Cmp/FrontDirRotatable.h"
#include "Game/Cmp/Rollable.h"
#include "Game/Cmp/BulletKnockbackable.h"

namespace Game{
    class CollisionEventArg{
        public:
    };
    class EnemyBallParams : public EnemyParams{

    };
    class EnemyBall : public Game::EnemyBase{
        public:
        EnemyBall();
        ~EnemyBall();
        static void destroy(Game::EnemyBase *);
        const char *getClassName() const;
        bool checkDerivedRuntimeTypeInfo(sead::RuntimeTypeInfo::Interface const *) const;
        sead::RuntimeTypeInfo::Interface* getRuntimeTypeInfo() const;
        Lp::Sys::ActorClassIterNodeBase *getClassIterNode();
        void setXLinkLocalPropertyDefinition_(int);
        void countXLinkLocalProperty_() const;
        void executeStateMachine_();
        void changeFirstState_();
        void load_();
        void reset_(Cmn::Def::ResetType);
        void firstCalc_();
        void createDesignerParams();
        bool isCreateDesignerParamsImpl_() const;
        EnemyBallParams *createMergedParams_();
        float getPlayerDamage() const;
        void interactCol(Game::CollisionEventArg const&) const;
        void createEnemyParams_();
        void calcTrackPaintable_(); // now called in firstCalc_
        void calcChangeState_(); // now called in firstCalc_
        void calcRollable_(); // now called in firstCalc_
        void calcFDRotatable_(); // now called in firstCalc_
        float getSpeed_() const;

        void stateEnterWait_();
        void stateWait_();
        void stateEnterNotice_();
        void stateNotice_();
        void stateEnterMove_();
        void stateMove_();
        void stateEnterDie_();
        void stateDie_();

        Lp::Utl::StateMachine mStateMachine;
        Cmp::Movable *mMovable;
        Cmp::FrontDirRotatable *mFrontDirRotatable;
        Cmp::Rollable *mRollable;
        Cmp::BulletKnockbackable *mBulletKnockbackable;
    };
};