#pragma once

#include "types.h"
#include "sead/color.h"
#include "sead/string.h"
#include "sead/array.h"
#include "Cmn/Def/Team.h"
#include "Cmn/TeamColorSet.h"
#include "Cmn/TeamColorParam.h"

namespace Cmn {
    class TeamColorMgr {
        public:
        enum ColorVariation {
            COLOR_0,
            COLOR_1,
            COLOR_2,
            COLOR_3,
            COLOR_4,
            COLOR_5,
            COLOR_6,
            COLOR_7,
            COLOR_8,
            COLOR_9,
            COLOR_A,
            COLOR_B,
        };

        enum Kind {
            TESTCOOL
        };

        /*struct TeamColorParamArry {
            u64 length;
            Cmn::TeamColorParam params[0x20];
        };*/

        struct TeamColorParamEntry
        {
            #pragma pack(push, 1)
            Cmn::TeamColorParam *mParamLookup;
            u32 mStart;
            s32 mBeginningOfLookup;
            s32 mLengthOfLookup;
            u32 unk;
            #pragma pack(pop)
        };

    
        char somethin[0x10];
        TeamColorParamEntry entries[10];
        _BYTE gap100[24];
        float gap118[12];
        _QWORD qword148;
        _QWORD qword150;
        _QWORD dword158;
        _QWORD qword160;
        _QWORD qword168;
        _QWORD dword170;
        _BYTE _178[0x18];
        Cmn::TeamColorSet teamColorSet;


        //static void calcSubColorSet(sead::SafeArray<sead::SafeArray<sead::Color4f, 12>, 4>&, Cmn::TeamColorSet const&);
		void updateSubColorSet_();
        void updateInkColor(float);
        sead::Color4f* getCurColor(Cmn::Def::Team, Cmn::TeamColorMgr::ColorVariation) const;
        void getNextVersus(Cmn::Def::VersusMode, Cmn::Def::VersusRule, bool, Cmn::TeamColorParam **)const;
        void getNextCoop(Cmn::TeamColorParam **)const;
        int getColorNumForKind(int kind){
            if(kind > 11){
                return 0;
            }
            return *(int*)(((u64)this) + 0x20 + 0x18 * kind);
        }   
        char *getColorNameForKindId(int kind, int id){
            if(kind > 11){
                return 0;
            }
            u32 a1 = (*(u32*)(((u64)this) + 0x18 + 0x18 * kind));
            u32 a2 = (*(u32*)(((u64)this) + 0x1C + 0x18 * kind)) + id;
            if(a2 < a1){
                a1 = 0;
            }
            u64 a3 = (*(u64*)(((u64)this) + 0x10 + 0x18 * kind));
            return *(char**)(a3 + (0x1C0 * (a2 - a1)) + 0x170);
        }   
        void getColorSet(Cmn::TeamColorMgr::Kind,int) const;
    };
};