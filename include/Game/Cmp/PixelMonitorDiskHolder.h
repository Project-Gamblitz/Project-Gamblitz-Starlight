#pragma once

#include "types.h"

namespace Game {
    namespace Cmp {
        class PixelMonitorDiskHolder : public Cmn::ComponentBase {
            public:
            PixelMonitorDiskHolder(Cmn::ComponentHolder *);
            _BYTE _28[0x38 - 0x28];
            sead::Vector3<float> *mKdPos; // 0x38
            _BYTE _40[0x8];
            float *mMonitorHalfSize; // 0x48
        };
    }
}