#pragma once

#include "types.h"
#include "MushWeaponInfo.h"
#include "MushMapInfo.h"
#include "MushHairInfo.h"
#include "MushBottomInfo.h"
#include "MushGearInfo.h"
#include "MushTankInfo.h"
#include "sead/heap.h"

namespace Cmn {
    class MushDataHolder {
        public:
        static Cmn::MushDataHolder* sInstance;

        sead::IDisposer mDisposer;
        sead::Heap *mHeap;
        Cmn::MushMapInfo* mMushMapInfo;
        Cmn::MushHairInfo *mMushHairInfo;// Cmn::MushHairInfo *
        Cmn::MushGearInfo *mMushGearInfo;// Cmn::MushGearInfo *
        Cmn::MushTankInfo *mMushTankInfo;// Cmn::MushTankInfo *
        Cmn::MushBottomInfo *mMushBottomInfo;// Cmn::MushBottomInfo *
        Cmn::MushWeaponInfo *mMushWeaponInfo;
    };
}