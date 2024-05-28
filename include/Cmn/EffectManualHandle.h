#pragma once

#include "types.h"
#include "Cmn/Def/Team.h"
#include "nn/vfx/System.h"

namespace Cmn {
    class EffectManualHandle {
        struct Reservation{
            _BYTE _0[0x60];
        };
        public:
            EffectManualHandle();
			void searchAndEmit(const char*, int, int);
			void setTeamColor(Cmn::Def::Team);
            void emitParticle(sead::Vector3<float> const&, void*);
            void emitParticle(sead::Matrix34<float> const&, void *);
            void createReservationInfo(short);
            void checkStopCalcAndDraw();
            u64 vftable;
            int mReservationsNum;
            Reservation *mReservations;
            int _18;
            nn::vfx::Handle *mEffectHandle;
            u64 _28;
    };
};