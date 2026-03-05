#pragma once

#include "Cmn/Actor.h"
#include "Lp/Utl.h"
#include "sead/vector.h"
#include "sead/matrix.h"
#include "nn/vfx/System.h"

namespace Game {
    class Player;
    class BulletSpSuperBall;
}

namespace Flexlion {

enum BSAState {
    cState_Pronounce = 0,
    cState_Wait = 1,
    cState_Burst = 2,
    cState_Count = 3,
};

class BulletSuperArtillery {
public:
    // Cmn::Actor-compatible layout (must be at offset 0)
    _BYTE _actorBase[0x348];

    // Custom fields
    Game::Player *mSender;
    gsys::Model *mTornadoModel;
    Game::BulletSpSuperBall *mSuperball;
    sead::Vector3<float> mFrom;
    sead::Vector3<float> mTo;
    sead::Vector3<float> mPos;
    sead::Vector3<float> mRot;
    int mStartFrm;
    bool mFlightActive;
    bool mHasBurst;
    sead::Matrix34<float> mXLinkMtx;
    nn::vfx::EmitterSet *mLaserESet;
    nn::vfx::EmitterSet *mLaserIconESet;
    int mCaptureEmitCounter;

    Lp::Utl::StateMachine mStateMachine;

    BulletSuperArtillery();

    static BulletSuperArtillery *create(Lp::Sys::Actor *parent, gsys::Model *model, Cmn::Def::Team team);

    void launch(Game::Player *sender, sead::Vector3<float> src, sead::Vector3<float> dst, int paintgamefrm);
    bool isActive() const;
    void reset();

    // Vtable overrides (static so they're plain function pointers)
    static const char *vtGetClassName(BulletSuperArtillery *self);
    static void vtFirstCalc(BulletSuperArtillery *self);
    static void vtFourthCalc(BulletSuperArtillery *self);
    static void vtOnActivate(BulletSuperArtillery *self, bool);
    static void vtOnSleep(BulletSuperArtillery *self);
    static int vtSetXLinkLocalPropertyDef(BulletSuperArtillery *self, int idx);
    static int vtCountXLinkLocalProperty(BulletSuperArtillery *self);
    static sead::Matrix34<float> *vtGetXLinkMtx(BulletSuperArtillery *self);
    static void vtCalcElinkEvent(BulletSuperArtillery *self, const void *eventArg);
    static void vtXlinkCalc(BulletSuperArtillery *self);

    // State machine callbacks
    void stateEnterPronounce();
    void statePronounce();
    void stateEnterWait();
    void stateWait();
    void stateEnterBurst();
    void stateBurst();

    // Helpers
    void calcFlight();
    void doBurst();
    void calcBurstFollow();
    void updateModelMatrix();
    void doSleep();

    // Accessors for actor base fields at known offsets
    inline Lp::Sys::Actor *asLpActor() { return (Lp::Sys::Actor *)_actorBase; }
    inline Lp::Sys::XLink *getXLink() { return *(Lp::Sys::XLink **)(_actorBase + 0x320); }
    inline void setActorFullModel(gsys::Model *m) { *(gsys::Model **)(_actorBase + 0x338) = m; }
    inline gsys::Model *getActorFullModel() { return *(gsys::Model **)(_actorBase + 0x338); }
    inline void setStateMachineInnerPtr(Lp::Utl::StateMachine *sm) { *(Lp::Utl::StateMachine **)(_actorBase + 0x340) = sm; }
    inline void setTeam(Cmn::Def::Team t) { *(Cmn::Def::Team *)(_actorBase + 0x328) = t; }
};

void initBSAVtable(BulletSuperArtillery *bsa);
void resetBSAStatics();

} // namespace Flexlion
