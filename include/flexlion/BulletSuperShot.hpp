#pragma once

#include "Cmn/Actor.h"
#include "Cmn/KDGndCol/Manager.h"
#include "Game/PaintUtl.h"
#include "Lp/Utl.h"
#include "sead/vector.h"
#include "sead/matrix.h"

namespace Game {
    class Player;

    class BulletSuperShot {
    public:
        // Cmn::Actor-compatible layout (must be at offset 0)
        _BYTE _actorBase[0x348];

        // Custom fields
        Game::Player *mSender;
        sead::Vector3<float> mPos;
        sead::Vector3<float> mVel;
        sead::Vector3<float> mStartPos;
        sead::Matrix34<float> mXLinkMtx;
        int mFrame;
        bool mActive;
        bool mSystemActive; // true between vtOnActivate and vtOnSleep

        // xlink effect handles
        u64 mBulletEvent;
        u32 mBulletEventId;

        Lp::Utl::StateMachine mStateMachine;

        BulletSuperShot();

        static BulletSuperShot *create(Lp::Sys::Actor *parent, Cmn::Def::Team team);
        void launch(Game::Player *sender, sead::Vector3<float> const &pos, sead::Vector3<float> const &vel);
        void doSleep();

        // State machine callbacks
        void stateEnterFlight();
        void stateFlight();
        void stateEnterIdle();
        void stateIdle();

        // Vtable overrides
        static const char *vtGetClassName(BulletSuperShot *self);
        static void vtFirstCalc(BulletSuperShot *self);
        static void vtOnActivate(BulletSuperShot *self, bool);
        static void vtOnSleep(BulletSuperShot *self);
        static const char *vtGetXlinkName(BulletSuperShot *self);
        static sead::Matrix34<float> *vtGetXLinkMtx(BulletSuperShot *self);
        static sead::Vector3<float> *vtGetXLinkScale(BulletSuperShot *self);
        static bool vtIsCreateSlink(BulletSuperShot *self);
        static int vtSetXLinkLocalPropertyDef(BulletSuperShot *self, int baseCount);
        static int vtCountXLinkLocalProperty(BulletSuperShot *self);
        void setXLinkRootMtx();

        // Accessors for actor base fields
        inline Lp::Sys::Actor *asLpActor() { return (Lp::Sys::Actor *)_actorBase; }
        inline Lp::Sys::XLink *getXLink() { return *(Lp::Sys::XLink **)(_actorBase + 0x320); }
        inline void setTeam(Cmn::Def::Team t) { *(Cmn::Def::Team *)(_actorBase + 0x328) = t; }
        inline void setStateMachineInnerPtr(Lp::Utl::StateMachine *sm) { *(Lp::Utl::StateMachine **)(_actorBase + 0x340) = sm; }

        // Flight parameters
        static constexpr float GRAVITY = 0.075f;
        static constexpr float LAUNCH_SPEED = 22.0f;
        static constexpr int MAX_FLIGHT_FRAMES = 300;

        enum State {
            cState_Flight = 0,
            cState_Idle = 1,
            cState_Count = 2,
        };
    };
}

void initBSSVtable(Game::BulletSuperShot *bss);
