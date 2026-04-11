#pragma once

#include "Cmn/Actor.h"
#include "Lp/Utl.h"
#include "sead/vector.h"
#include "sead/matrix.h"
#include "Game/Gachihoko.h"

namespace Game {
    class Player;
	class Sprinkler : public Cmn::Actor {
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
	class Shield : public Cmn::Actor {
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
	class JumpFlag : public Cmn::Actor {
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
	class BulletUmbrellaCanopyBase : public Cmn::Actor {
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
	class SpongeVersus : public Cmn::Actor {
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
	class BulletBombTako : public Cmn::Actor {
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
	class IidaBombOnline : public Cmn::Actor {
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
	class Blowouts : public Cmn::Actor {
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
	class BulletShield : public Cmn::Actor {
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
	class BulletBombChase : public Cmn::Actor {
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
	class BulletSpSuperStamp : public Cmn::Actor {
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
	class BulletSpSuperBubble : public Cmn::Actor {
	public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
	};
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
	void eatBombs(float radiusSq);void eatActorClass(Lp::Sys::ActorClassIterNodeBase *iterNode, float radiusSq, int reactionType, int vtableOffset = 744);

    // Vtable overrides (static so they're plain function pointers)
    static const char *vtGetClassName(BulletSuperArtillery *self);
    static void vtFirstCalc(BulletSuperArtillery *self);
	static void vtSecondCalc(BulletSuperArtillery *self);
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
    void updateModelMatrix();
    void doSleep();

    // Accessors for actor base fields at known offsets
    inline Lp::Sys::Actor *asLpActor() { return (Lp::Sys::Actor *)_actorBase; }
    inline Lp::Sys::XLink *getXLink() { return *(Lp::Sys::XLink **)(_actorBase + 0x320); }
    inline void setActorFullModel(gsys::Model *m) { *(gsys::Model **)(_actorBase + 0x338) = m; }
    inline gsys::Model *getActorFullModel() { return *(gsys::Model **)(_actorBase + 0x338); }
    inline void setStateMachineInnerPtr(Lp::Utl::StateMachine *sm) { *(Lp::Utl::StateMachine **)(_actorBase + 0x340) = sm; }
    inline void setTeam(Cmn::Def::Team t) { *(Cmn::Def::Team *)(_actorBase + 0x328) = t; }
	static inline sead::Vector3<float> getActorModelPos(void *actor) {
    gsys::Model *model = *(gsys::Model **)((u8 *)actor + 0x338);
    if (model) {
        return { model->mtx.matrix[0][3], model->mtx.matrix[1][3], model->mtx.matrix[2][3] };
    }
    return sead::Vector3<float>::zero;
}
};

void initBSAVtable(BulletSuperArtillery *bsa);
void resetBSAStatics();

} // namespace Flexlion
