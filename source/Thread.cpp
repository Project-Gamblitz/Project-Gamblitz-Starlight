#include "flexlion/Thread.hpp"
#include <stdlib.h>

namespace Flexlion{
    Thread::Thread(voidFunc function, Flexlion::ThreadType type, int cps){
        memset(this, 0, sizeof(Flexlion::Thread));
        stack = aligned_alloc(0x1000, 0x4000);
        mFunction = function;
        this->mType = type;
        mIsCalc = 1;
        mCps = cps;
    }
    Thread::~Thread(){
        if(this->mInit){
            this->stop();
        }
        delete stack;
    }
    void Thread::calc(){
        this->mRunning = 1;
        while(!this->mCancel){
            if(!this->mIsCalc){
                nn::os::SleepThread(nn::TimeSpan::FromNanoSeconds(int(1000000000 / mCps)));
                continue;
            }
            this->mFunction();
            switch(this->mType){
            case Flexlion::ThreadType::cManualSync:
                this->mIsCalc = 0;
                break;
            case Flexlion::ThreadType::cOnce:
                this->mRunning = 0;
                this->stop();
                continue;
            };
            nn::os::SleepThread(nn::TimeSpan::FromNanoSeconds(int(1000000000 / mCps)));
        }
        this->mRunning = 0;
    }
    void Thread::start(){
        if(this->mInit){
            return;
        }
        this->mCancel = 0;
        nn::os::LockMutex(&this->mMutex);
        nn::os::CreateThread(&this->mThread, (voidFuncArg)(&Thread::calc), this, this->stack, 0x4000, 16, 3);
        nn::os::StartThread(&this->mThread);
        nn::os::UnlockMutex(&this->mMutex);
        this->mInit = 1;
    }
    void Thread::stop(){
        if(!this->mInit){
            return;
        }
        nn::os::LockMutex(&this->mMutex);
        if(this->mRunning){
            this->mCancel = 1;
            while(this->mRunning){
                nn::os::SleepThread(nn::TimeSpan::FromNanoSeconds(1));
            }
        }
        nn::os::DestroyThread(&this->mThread);
        nn::os::UnlockMutex(&this->mMutex);
        this->mInit = 0;
    }
};