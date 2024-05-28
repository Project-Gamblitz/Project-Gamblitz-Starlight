#pragma once

#include "types.h"
#include "Cmn/Actor.h"
#include "Game/BulletBombQuick.h"
#include "Game/BulletSuperLaser.h"
#include "Game/BulletSpSuperBall.h"
#include "Game/BulletRollerCore.h"

namespace Game {
    class BulletSpSuperMissile : public Cmn::Actor{
        public:
        _BYTE _348[0x18];
        Cmn::Actor *mSender;
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
    class Bullet: public Cmn::Actor{
        public:
        struct InitializeArg{
            Cmn::Def::Team mTeam;
            int _4;
            _BYTE _8[0x10];
            Cmn::Actor *mSender;
            int mWeaponId;
        };
        void initialize(Game::Bullet::InitializeArg const&);
        _BYTE _348[0x18];
        Cmn::Actor *mSender;
        int mWeaponId;
    };
    class BulletSimple : public Bullet{
        public:
        struct SimpleInitializeArg{
            _BYTE _0[0x30];
        };
        void initialize(Game::Bullet::InitializeArg const&, Game::BulletSimple::SimpleInitializeArg const&);
    };
    class BulletSpAquaBall : public Cmn::Actor{
        public:
        _BYTE _348[0x18];
        Cmn::Actor *mSender;
        _BYTE _368[0x384 - 0x368];
        sead::Vector3<float> mPosition;
        void setPos(sead::Vector3<float> const&);
        void initializeSender(Cmn::Actor const*, int, int, sead::Vector3<float> const&);
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
    class BulletGachihoko : public Cmn::Actor{
        public:
        _BYTE _348[0x18];
        Cmn::Actor *mSender;
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
    class BulletGachihokoSplash{
        public:
    };
    class BulletSlosherBase{
        public:
    };
    class BulletSlosherSplash{
        public:
    };
	class BulletMgr {
		public :
        static Game::BulletMgr *sInstance;
        Lp::Sys::Actor* getBulletParent(){
            auto parent = *((Lp::Sys::Actor* (*)[1000])this->mBulletPool);
            int coolnum = this->_470;
            if(this->_470 > this->_474 + 12){
                coolnum = 0;
            }
            return parent[coolnum];
        }
        Cmn::Actor *activateOneCancelUnnecessary(int, bool, sead::Vector3<float> const&, sead::Vector3<float> const&, int);
		_BYTE _0[0x468];
        Lp::Sys::Actor *mBulletPool;
        int _470;
        int _474;
        int mBulletCount;
        int mBulletPoolStart;
	};
};