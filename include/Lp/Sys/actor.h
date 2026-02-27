#include "types.h"

#include "sead/tree.h"
#include "sead/string.h"
#include "Lp/Sys/ActorClassIterNodeBase.h"
#include "sead/critical.h"
#include "worker.h"

#pragma once

namespace Lp {
  namespace Sys {
      class ActorSystem{
        public:
        static Lp::Sys::ActorSystem *sInstance;
        _BYTE _0[0x188];
        sead::CriticalSection *criticalSession;
      };
      class Actor{
        public:
          Actor();
          ~Actor();
          
		  template<typename T>
		  static T* create(Lp::Sys::Actor*, sead::Heap*);
      template<typename T>
      static T* createCstm(Lp::Sys::Actor*parent){
        Lp::Sys::ActorSystem *system = Lp::Sys::ActorSystem::sInstance;
        system->criticalSession->lock();
        T *actor = new T();
        actor->mHeap = sead::HeapMgr::sInstance->getCurrentHeap();
        ((Lp::Sys::Actor*)actor)->actorSysOnCreate(parent);
        system->criticalSession->unlock();
        return actor;
      };
      enum ListNodeKind{
        None
      };
      void reserveActivate(bool);
      void reserveSleepAll_(Lp::Sys::Actor::ListNodeKind);
      void reserveRemoveAll();
      void actorSysCalc();
      void actorSysOnCreate(Lp::Sys::Actor*);

          u64 vtable;
          u64 LpActor_x8;
          sead::TreeNode mTreeNodes[2];
          char LpActor_x60[0xB8];
          u64 LpActor_x118;
          u64 LpActor_x120;
          u64 LpActor_x128;
          u64 LpActor_x130;
          u64 LpActor_x138;
          u64 LpActor_x140;
          u64 LpActor_x148;
          u64 LpActor_x150[16];
          char LpActor_x1D0[128];
          sead::Heap *mHeap;
          u32 LpActor_x258;
          u32 LpActor_x25C;
          u64 mSystemTick; // _260
          _QWORD LpActor_x268;
          sead::BufferedSafeStringBase<char> mLpActorStr;
          char LpActor_x288[64];
          u64 LpActor_x2C8;
          u64 LpActor_x2D0;
          u64 LpActor_x2D8;
          u64 LpActor_x2E0;
      };

	  static void setMap_(sead::BufferedSafeStringBase<char> *);
  };
};