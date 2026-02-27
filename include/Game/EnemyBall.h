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

namespace Lp { namespace Sys {
    template<typename T>
    struct FamilyParam {
        _BYTE _node[0x2C];
        T mValue;
        _BYTE _pad[0x30 - 0x2C - sizeof(T)];
    };
}}

namespace Game{
    class Player;
    class Bullet;

    namespace Cmp {
        class SearchablePlayerParams;
        class BoneDirectableParams;
        class SearchableBase {
        public:
            void search();
            bool isLockOn() const;
            _BYTE _0[0xF8];
        };
        class SearchablePlayer : public SearchableBase {
        public:
            SearchablePlayer(Cmn::ComponentHolder *, SearchablePlayerParams const*, Game::Actor const*);
            sead::Vector3<float> *getTargetPos() const;
            _BYTE _F8[0x140 - 0xF8];
        };
        class SearchablePlayerParams {
        public:
            SearchablePlayerParams();
            inline void readFamilyParams(const void *familyParams){
                mFamilyParams = familyParams;
            };
            _BYTE _0[0x10];
            const void *mFamilyParams;
            _BYTE _18[0x20 - 0x18];
        };
        class SearchablePlayerParamsFamily {
        public:
            _BYTE _0[0x4C0];
        };
        class TrackPaintable {
        public:
            void init_(Cmn::Def::Team team, float radius);
            _BYTE _0[0x48];
            bool mIsEnabled;
            _BYTE _49[0x7];
            sead::Vector3<float> mPos;
            sead::Vector3<float> mNrm;
            _BYTE _68[0x10];
        };
        class BoneDirectableToSearch {
        public:
            BoneDirectableToSearch(Cmn::ComponentHolder *, BoneDirectableParams const*, int, Game::ModelCB *, SearchablePlayer const*);
            _BYTE _0[0x50];
            int *mBoneIndices;
            _BYTE _58[0x100 - 0x58];
        };
        class SinkableParams {
        public:
            SinkableParams();
            u64 _vtable;
            u64 _08;
            const void *mFamilyParams;   // +0x10
            const void *mDesignerParams; // +0x18
        };
        struct BulletKnockbackableParamsDesigner {
            _BYTE _params[0x38]; // Lp::Sys::Params base
            Lp::Sys::FamilyParam<float> mImpactBulletZ;
            Lp::Sys::FamilyParam<float> mSinkRate;
            Lp::Sys::FamilyParam<float> mImpactVelGs;
        }; // size 0xC8
        class BulletKnockbackableParams {
        public:
            BulletKnockbackableParams();
            void readDesignerParams(BulletKnockbackableParamsDesigner const*);
            u64 _vtable;
            u64 _08;
            const void *mFamilyParams;   // +0x10
            const void *mDesignerParams; // +0x18
            float mImpactBulletZ;        // +0x20
            float mSinkRate;             // +0x24
            float mImpactVelGs;          // +0x28
            u32 _pad;                    // +0x2C
        };
        class BoneDirectableParams {
        public:
            BoneDirectableParams();
            u64 _vtable;
            u64 _08;
            const void *mFamilyParams;   // +0x10
            const void *mDesignerParams; // +0x18
        };
    }

    class CollisionEventArg {
    public:
        void *mCollidee;
    };

    class EnemyBallParamsFamily : public EnemyParamsFamily {
    public:
        EnemyBallParamsFamily();

        Lp::Sys::FamilyParam<float> mSpeed;                 // +0x558
        Lp::Sys::FamilyParam<float> mSpeedPL;               // +0x588
        Lp::Sys::FamilyParam<float> mAcc;                   // +0x5B8
        Lp::Sys::FamilyParam<float> mRotRate;               // +0x5E8
        Lp::Sys::FamilyParam<float> mRotSpeed;              // +0x618
        Lp::Sys::FamilyParam<float> mRotSpeedPL;            // +0x648
        Lp::Sys::FamilyParam<float> mTrackPaintRadius;      // +0x678
        Lp::Sys::FamilyParam<float> mImpactToPlayer;        // +0x6A8
        Lp::Sys::FamilyParam<float> mImpactToPlayerX;       // +0x6D8
        Lp::Sys::FamilyParam<float> mImpactBulletX;         // +0x708
        Lp::Sys::FamilyParam<int>   mDamagePlayer;          // +0x738
        Lp::Sys::FamilyParam<float> mMonitorHalfSize;       // +0x768
        Lp::Sys::FamilyParam<float> mTrackPaintableOffsetZ; // +0x798
        Lp::Sys::FamilyParam<bool>  mMuteki;                // +0x7C8
        Lp::Sys::FamilyParam<bool>  mMutekiBulletIgnore;    // +0x7F8
        Lp::Sys::FamilyParam<int>   mChanceSinkFrame;       // +0x828
        Lp::Sys::FamilyParam<int>   mMutekiRefreshFrame;    // +0x858
        Lp::Sys::FamilyParam<bool>  mIsMutekiChanceChanging;// +0x888
        Lp::Sys::FamilyParam<bool>  mIsMutekiNormalChanging;// +0x8B8
        Lp::Sys::FamilyParam<float> mChanceStopRollAngle;   // +0x8E8
        Lp::Sys::FamilyParam<int>   mReturnWaitFrame;       // +0x918
        Lp::Sys::FamilyParam<bool>  mStopMoveOnReturn;      // +0x948
        Lp::Sys::FamilyParam<float> mTrackPaintableRadius;  // +0x978
        Lp::Sys::FamilyParam<float> mBulletHitPaintRadius;  // +0x9A8
        Cmp::SearchablePlayerParamsFamily mSearchablePlayerParamsFamily; // +0x9D8
    };

    struct SinkableParamsDesigner {
        _BYTE _params[0x38]; // Lp::Sys::Params base
        Lp::Sys::FamilyParam<float> mSinkAcc;
        Lp::Sys::FamilyParam<float> mSinkSpeed;
        Lp::Sys::FamilyParam<float> mOfstMax;
        Lp::Sys::FamilyParam<float> mSpringK;
        Lp::Sys::FamilyParam<float> mSpringDamp;
        Lp::Sys::FamilyParam<bool> mSpringEnable;
    }; // size 0x158

    struct BoneDirectableParamsDesigner {
        _BYTE _params[0x38]; // Lp::Sys::Params base
        Lp::Sys::FamilyParam<float> mAngleLimitP;
        Lp::Sys::FamilyParam<float> mAngleLimitY;
        Lp::Sys::FamilyParam<float> mLerpRate;
    }; // size 0xC8

    class EnemyBallParamsDesigner : public EnemyParamsDesigner {
    public:
        EnemyBallParamsDesigner();
        Cmp::BulletKnockbackableParamsDesigner mBulletKnockbackableDesigner; // +0x220
        SinkableParamsDesigner mSinkableDesigner;                            // +0x2E8
        BoneDirectableParamsDesigner mBoneDirectableDesigner;                // +0x440
    };

    struct EnemyBallParamsVtable {
        u64 dtor_D2;
        u64 dtor_D0;
        u64 checkDerivedRuntimeTypeInfo;
        u64 getRuntimeTypeInfo;
        u64 readMuObj;
        u64 readFamilyParams;
        u64 readDesignerParams;
    };

    class EnemyBallParams : public EnemyParams {
    public:
        EnemyBallParams();
        ~EnemyBallParams();

        void readMuObj(Cmn::MuObj const*,sead::SafeStringBase<char> const&);
        void readFamilyParams(Game::EnemyBallParamsFamily const*);
        void readDesignerParams(Game::EnemyBallParamsDesigner const*);

        Cmp::BulletKnockbackableParams mBulletKnockbackableParams; // +0xC0
        Cmp::SinkableParams mSinkableParams;                       // +0xF0
        Cmp::SearchablePlayerParams mSearchablePlayerParams;       // +0x110
        Cmp::BoneDirectableParams mBoneDirectableParams;           // +0x130
    };

    class EnemyBall : public Game::EnemyBase {
    public:
        EnemyBall();
        ~EnemyBall();
        static void destroy(Game::EnemyBase *);

        const char *getClassName() const;
        bool checkDerivedRuntimeTypeInfo(sead::RuntimeTypeInfo::Interface const *) const;
        sead::RuntimeTypeInfo::Interface* getRuntimeTypeInfo() const;
        Lp::Sys::ActorClassIterNodeBase *getClassIterNode();

        int setXLinkLocalPropertyDefinition_(int);
        int countXLinkLocalProperty_() const;
        void executeStateMachine_();
        void changeFirstState_();

        void load_();
        void reset_(Cmn::Def::ResetType);
        void firstCalc_();
        EnemyBallParamsDesigner *createDesignerParams();
        bool isCreateDesignerParamsImpl_() const;
        EnemyBallParams *createMergedParams_();
        EnemyBallParamsFamily *createEnemyParams_();

        float getPlayerDamage() const;
        void interactCol(Game::CollisionEventArg const&);
        float getSpeed_() const;

        void stateEnterWait_();
        void stateWait_();
        void stateEnterNotice_();
        void stateNotice_();
        void stateEnterMove_();
        void stateMove_();
        void stateEnterDie_();
        void stateDie_();

        void calcSearchOwnerMtx_();
        void calcSearch_();
        void calcChangeAnim_();
        void calcMuteki_();
        void calcRoll_();
        void getMoveVel_(sead::Vector3<float> *out);
        void calcRollableForce_(float angle);
        void calcRollable_(bool isOnGround);
        void calcFDRotatable_();
        float getRotSpeed_() const;
        void calcCheckGndColMoveSphere_();
        void calcChangeState_();
        bool checkIsFall_();
        void rotMtxByFrontDir_();
        void onHitWall_();
        void onHitKeepOutWall_(sead::Vector3<float> const& prevVel);
        void onHitImpl_(sead::Vector3<float> const& hitPos, sead::Vector3<float> const& hitNrm);
        void interactColPlayer_(Game::Player *);
        void interactColBullet_(Game::Bullet *);
        void paintHitBullet_();
        void setDirectionToPhysics_();
        void calcTrackPaintable_();
        void setObjColMuteki_();

        Lp::Utl::StateMachine mStateMachine;
        Cmp::Movable *mMovable;
        Cmp::FrontDirRotatable *mFrontDirRotatable;
        Cmp::Rollable *mRollable;
        Cmp::BulletKnockbackable *mBulletKnockbackable;
        Cmp::AnimSetController *mChanceAnimSetController;
        Cmp::TrackPaintable *mTrackPaintable;
        Cmp::SearchablePlayer *mSearchablePlayer;
        Cmp::BoneDirectableToSearch *mBoneDirectableToSearch;
        sead::Vector3<float> mSearchDir;
        sead::Vector3<float> mCrossDir;
        sead::Vector3<float> mSearchOwnerFwd;
        sead::Vector3<float> mPrevPos;
        u32 mBumpSfxTimer;
        u32 mHitWallCounter;
        u32 mMutekiState;
        u32 mHitKeepOutTimer;
        u32 mPlayerHitTimer;
        u8 mDrownState;
        u8 mIsReturning;
        u8 mWasHitWall;
        u8 _padState;
        inline EnemyBallParams *getMergedParams() const {
            return (EnemyBallParams *)mStoredMergedParams;
        }
        inline EnemyBallParamsFamily *getFamilyParams() const {
            return *(EnemyBallParamsFamily **)((char *)mStoredMergedParams + 0x10);
        }
    };
};
