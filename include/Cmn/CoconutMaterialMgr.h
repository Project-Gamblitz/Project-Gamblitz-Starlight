#pragma once

#include "sead/string.h"
#include "types.h"
#include "agl/DrawContext.h"

namespace Cmn {
    class CoconutMaterialMgr{
        public:
        void draw(agl::DrawContext *);
        static void deleteInstance();
    };
};