#pragma once

#include "types.h"

namespace Match{
    class UIVersusPageHandler{
        public:
        int getDecideBtnPrivateMain() const;
        void forceDecideStandbyBtnPrivateMain();
        void lockDecideStandbyBtnPrivateMain();
        bool isEndDecidePrivateMain() const;
    };
};