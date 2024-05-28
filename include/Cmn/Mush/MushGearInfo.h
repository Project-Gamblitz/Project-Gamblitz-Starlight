#pragma once

#include "types.h"
#include "sead/string.h"
#include "MushInfoKindBase.h"
#include "Cmn/GearData.h"
#include "Cmn/Def/Gear.h"

namespace Cmn {
    class MushGearInfo : public MushInfoWithKindBase<Cmn::GearData,Cmn::Def::GearKind>
    {
        public:
        Cmn::GearData* searchByName(Cmn::Def::GearKind, sead::SafeStringBase<char> const&) const; 
        int searchIdArmor(Cmn::Def::GearKind,int) const;
        int searchIdOctaArmor(Cmn::Def::GearKind,int) const;
        u64 vtable;
        MushSubHolder mMushGearHolders[3];
        int getGearIdByOrder(int kind, int id){
            if(kind > 2){
                return 0;
            }
            return *((int*)mMushGearHolders[kind].getByIndex(0x1F30, id));
        };  
    };
};
