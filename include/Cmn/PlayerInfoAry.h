#pragma once

#include "types.h"
#include "Cmn/PlayerInfo.h"
#include "Cmn/Def/Mode.h"

namespace Cmn
{
    class PlayerInfoAry {
        public:
        int getValidInfoNum() const{
            return infos[0]->mValid + infos[1]->mValid + infos[2]->mValid + infos[3]->mValid + infos[4]->mValid + infos[5]->mValid + infos[6]->mValid + infos[7]->mValid + infos[8]->mValid + infos[9]->mValid;
        };
        void dbgSetInfoIfInvalid(Cmn::Def::Mode ,sead::SafeStringBase<char> const&);
        void resetInfos(void){
            for(int i = 1; i < 10; i++){
                this->infos[i]->mValid = 0;
            }
        };
        __int64 vtable;
        Cmn::PlayerInfo *infos[10];
    };
};