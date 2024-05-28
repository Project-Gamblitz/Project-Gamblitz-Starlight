#pragma once

#include "types.h"
#include "sead/string.h"
#include "Cmn/Def/Gear.h"
#include "Cmn/Def/CustomPartsMaterial.h"
#include "sead/calendar.h"

namespace Cmn {
    class GearData
    {
    public:
        enum class HideEar : int { None, Both, Female, Male };
        enum class HowToGet : int { cShop, cOrder, cOther, cImpossible };
        const char *HowToGetText(Cmn::GearData::HowToGet id){
            switch(id){
            case Cmn::GearData::HowToGet::cShop:
                return "cShop";
            case Cmn::GearData::HowToGet::cOrder:
                return "cOrder";
            case Cmn::GearData::HowToGet::cOther:
                return "cOther";
            case Cmn::GearData::HowToGet::cImpossible:
                return "cImpossible";
            default:
                return "Invalid";
            };
        }
        int mId;
        Cmn::Def::GearKind mGearKind;
        sead::BufferedSafeStringBase<char> mName;
        char mNameData[64];
        sead::BufferedSafeStringBase<char> mModelName;
        char mModelNameData[64];
        _BYTE idk[0x5C];
        int mPrice;
        sead::BufferedSafeStringBase<char> mBrand;
        char mBrandData[64];
        int mRarity;
        Cmn::Def::BasicSkill mSkill0;
        Cmn::Def::CustomPartsMaterial mCustomPartsMaterial;
        int mIsUnisex;
        sead::BufferedSafeStringBase<char> mAlphaMaskF;
        char mAlphaMaskFData[64];
        sead::BufferedSafeStringBase<char> mAlphaMaskM;
        char mAlphaMaskMData[64];
        Cmn::GearData::HideEar mHideEar;
        int mHarnessType;
        Cmn::GearData::HowToGet mHowToGet;
        int mAocId;
        int mIsPressRelease;
        // ?????
        float mHairArrange[129];
        float mManualBindSRTByHair[129];
        sead::CalendarTime mW3CUTCFrom;
    };
};