#pragma once

#include "types.h"
#include "sead/string.h"
#include "MushInfoKindBase.h"
#include "Cmn/WeaponData.h"
#include "Cmn/Def/WeaponKind.h"

namespace Cmn {

    class MushWeaponInfo : public MushInfoWithKindBase<Cmn::WeaponData,Cmn::Def::WeaponKind>
    {
        public:
        Cmn::WeaponData* searchByName(Cmn::Def::WeaponKind, sead::SafeStringBase<char> const&) const; 
        u64 vtable;
        MushSubHolder mWeaponHolders[3];
        int getWeaponByOrder(int kind, int id){
            if(kind > 2){
                return 0;
            }
            return *((int*)mWeaponHolders[kind].getByIndex(0x398, id));
        }  
        Cmn::WeaponData *getDupWeaponById(int kind, int id){
            if(kind > 2){
                return 0;
            }
            Cmn::WeaponData *data = this->getById((Cmn::Def::WeaponKind)kind, id);
            if(data == NULL){
                return NULL;
            }
            for(int i = 0; i < mWeaponHolders[kind].mEntryNum; i++){
                Cmn::WeaponData *checkData = (Cmn::WeaponData *)(mWeaponHolders[kind].getByIndex(0x398, i));
                if(checkData->mId != data->mId and strcmp(checkData->mName.mCharPtr, data->mName.mCharPtr) == 0){
                    return checkData;
                }
            }
            return NULL;
        }  
    };
};
