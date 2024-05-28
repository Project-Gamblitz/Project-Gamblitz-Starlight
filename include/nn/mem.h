/**
 * @file mem.h
 * @brief Memory allocation functions.
 */

#pragma once

#include "os.h"
#include "types.h"
#include <type_traits>
namespace nn
{
    namespace mem
    {
        class StandardAllocator
        {
        public:
            StandardAllocator();
            StandardAllocator(void* addr, size_t size);

            void Initialize(void* address, u64 size);
            void Finalize();
            void* Reallocate(void* address, u64 newSize);
            void* Allocate(u64 size);
            void* Allocate( size_t size, size_t alignment );
            void Free(void* address);
            void Dump();

            bool mIsInitialized; // _0
            bool mIsEnabledThreadCache; // _1
            uintptr_t m_CentralAllocatorAddr;
            uint32_t tlsSlotValue;
            std::aligned_storage< 48, 8 >::type storage;
        };
    };
};