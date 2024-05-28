#pragma once

#include "types.h"
#include "sead/string.h"
#include "sead/FileDevice.h"

namespace sead {
    class FileDeviceMgr {
        public:
        static sead::FileDeviceMgr *sInstance;
        void mount(sead::FileDevice *,sead::SafeStringBase<char> const&);
        void mount_(sead::Heap *);
    };
};