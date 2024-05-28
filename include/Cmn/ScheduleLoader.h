#pragma once

#include "types.h"
#include "sead/string.h"
#include "sead/heap.h"

namespace Cmn{
    class ScheduleLoader{
        public:
        void *load(sead::SafeStringBase<char> const&,sead::SafeStringBase<char> const&,int,ulong *,bool,sead::Heap *); // first arg is folder(fesdata) and second is filename
    };
    class ScheduleFest{
        public:
        void *loadDeliMapFile(sead::SafeStringBase<char> const&,sead::Heap *) const;
    };
};