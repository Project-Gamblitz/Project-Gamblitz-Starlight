#pragma once

#include "types.h"

namespace Game {
    class FilmingSupporter {
        public:
        FilmingSupporter();
        static void clearPaint();
        static void applyHideLayout(bool);
        void setVisiblePlayer_(bool);
        void setVisibleWithoutPlayer_(bool);
        static void applyHideEffect(bool);
        static void setClearBG_(bool);
        _BYTE _0[0x120];
    };
};