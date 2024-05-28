#include "flexlion/Sp1WeaponLookup.hpp"

namespace Starlion{
    namespace Sp1WeaponLookup{
        void Initialize(){
            memset(sp2WeapIds, -1, sizeof(sp2WeapIds));
            Cmn::MushDataHolder *mushData = Cmn::MushDataHolder::sInstance;
            Cmn::MushWeaponInfo *weaponInfo = mushData->mMushWeaponInfo;
            for(int i = 0; i < lookupWeapNum; i++){
                Cmn::WeaponData *data = weaponInfo->getDupWeaponById(int(Cmn::Def::WeaponKind::cMain), lookupWeapIds[i]);
                if(data == NULL){
                    continue;
                }
                sp2WeapIds[i] = data->mId;
            }
        }
        void getLookupRefId(short *res, int playerWeaponId, int weaponid){
            if(playerWeaponId != weaponid){
                for(int i = 0; i < lookupWeapNum; i++){
                    if(lookupWeapIds[i] == playerWeaponId and sp2WeapIds[i] == weaponid){
                        *res = lookupWeapIds[i];
                        return;
                    }
                }
            }
        }
    };
};