#pragma once

#include "types.h"
#include "Cmn/PlayerCustomPart.h"
#include "Cmn/Def/WeaponKind.h"

namespace Cmn {
    class PlayerWeaponVtable : public Cmn::ActorVtable{
        public:
        u64 setupWithoutModel;
        u64 beforeCalcDraw;
        u64 onRequestDraw;
        u64 onSetVisible;
        u64 getAlphaMin;
        u64 onSetXluAlpha;
        u64 changeTeamColor;
        u64 onSetEnableDepthSilhouette;
        u64 onSetDepthSilhouetteColor;
        u64 onChangeControlledPlayer;
        u64 standAlone;
        u64 isModelResHasAnim;
        u64 isCreateAnim;
        u64 onAttach;
        u64 setupWithModel;
        u64 createModel_Custom;
        u64 beforeModelUpdateWorldMatrix;
        u64 afterModelUpdateWorldMatrix;
        u64 setLinkUserName;
        u64 calcDraw_ForLinkAction;
        u64 setLinkAction;
        u64 getLinkAction;
        u64 releaseLinkAction;
        u64 setChargeRate;
        u64 setChargeKeepFrm;
        u64 setFovy;
        u64 setMuzzleShotDirXZDot;
        u64 setShooterVariableFrm;
        u64 checkAndPlayCurInkAction_Final;
    };
    class PlayerWeapon : public Cmn::PlayerCustomPart {
	public:
    PlayerWeapon();
    bool has2(Cmn::Def::WeaponKind,int);
    u32 _358;
    u32 mWeaponId;
    u32 _360;
    u32 mPartId;
    };
    class PlayerWeaponSuperMissile : public PlayerWeapon{
        public:
        PlayerWeaponSuperMissile();
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
    class PlayerWeaponShooter : public PlayerWeapon{
        public:
        PlayerWeaponShooter();
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
};