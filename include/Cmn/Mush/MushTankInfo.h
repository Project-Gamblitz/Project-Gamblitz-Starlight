#pragma once

#include "types.h"
#include "sead/string.h"
#include "MushInfoKindBase.h"
#include "Cmn/GearData.h"
#include "Cmn/Def/Gear.h"

namespace Cmn {
    class MushTankInfo : public MushInfoWithKindBase<Cmn::GearData,Cmn::Def::GearKind>
    {
        public:
        u64 vtable;
        MushSubHolder mMushTankHolder;
        int getTankByOrder(int id){
            return *((int*)(mMushTankHolder.getByIndex(0x118, id)));
        }  
    };
};
