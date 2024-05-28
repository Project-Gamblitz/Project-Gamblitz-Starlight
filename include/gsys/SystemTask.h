#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Lp/Sys/ctrl.h"
#include "agl/DrawContext.h"

namespace gsys {
    class SystemTask {
        public:
        static gsys::SystemTask* sInstance;
        void invokeCalcFrame_();
        void postCalc_();
        void drawTV_();
        void invokeDrawTV_(agl::DrawContext*);
        _BYTE _0[0x550];
        u32 mIsHideEff;
        u32 _554;
        int mEnabledDrawOptions;
    };
};