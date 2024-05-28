#pragma once

#include "types.h"
#include "Cmn/Def/PlayerModelType.h"

namespace Cmn
{
    class MushHairInfo
    {
    public:
        int searchIdDefault(Cmn::Def::PlayerModelType)const;
        u64 vtable;
        MushSubHolder mHairHolders[7];
        int getHairByOrder(int playertype, int id){
            if(playertype > 7){
                return 0;
            }
            return *((int*)mHairHolders[playertype].getByIndex(0x118, id));
        }  
    };
};
