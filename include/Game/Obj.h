#pragma once

#include "types.h"
#include "Cmn/Actor.h"
#include "string.h"
#include "sead/vector.h"
#include "Lp/Utl.h"
#include "Cmn/KDUtl/AttT.h"

namespace Game {
	class Actor : public Cmn::Actor{
		public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
		void setScale(float);
		void calcRequestDraw();
		sead::Vector3<float> getModelOrNormalPos(){
			if(this->mActorFullModel != NULL){
				return this->getModelPos();
			}
			return this->mPos;
		}
		Cmn::KDUtl::AttT *getKdAttT(){
			return (Cmn::KDUtl::AttT *)&mtx00;
		};
		Cmn::KDUtl::MtxT mKdMtx;
		float mtx00;
		float mtx10;
		float mtx20;
		float mtx01;
		float mtx11;
		float mtx21;
		float mtx02;
		float mtx12;
		float mtx22;
        sead::Vector3<float> mPos;
		_BYTE _3A8[0x60];
		sead::Vector3<float> mScale;
		int _414;
	};
	class Obj : public Game::Actor{
		public:
		static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
		void setScale(float scale){
			this->mScale.mX=scale;
			this->mScale.mY=scale;
			this->mScale.mZ=scale;
			if(this->mActorFullModel != NULL){
				this->mActorFullModel->mVel.mX=scale;
				this->mActorFullModel->mVel.mY=scale;
				this->mActorFullModel->mVel.mZ=scale;
			}
		};
		void mulScale(float mul){
			this->mScale.mX*=mul;
			this->mScale.mY*=mul;
			this->mScale.mZ*=mul;
			if(this->mActorFullModel != NULL){
				this->mActorFullModel->mVel.mX*=mul;
				this->mActorFullModel->mVel.mY*=mul;
				this->mActorFullModel->mVel.mZ*=mul;
			}
		};
	};
	class MapObjBase : public Game::Obj{
		public:
		_BYTE _418[0x16C];
        Lp::Utl::StateMachine mStateMachine;
	};
};