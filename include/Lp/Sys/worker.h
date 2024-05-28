#pragma once

#include "types.h"
#include "sead/task.h"
#include "sead/delegate.h"

namespace Lp{
    namespace Sys{
        class WorkerJobBase{
            public:
            WorkerJobBase(){
                mDelegate = sead::Delegate<sead::Task>();
                mIsCompleted = 0;
                _0xC = 0;
            }
            virtual void execute(){
                this->execute_();
                mIsCompleted = 1;
            }
            virtual void execute_();/*{
                if ( mDelegate._8 ) {
                    u64 v2 = mDelegate._10;
                    if ( mDelegate._18 & 1LL | v2 ){
                        u64 *v4 = (u64 *)(mDelegate._8 + (mDelegate._18 >> 1));
                        if ( mDelegate._18 & 1 )
                            v2 = *(u64 *)((u64)v4 + v2);
                        (u64 *)((u64 (*)(u64 *))v2)(v4);
                    }
                }
            }*/
            u32 mIsCompleted;
            u32 _0xC;
            sead::Delegate<sead::Task> mDelegate;
        };
    };
};