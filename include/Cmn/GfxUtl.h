#pragma once

#include "types.h"

#include "sead/color.h"
#include "gsys/model.h"
#include "Cmn/Def/Team.h"
#include "sead/string.h"
#include "lp/Sys/modelarc.h"
#include "gsys/model.h"
#include "sead/heap.h"
#include "agl/TextureData.h"

namespace Cmn {
    class GfxMgr;
    class GfxUtl {
        public:
        static gsys::Model *createModel(Cmn::Def::Team,sead::SafeStringBase<char> const&,Lp::Sys::ModelArc &,Lp::Utl::ModelCreateArg const&,sead::Heap *);
        static gsys::Model *createModel(Cmn::Def::Team,sead::SafeStringBase<char> const&,Lp::Sys::ModelArc &,char const**,unsigned int,Lp::Utl::ModelCreateArg const&,sead::Heap *);
        static gsys::Model *createModelAndAnim(Cmn::Def::Team,sead::SafeStringBase<char> const&,Lp::Sys::ModelArc &,Lp::Sys::ModelArc &,Lp::Utl::ModelCreateArg const&,Lp::Utl::AnimCreateArg const&,bool,sead::Heap *);
        static u64 getLyrIdx_3D_MissionMonitor();
        static int getLyrIdx_3D_Main();
        static int getLyrIdx_2D_Main();
        static int getLyrIdx_2D_LytTex();
        static void replaceTeamColor(gsys::Model *, sead::Color4f const&);
        static void setTwoColorCompTeam(gsys::Model *, Cmn::Def::Team);
        static void replaceAlphaBravoColorForCalcColor_(gsys::Model*, sead::Color4f const&, sead::Color4f const&);
        static int getInkType();
        static Cmn::GfxMgr *getGfxMgr();
        static void setDepthSilhouetteColor(gsys::Model *,sead::Color4f const&);
        static void setXluZPrePassVisibleZero(gsys::Model *);
        static void checkAndEnablePlanerReflection(gsys::ModelScene *,gsys::Model *);
        static void disableBakeLight(gsys::Model *);
        static void createTexture(Lp::Sys::ModelArc *, const char *, agl::TextureData *);
    };
};