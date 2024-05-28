#pragma once

#include "types.h"

#include "Lp/Sys/actor.h"
#include "Lp/Sys/xlinkiuser.h"
#include "Cmn/Def/Team.h"
#include "Lp/Sys/xlink.h"
#include "sead/vector.h"
#include "sead/bound.h"
#include "Lp/Utl.h"
#include "Cmn/ComponentHolder.h"

namespace Cmn
{
	class ActorVtable{
		public:
		u64 actordtor2;
		u64 actordtor0;
		u64 getDefaultCalcByTiming;
		u64 getDefaultCalcByParent;
		u64 actorLoad;
		u64 actorPostLoad;
		u64 actorEnter;
		u64 actorPostEnter;
		u64 actorReset;
		u64 actorPostReset;
		u64 actorExit;
		u64 actorDestroy;
		u64 actorCalc;
		u64 actorCalc_PostChildCalcByThis;
		u64 actorDbgDraw3D;
		u64 actorDbgDraw2D;
		u64 actorOnCreate;
		u64 actorOnActivate;
		u64 actorOnSleep;
		u64 actorOnRemove;
		u64 dbgOnAfterSleepBeforeReActivate;
		u64 getProcBarColor;
		u64 getDbgMtx;
		u64 getClassName;
		u64 checkDerivedRuntimeTypeInfo;
		u64 getRuntimeTypeInfo;
		u64 getClassIterNode;
		u64 load;
		u64 postLoad;
		u64 enter;
		u64 postEnter;
		u64 exit;
		u64 destroy;
		u64 firstCalc;
		u64 secondCalc;
		u64 thirdCalc;
		u64 fourthCalc;
		u64 fifthCalc;
		u64 xlinkCalc;
		u64 calc_PostChildCalcByThis;
		u64 onCreate;
		u64 onActivate;
		u64 onSleep;
		u64 prepareReset;
		u64 reset;
		u64 postReset;
		u64 isSkipReset;
		u64 checkResetCoreSafe;
		u64 getCameraFocusMtx;
		u64 calcElinkEvent;
		u64 getXLinkMtx;
		u64 getXLinkScale;
		u64 getModelRTMtx;
		u64 getModelScale;
		u64 isUseXlink;
		u64 isXlinkDoSleepEmitAndPlay;
		u64 isCalcXlink;
		u64 isCreateSlink;
		u64 getXlinkName;
		u64 onXlinkSetupModels;
		u64 onXlinkSetupActionSlot;
		u64 onXlinkSetupProperty;
		u64 setXlinkLocalPropertyDefinition;
		u64 countXlinkLocalProperty;
		u64 createAdditionalActionSlot;
		u64 createComponentHolder;
		u64 executeStateMachine;
		u64 resetState;
		u64 changeFirstState;
    };
    class Actor : public Lp::Sys::Actor, public Lp::Sys::XLinkIUser
    {
          public:
        
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        void setModelScale(float scale){
			if(this->mActorFullModel != NULL){
				this->mActorFullModel->mVel.mX=scale;
				this->mActorFullModel->mVel.mY=scale;
				this->mActorFullModel->mVel.mZ=scale;
			}
        };
        void mulModelScale(float mul){
			if(this->mActorFullModel != NULL){
				this->mActorFullModel->mVel.mX*=mul;
				this->mActorFullModel->mVel.mY*=mul;
				this->mActorFullModel->mVel.mZ*=mul;
			}
        };
        sead::Vector3<float> getModelPos(){
			sead::Vector3<float> mPos = {0, 0, 0};
			sead::BoundBox3<float> bounds;
			if(this->mActorFullModel != NULL){
				this->mActorFullModel->calcBoundAABB(&bounds, false);
				mPos.mX = (bounds.mMax.mX + bounds.mMin.mX) / 2;
				mPos.mY = (bounds.mMax.mY + bounds.mMin.mY) / 2;
				mPos.mZ = (bounds.mMax.mZ + bounds.mMin.mZ) / 2;
			}
			return mPos;
		};
        bool isActive(){
			if(this->mXLink != NULL){
				return !this->mXLink->isSleep();
			}
			return 0;
        };
        void setInnerStateMachinePtr(Lp::Utl::StateMachine *ptr){
          	mStateMachineInnerPtr = ptr;
        };
		
        Lp::Sys::XLink* mXLink; // Lp::Sys::XLink*
        Cmn::Def::Team mTeam;
        u32 CmnActor_32C;
        ComponentHolder* mComponentHolder; 
        gsys::Model *mActorFullModel;
        Lp::Utl::StateMachine *mStateMachineInnerPtr; 
  	};
};