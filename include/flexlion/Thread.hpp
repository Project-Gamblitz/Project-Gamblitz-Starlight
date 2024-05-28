#pragma once

#include "types.h"
#include "mem.h"
#include "memory.h"
#include "nn/os.h"
#include "vector"
#include "operator.h"

using voidFunc = void (*)();
using voidFuncArg = void (*)(void *);

namespace Flexlion{
    enum ThreadType{
        cDefault, cOnce, cManualSync
    };
    class Thread{
        public:
        Thread(voidFunc function, Flexlion::ThreadType type, int cps);
        ~Thread();
        void calc();
        void start();
        void stop();
        voidFunc mFunction;
        bool mInit;
        bool mRunning;
        bool mIsCalc;
        bool mCancel;
        Flexlion::ThreadType mType;
        int mCps;
        nn::os::MutexType mMutex;
        nn::os::ThreadType mThread;
        void *stack;
    };
};