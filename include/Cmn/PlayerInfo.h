#pragma once

#include "types.h"
#include "Cmn/Def/MMR.h"
#include "Cmn/Def/Gear.h"
#include "Cmn/Def/Weapon.h"
#include "Cmn/Def/Team.h"
#include "Cmn/Def/PlayerModelType.h"
#include "sead/color.h"
#include "Cmn/PlayerInfoUtil.h"

namespace Cmn
{
    class PlayerInfo {
        public:
        u32 PlayerInfo_x0;
        u32 PlayerInfo_x4;
        u32 PlayerInfo_x8;
        u32 PlayerInfo_xC;
        u32 mPlayerIndex;
        u16 mValid;
        char16_t mPlayerName[17];
        Cmn::Def::Team mTeam;
        Cmn::Def::PlayerModelType mModelType;
        u32 mModelId;
        u32 mSkinColorId;
        u32 mEyeColorId;
        Cmn::Def::Weapon mWeapon;
        u32 PlayerInfo_x5C;
        u32 PlayerInfo_x60;
        Cmn::Def::Gear mShoeGear;
        Cmn::Def::Gear mClothGear;
        Cmn::Def::Gear mHeadGear;
        u32 mTankId;
        u32 mBottomId;
        sead::Color4f mColor; //Only used for the plaza NPCs ?
        u32 mPlayerRank;
        u32 mStarRank;
        u32 mUdemaeGrades[5];
        u32 PlayerInfo_xF8;
        u32 PlayerInfo_xFC;
        Cmn::Def::MMR mMMR1;
        Cmn::Def::MMR mMMR2;
        char gap_x120[0x80];

        void setPlayerRank(int);
        void dbgSetPlayerName(sead::SafeStringBase<char> const&);
        void setPlayerName(char16_t const*);
		void dbgSetRandomValue(int);

        enum setInfoType{
            cNone, cRandom, cSaveData, cAgent3
        };

        void setInfoByPlayerType(int PlayerType, Cmn::PlayerInfo::setInfoType setType, Cmn::Def::Mode mode){
            switch(setType){
            case Cmn::PlayerInfo::setInfoType::cRandom:
                this->dbgSetRandomValue(NULL);
                break;
            case Cmn::PlayerInfo::setInfoType::cSaveData:
                Cmn::PlayerInfoUtil::setPlayerInfoBySaveData(this, mode);
                break;
            case Cmn::PlayerInfo::setInfoType::cAgent3:
                Cmn::PlayerInfoUtil::setPlayerInfoBySaveData(this, mode);
                Cmn::PlayerInfoUtil::setPlayerInfoAgentThree(this, this->mTeam);
                break;
            };
            if(PlayerType == 0 or PlayerType == 2 or PlayerType == 4 or PlayerType > 5){
                mBottomId = 1;
            } else{
                mBottomId = 5;
            }
            this->mModelType = (Cmn::Def::PlayerModelType)PlayerType;
            switch((int)mModelType){
            case 0:
                this->mModelId = 0;
                break;
            case 1:
                this->mModelId = 2;
                break;
            case 2:
                this->mModelId = 50;
                break;
            case 3:
                this->mModelId = 51;
                break;
            case 4:
                this->mModelId = 110;
                break;
            case 5:
                this->mModelId = 111;
                break;
            case 6:
                this->mModelId = 100;
                break;
            case 7:
                this->mModelId = 102;
                break;
            };
        }
    };
};
