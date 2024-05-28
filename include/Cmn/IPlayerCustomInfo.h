#pragma once

#include "types.h"
namespace Game{
    class Player;
};
namespace Cmn {
    class IPlayerCustomInfo;
    using gpFunc = Game::Player *(*)(Cmn::IPlayerCustomInfo*);
    class IPlayerCustomInfoVtable{
        public:
        u64 isUseStandAlone;
        u64 getCustomMgr;
        u64 isCreateWeapon;
        u64 getMainWeaponInfo;
        u64 isCreateTank;
        u64 isCreateAllTank;
        u64 getCreateTankNum;
        u64 getCreateTankDataId;
        u64 isCreateArmomr;
        u64 isCreateAllBottom;
        u64 getCreateBottomDataId;
        u64 isCreateMantle;
        u64 getGearCreateActorNum;
        u64 getGearInfo;
        u64 getModelType;
        u64 getTeam;
        u64 getXluZPrePassID_Offset;
        u64 getFestTeam;
        u64 getFestId;
        u64 getHumanBoneAccessor;
        u64 isUseLODModel;
        u64 addToOcclusionQuery;
        u64 getModelScene;
        u64 getHairId;
        gpFunc getGamePlayer;
        u64 getCustomizePlayer;
        u64 getShopPlayerDressup;
        u64 getPlayerViewerPlayer;
        u64 getStaffRollOctaPlayer;
    };
	class IPlayerCustomInfo{
		public:
        IPlayerCustomInfo(){
            vtable = NULL;
        };
		Cmn::IPlayerCustomInfoVtable *vtable;
        u64 *mPlayerPtr;
        _BYTE _10[0x20];
	};
};