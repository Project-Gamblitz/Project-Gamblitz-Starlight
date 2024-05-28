#pragma once

#include "types.h"
#include <array>
#include "libnx/nx/include/switch.h"

namespace exl::util {

    struct Range {
        uintptr_t m_Start;
        size_t m_Size;

        constexpr uintptr_t GetEnd() const { return m_Start + m_Size; }
    };

    struct ModuleInfo {
        Range m_Total;
        Range m_Text;
        Range m_Rodata;
        Range m_Data;
        /* TODO: bss? */
    };

    namespace mem_layout {
        static constexpr int s_MaxModules = 13;
        inline int s_ModuleCount = -1;
    }

    namespace impl {
        void InitMemLayout();

        namespace mem_layout {
            inline std::array<ModuleInfo, util::mem_layout::s_MaxModules> s_ModuleInfos;
        }
    }

    static inline const ModuleInfo& GetModuleInfo(int index) {
        if(!(index < mem_layout::s_ModuleCount)) *(u64*)69 = 69;
        return impl::mem_layout::s_ModuleInfos.at(index); 
    }

    namespace mem_layout {
        static constexpr int s_RtldModuleIdx = 0;
        static constexpr int s_MainModuleIdx = 1;

        /* Decided at runtime. */
        inline int s_SelfModuleIdx = -1;

        }
    static inline const ModuleInfo& GetRtldModuleInfo() { return GetModuleInfo(mem_layout::s_RtldModuleIdx); }
    static inline const ModuleInfo& GetMainModuleInfo() { return GetModuleInfo(mem_layout::s_MainModuleIdx); }

    
    static inline const ModuleInfo& GetSelfModuleInfo() { return GetModuleInfo(mem_layout::s_SelfModuleIdx); }
};