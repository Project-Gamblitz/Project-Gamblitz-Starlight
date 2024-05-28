#pragma once

#include "types.h"
#include "sead/font.h"
#include "sead/string.h"

namespace Lp
{
    namespace UI {
        class FontMgr{
            public:
            sead::FontBase *getFont(sead::SafeStringBase<char> const&);
        };
        class UIMgr{
          public:
          static Lp::UI::UIMgr *sInstance;
          _BYTE _0[0x400];
          Lp::UI::FontMgr *fontMgr;
        };
    };
};
