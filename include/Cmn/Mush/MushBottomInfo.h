#pragma once

#include "types.h"
#include "Cmn/Def/PlayerModelType.h"
#include "Cmn/Def/Sex.h"
#include "MushInfoKindBase.h"

namespace Cmn
{
    class BottomData{
        public:
        int id;
    };
    class MushBottomInfo
    {
    public:
        int searchIdDefault(Cmn::Def::Sex)const;
        u64 vtable;
        MushSubHolder mBottomHolders[2];
        int getBottomById(int sex, int id){
            if(sex > 1){
                return 0;
            }
            return *((int*)mBottomHolders[sex].getByIndex(0x178, id));
        }   
    };
};
