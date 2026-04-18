#pragma once

#include "Utils.hpp"
#include "Cmn/PlayerWeapon.h"
#include "Cmn/KDGndCol/Manager.h"
#include "Prot/Prot.h"
#include "sead/list.h"
#include "flexlion/BulletSuperArtillery.hpp"
#define TORNADO_SPECIAL_ID 25

namespace Flexlion{
    enum TornadoState{
        cNone, cAim, cShootPrepare, cShoot,
    };
    class InkstrikeMgr{
        public:
        static InkstrikeMgr *sInstance;
        InkstrikeMgr();
        void detectChangeState(Game::Player *player);
        void registerPlayer(Game::Player *player);
        void playerFourthCalc(Game::Player *player);
        void playerFirstCalc(Game::Player *player);
        void playerThirdCalc(Game::Player *player);
        void informShotInkstrike(Game::Player *player, sead::Vector3<float> pos, sead::Vector3<float> dest, int paintgamefrm);
        void onCalc();
		bool mMatchEnding;
        Lp::Sys::ModelArc *mTornadoArc;
        Lp::Sys::ModelArc *mTornadoMonitorArc;
        TornadoState playerState[10];
        bool isAppliedWeapon[10];
        gsys::Model *mTornadoModel[10];
        gsys::Model *mTornadoMonitorModel[10];
        int mTankRootBoneIdx[10];
        int mShootPrepareFrm[10];
        int mShootFrm[10];
        sead::Vector3<float> mPendingDest[10];
        BulletSuperArtillery *bullets[10][2];
		int pickFreeSlot(int playerId);
		BulletSuperArtillery *getActiveBullet(int playerId);
		int mActiveSlot[10];
        bool mAimValid[10];
		bool mWasAHeld[10];
		bool mMapOpen[10];
        bool mWeaponHidden[10];
        bool mRemoteShotPending[10];
        u16 mDbgColAttr;
        bool mDbgColIsWall;
        bool isShot;
        float cameraanim;
        float cameraheight;
        float camerafovy;
        bool isBulletDeinit;
		float mSpawnY;  // stored once at match start, shared for all players
		bool mSpawnYCaptured;
    };
}
