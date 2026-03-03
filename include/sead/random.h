/**
 * @file random.h
 * @brief Random API.
 */

#pragma once

#include "heap.h"
#include "types.h"

namespace sead
{
    class Heap;

    class Random
    {
    public:
        void init();
        void init(u32);
        void init(u32, u32, u32, u32);
        u32 getU32();
        u64 getU64();
        void getContext(u32 *, u32 *, u32 *, u32 *) const;

        u32 mX; // _0
        u32 mY; // _4
        u32 mZ; // _8
        u32 mW; // _C
    };

    class GlobalRandom : public sead::Random
    {
    public:
        static GlobalRandom* sInstance;
    };
};