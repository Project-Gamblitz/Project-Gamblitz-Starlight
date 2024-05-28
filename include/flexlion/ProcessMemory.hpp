#pragma once

#include "types.h"
#include "config.h"
#include "libnx/nx/include/switch.h"
#include "exl/mem_layout.hpp"

class ProcessMemory
{
    public:
    enum ModuleIndex{
        Invalid, Rtld, Main, Subsdk0, Subsdk1, Sdk
    };
        static u64 ModuleAddr(u64 addr, int moduleIndex);
        static u64 ImageAddr(u64 addr);
        static u64 RtldAddr(u64 addr);
        static u64 MainAddr(u64 addr);
        static u64 SdkAddr(u64 addr);
        static u64 Subsdk0Addr(u64 addr);
        static ProcessMemory::ModuleIndex getModuleForAddr(u64 addr);
};