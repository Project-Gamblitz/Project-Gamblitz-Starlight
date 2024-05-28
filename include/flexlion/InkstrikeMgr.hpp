#pragma once

#include "Utils.hpp"
#include "Cmn/PlayerWeapon.h"
#include "Prot/Prot.h"
#include "sead/list.h"
#define TORNADO_SPECIAL_ID 25

namespace Flexlion{
    enum TornadoState{
        cNone, cAim, cShoot,
    };
    class BulletTornado{
        public:
        BulletTornado();
        void onActivate(Game::Player *Sender, gsys::Model *Model, sead::Vector3<float> src, sead::Vector3<float> dst, int paintgamefrm);
        void onCalc();
        void calcBurst();
        void burst();
        void onRender();
        void reset();
        gsys::Model *model;
        Game::Player *sender;
        Game::BulletSpSuperBall *superball;
        sead::Vector3<float> from;
        sead::Vector3<float> to;
        sead::Vector3<float> rot;
        sead::Vector3<float> pos;
        int startfrm;
        bool isactive;
        bool isShot;
    };
    class InkstrikeMgr{
        public:
        static InkstrikeMgr *sInstance;
        InkstrikeMgr();
        void detectChangeState(Game::Player *player);
        void registerPlayer(Game::Player *player);
        void playerFourthCalc(Game::Player *player);
        void playerFirstCalc(Game::Player *player);
        void informShotInkstrike(Game::Player *player, sead::Vector3<float> pos, sead::Vector3<float> dest, int paintgamefrm);
        void onCalc();
        Lp::Sys::ModelArc *mTornadoArc;
        TornadoState playerState[10];
        bool isAppliedWeapon[10];
        gsys::Model *mTornadoModel[10];
        BulletTornado *bullets[10];
        float bulletanim[10];
        bool isShot;
        float cameraanim;
        float cameraheight;
        bool isBulletDeinit;
    };
}