#pragma once

#include "types.h"

namespace Cmn{
    class StaticParamsHolder{
        public:
        static StaticParamsHolder *sInstance;
        void createPlayerCustomHairArrangeParamIfNotCreatedYet();
    };
};