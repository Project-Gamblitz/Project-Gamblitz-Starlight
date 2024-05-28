#pragma once

#include "types.h"

namespace sead {
    template <class T, u64 size>
    class FixedRingBuffer {
        public:
        T* mBufferPtr;
        u64 mSize;
        u64 mUnk;
        T mBuffer[size];
    };
    template <class T>
    class RingBuffer{
        public:
        RingBuffer(){
            mBufferPtr = mBuffer;
            mBufferSize = 0x40;
        };
        T* mBufferPtr;
        u64 mBufferSize;
        u32 mSize;
        T mBuffer[0x100];
    };
};