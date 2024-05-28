#pragma once

#include "types.h"

#include "Cmn/Actor.h"
#include "Game/Obj.h"
#include "Game/ActorMgr.h"

namespace Game {
    class MissilePosition : public Game::MapObjBase{
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
    class Lift : public Game::MapObjBase{
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
    };
    class PaintTargetArea : public Game::MapObjBase{
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        _BYTE _594[0x628 - sizeof(Game::MapObjBase)];
        sead::BoundBox3<float> *bounds;
    };
    class MapObjMgr : public Game::ActorMgr{
        public:
        static Game::MapObjMgr* sInstance;
    };
};