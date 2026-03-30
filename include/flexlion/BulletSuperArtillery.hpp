#pragma once

#include "Cmn/Actor.h"
#include "Lp/Utl.h"
#include "sead/vector.h"
#include "sead/matrix.h"

namespace Game {
    class Player;
}

namespace Flexlion {

enum BSAState {
    cState_Pronounce = 0,
    cState_Wait = 1,
    cState_Burst = 2,
    cState_Aim = 3,
    cState_Count = 4,
};

class BulletSuperArtillery {
public:
    // Cmn::Actor-compatible layout (must be at offset 0)
    _BYTE _actorBase[0x348];

    // Custom fields
    Game::Player *mSender;
    gsys::Model *mTornadoModel;
    sead::Vector3<float> mFrom;
    sead::Vector3<float> mTo;
    sead::Vector3<float> mPos;
    sead::Vector3<float> mRot;
    int mStartFrm;
    bool mFlightActive;
    bool mHasBurst;
	bool mMatchEnding;
    sead::Matrix34<float> mXLinkMtx;
    float mBurstRadius;
    int mBurstFrm;

    Lp::Utl::StateMachine mStateMachine;

    BulletSuperArtillery();

    static BulletSuperArtillery *create(Lp::Sys::Actor *parent, gsys::Model *model, Cmn::Def::Team team);

    void prepare(Game::Player *sender);
    void launch(sead::Vector3<float> src, sead::Vector3<float> dst, int paintgamefrm, bool forcedByMatchEnd = false);
    void cancel();
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
    static sead::Vector3<float> *vtGetXLinkScale(BulletSuperArtillery *self);

    // State machine callbacks
    void stateEnterPronounce();
    void statePronounce();
    void stateEnterWait();
    void stateWait();
    void stateEnterBurst();
    void stateBurst();
    void stateEnterAim();
    void stateAim();

    // Helpers
    void calcTankBone();
    void calcFlight();
    void doBurst();
    void calcBurstFollow();
    void calcBurstPaintAndDamage();
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
