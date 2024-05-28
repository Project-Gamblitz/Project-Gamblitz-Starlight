#pragma once

#include "types.h"
#include "flexlion/ProcessMemory.hpp"
#include "sead/string.h"
#include "sead/heap.h"
#include "sead/runtime.h"
#include "agl/lyr/renderinfo.h"
#include "Cmn/GfxMgr.h"
#include "Cmn/PBRMgr.h"
#include "gsys/layer.h"
#include "Lp/Utl.h"
#include "Cmn/Def/ResetType.h"
#include "starlight/input.hpp"
#include "gsys/model.h"
#include "Lp/Sys/modelarc.h"
#include "lp/Utl.h"
#include "sead/camera.h"
#include "Lp/Sys/SceneMgr.h"
#include "Cmn/Actor.h"
namespace Lp{
  namespace Sys{
    class ISceneInfoCstm{
      public:
      static Lp::Sys::ISceneInfoCstm *spCstm;
    };
    class Scene{
      public:
      Scene(sead::Heap* a);
      u64 checkDerivedRuntimeTypeInfo(sead::RuntimeTypeInfo::Interface const*) const;
      u64 getRuntimeTypeInfo() const;
      void sceneLoad();
      void sceneEnter();
      void sceneReset();
      void sceneCalc();
      void sceneDraw2D(agl::lyr::RenderInfo const&) const;
      void sceneDraw3D(agl::lyr::RenderInfo const&) const;
      void sceneExit();
      void sceneDestroy();
      void dbgShortReset();
      void dbgShortExit();
      //u64 vtable;
      _BYTE _8[0x230];
    };
  };
};
namespace Cmn{
  class SceneBaseVtable{
    public:
    u64 checkDerivedRuntimeTypeInfo;
    u64 getRuntimeTypeInfo;
    u64 sceneLoad;
    u64 sceneEnter;
    u64 sceneReset;
    u64 sceneCalc;
    u64 sceneDraw2D;
    u64 sceneDraw3D;
    u64 sceneExit;
    u64 sceneDestroy;
    u64 dbgShortReset;
    u64 dbgShortExit;
    u64 load;
    u64 enter;
    u64 preActorCalc;
    u64 exit;
    u64 destroy;
    u64 postDestroy;
    u64 draw2D;
    u64 postDraw2D;
    u64 draw3D;
    u64 postDraw3D;
    u64 cbResetShort;
    u64 cbResetLong;
    u64 cbExitShort;
    u64 cbExitLong;
    u64 createSceneSound;
    u64 prepareReset;
    u64 reset;
    u64 scenedtor2;
    u64 scenedtor0;
  };
  class SceneBase : public Lp::Sys::Scene{
    public:
    SceneBase(sead::Heap* a);
    u64 checkDerivedRuntimeTypeInfo(sead::RuntimeTypeInfo::Interface const*) const;
    u64 getRuntimeTypeInfo() const;
    void sceneLoad();
    void sceneEnter();
    void sceneReset();
    void sceneCalc();
    void sceneDraw2D(agl::lyr::RenderInfo const&) const;
    void sceneDraw3D(agl::lyr::RenderInfo const&) const;
    void sceneExit();
    void sceneDestroy();
    void load();
    void enter();
    void preActorCalc();
    void exit();
    void destroy();
    void postDestroy();
    void draw2D(agl::lyr::RenderInfo const&) const;
    void postDraw2D(agl::lyr::RenderInfo const&) const;
    void draw3D(agl::lyr::RenderInfo const&) const;
    void postDraw3D(agl::lyr::RenderInfo const&) const;
    void cbResetShort();
    void cbResetLong();
    void cbExitShort();
    void cbExitLong();
    void createSceneSound();
    void prepareReset(Cmn::Def::ResetType);
    void reset(Cmn::Def::ResetType);
    ~SceneBase();
    _BYTE _238[0xB8];
    Cmn::GfxMgr *mGfxMgr;
    _BYTE _2F8[0x80];
  };
};