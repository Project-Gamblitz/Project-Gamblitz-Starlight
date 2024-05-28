#include "flexlion/ProcessMemory.hpp"


u64 ProcessMemory::ModuleAddr(u64 addr, int moduleIndex)
{
    if(moduleIndex < 0 or moduleIndex > 12){
        return NULL;
    }
    MemoryInfo mi = {};
    u32 pi;
    u64 curaddr = 0;
    int moduleCount = 0;
    static u64 allAddr[13];
    
    if(allAddr[moduleIndex] == NULL){
        while (true)
        {
            svcQueryMemory(&mi, &pi, curaddr);
            if (mi.perm == Perm_Rx && moduleCount++ == moduleIndex)
                break;
                
            curaddr = mi.addr + mi.size;
        }
        allAddr[moduleIndex] = mi.addr;
    }
    
    return allAddr[moduleIndex] + addr;
}

u64 ProcessMemory::ImageAddr(u64 addr)
{
    return ProcessMemory::RtldAddr(addr);
}
u64 ProcessMemory::RtldAddr(u64 addr)
{
    return ProcessMemory::ModuleAddr(addr, 0);
}
u64 ProcessMemory::MainAddr(u64 addr)
{
    exl::util::impl::InitMemLayout();
    return exl::util::GetMainModuleInfo().m_Total.m_Start + addr;
}

u64 ProcessMemory::SdkAddr(u64 addr)
{
    return ProcessMemory::ModuleAddr(addr, 3);
}

u64 ProcessMemory::Subsdk0Addr(u64 addr)
{
    return ProcessMemory::ModuleAddr(addr, 2);
}

ProcessMemory::ModuleIndex ProcessMemory::getModuleForAddr(u64 addr){
    if(ProcessMemory::RtldAddr(0) > addr){
        return ProcessMemory::ModuleIndex::Invalid;
    } else if(ProcessMemory::MainAddr(0) > addr and addr >= ProcessMemory::RtldAddr(0)){
        return ProcessMemory::ModuleIndex::Rtld;
    } else if(ProcessMemory::Subsdk0Addr(0) > addr and addr >= ProcessMemory::MainAddr(0)){
        return ProcessMemory::ModuleIndex::Main;
    } else if(ProcessMemory::SdkAddr(0) > addr and addr >= ProcessMemory::Subsdk0Addr(0)){
        return ProcessMemory::ModuleIndex::Subsdk0;
    } else{
        return ProcessMemory::ModuleIndex::Sdk;
    }
}