#pragma once

#include "types.h"
#include "sead/vector.h"

namespace Game{
    class Player;
    class PlayerStateCloneEvent{
        public:
        _BYTE _data[40]; // byte[32] is the packet value, 40 bytes total (matches game's actual struct size)
    };
    class PlayerCloneObj{
        public:
        bool pushPlayerStateEvent(const Game::PlayerStateCloneEvent &event);
    };
    class CloneObjMgr{
        public:
        _BYTE _0[0x400];
        bool mIsOfflineScene;
        int mSenderPlayerIdx;
    };
    class BulletCloneEvent{
        public:
        enum Type{
            BulletTypeInkstrike = 113
        };
        Type mType;
        sead::Vector3<float> mPos;
        sead::Vector3<float> mVel;
        int mPlayerId;
        ushort _unk;
    };
    class BulletCloneHandle{
        public:
        void sendEvent_Shot(int,sead::Vector3<float> const&,sead::Vector3<float> const&,Game::BulletCloneEvent::Type,ushort);
        u64 *vtable;
        CloneObjMgr *mCloneObjMgr;
        Game::PlayerCloneObj *mPlayerCloneObj;
    };
    class PaintCloneHandle{
        public:
        u64 *vtable;
        CloneObjMgr *mCloneObjMgr;
        Game::PlayerCloneObj *mPlayerCloneObj;
    };
    class PlayerCloneHandle{
        public:
        u64 *vtable;
        CloneObjMgr *mCloneObjMgr;
        Game::PlayerCloneObj *mPlayerCloneObj;
        BulletCloneHandle *mBulletCloneHandle;
        PaintCloneHandle *mPaintCloneHandle;
    };
    class PlayerNetControl{
        public:
        static void (*calcReceive)(Game::PlayerNetControl*);
        void sendSignal_AllMarking();
        Game::Player *mPlayer;
        Game::PlayerCloneHandle *mCloneHandle;
    };
};