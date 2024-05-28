#pragma once

#include "types.h"

namespace sead{
    class WorkerMgr;
};
namespace gsys {
    class ParticleMgr {
        public:
        static gsys::ParticleMgr* sInstance;
        void calcStep2(sead::WorkerMgr *);
        void calcStep3(sead::WorkerMgr *);
        void calcStep4(sead::WorkerMgr *);
        void calcStep5(sead::WorkerMgr *);
    };
};