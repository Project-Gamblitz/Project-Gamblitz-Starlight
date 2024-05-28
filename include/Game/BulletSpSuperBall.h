#pragma once
#include "Cmn/Actor.h"
#include "Cmn/KDUtl/MtxT.h"
#include "Game/BulletBombCmnCore.h"

namespace Game{
    struct SuperBallShotArg1{
        _BYTE _0[0x30];
    };
    struct SuperBallShotArg2{
        _BYTE _0[0x10];
    };
    class BulletSpSuperBall : public Cmn::Actor{
        public:
        void Initialize(int, sead::Vector3<float>*);
        void shot(Game::Player *sender, int senderId, int senderId2, sead::Vector3<float> *startpos, SuperBallShotArg2 *arg2, int poop);
        void firstCalc();
        void firstCalcSub();
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        _BYTE _348[0x18];
        Cmn::Actor *mSender;
        _BYTE _368[0x388 - 0x368]; 
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
        _BYTE _3B8[0x3D8 - 0x3B8];
        sead::Vector3<float> mVel;
        _BYTE _3E4[0x55F-0x3E4];
        char _55F;
        _BYTE _560[0x588 - 0x560];
        sead::Matrix34<float> mMatrix;
        _BYTE _5B8[0x5E8 - 0x5b8];
        Game::BulletBombCmnCore *mCore;
        _BYTE _5D0[0x600-0x5D0];
        bool mIsHitGnd;
        _BYTE _601[0x610 - 0x601];
        u64 _610;
        Cmn::KDUtl::MtxT mtx_618;
    };
};