#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/CameraMgr.h"


namespace Cmn {
    class CameraAccessor{
        public:
        static Cmn::CameraAccessor *sInstance;
        Cmn::CameraMgr *getCameraMgr(Cmn::CameraMgr::LayerId);
    };
};