#pragma once

#include "types.h"
#include "Lp/Sys/actor.h"
#include "Cmn/Actor.h"
#include "Lp/Utl.h"
#include "sead/random.h"
#include "sead/array.h"
#include "Cmn/Def/ResetType.h"

namespace Game
{
    namespace Coop
    {
        class EnemyDirector : public Cmn::Actor, sead::IDisposer
        {
        public:
            void reset(Cmn::Def::ResetType);
            sead::PtrArrayImpl mEnemyArray1;
            char char378;
            _BYTE gap379[183];
            sead::PtrArrayImpl mEnemyArray2;
            char char440;
            _BYTE gap441[183];
            sead::PtrArrayImpl mEnemyArray3;
            char char508;
            _BYTE gap509[183];
            _QWORD qword5C0;
            _DWORD dword5C8;
            sead::Random mRandom[2];
            _BYTE gap5EC[4];
            _QWORD qword5F0;
            _QWORD qword5F8;
            _QWORD qword600;
            _QWORD qword608;
            _QWORD qword610;
            _QWORD qword618;
            _DWORD dword620;
            _BYTE gap624[4];
            _QWORD qword628;
            _DWORD dword630;
            _BYTE gap634[96];
            _DWORD mActiveEnemyMax1;
            _DWORD mActiveEnemyMax2;
            _DWORD mActiveEnemyNum;
            _BYTE gap694[240];
            _DWORD dword76C;
            _QWORD qword770;
            _QWORD qword778;
            _QWORD qword780;
            _DWORD dword788;
        };
    }; // namespace Coop
};     // namespace Game