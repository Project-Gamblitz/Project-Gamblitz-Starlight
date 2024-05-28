#pragma once

#include "Cmn/GfxUtl.h"

namespace Cmn{
    class PlayerCustomHumanBoneAccessor{
        public:
        PlayerCustomHumanBoneAccessor(gsys::Model *model);
        gsys::Model *mModel;
        _BYTE _8[0x244]; // 2a8
        int mWeaponBone;
        _BYTE _250[0x2B0 - 0x250];
    };
};