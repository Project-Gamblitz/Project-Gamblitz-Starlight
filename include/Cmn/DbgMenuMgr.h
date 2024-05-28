#pragma once

#include "types.h"
#include "sead/heap.h"
#include "agl/lyr/renderinfo.h"

namespace Cmn {
    class DbgMenuMgr{
        public:
        static Cmn::DbgMenuMgr *sInstance;
        void setEnable(bool);
        static void createInstance(sead::Heap *);
        void calc();
        void draw(agl::lyr::RenderInfo const&) const;
    };
};