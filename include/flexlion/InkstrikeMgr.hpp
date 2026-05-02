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
        // Hard-reset every player + every BSA bullet — used when shooting-
        // range cleanup (Game::SeqMgrShootingRange::stateEnterInkReset)
        // fires. Cancels all bullets, clears all per-player state, kills
        // any lingering xlinks, releases the tank-override lock.
        void resetForCleanup();
        void tryCaptureSpawnY();
		void snapshotCamUp(sead::Vector3<float> camAt);
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
        float mDbgColY;  // Y coord where the validated hit was found
        bool mDbgColIsObject;  // hit was on a registered Obj_ block, not the main Fld_
        // Diagnostics for object-detection wiring
        u32 mDbgMainBlkLo;   // low 32 bits of mainBlock pointer (0 = mgr/mainBlock missing)
        u32 mDbgFloorBlkLo;  // low 32 bits of floor block pointer at +240
        u32 mDbgWallBlkLo;   // low 32 bits of wall block pointer at +248
        const char *mDbgColReason;  // last validation reason (ACCEPT_FLOOR, BAD_BITS, NO_GEOM, etc.)
        bool isShot;
        float cameraanim;
        float cameraheight;
        float camerafovy;
        bool isBulletDeinit;
		float mSpawnY;  // stored once at match start, shared for all players
		bool mSpawnYCaptured;float mCamUpX;   // X component of camera up vector (-1, 0, or +1)
		float mCamUpZ;   // Z component of camera up vector (-1, 0, or +1)
		// Exactly one is nonzero; together they form an axis-aligned mUp
		bool mNeedCamUpSnapshot;
    };
}
