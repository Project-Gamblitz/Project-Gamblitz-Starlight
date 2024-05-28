#pragma once

#include "types.h"
#include "sead/string.h"

namespace sead {
    class FileDevice{
        public:
        _BYTE _0[0x40]; // 90
        sead::SafeStringBase<char> mStr1;
        _BYTE _50[0x30];
        sead::SafeStringBase<char> mStr2;
    };
    class NinFileDeviceBase : public sead::FileDevice{
        public:
        NinFileDeviceBase(sead::SafeStringBase<char> const&, sead::SafeStringBase<char> const&);
    };
    class NinHostIOFileDevice : public sead::NinFileDeviceBase {
        public:
        NinHostIOFileDevice();
    };
    class NinContentFileDevice : public sead::NinFileDeviceBase {
        public:
        NinContentFileDevice();
    };
    class NinAocFileDevice : public sead::NinFileDeviceBase {
        public:
        NinAocFileDevice(sead::SafeStringBase<char> const&);
    };
    class NinSaveFileDevice : public sead::NinFileDeviceBase {
        public:
        NinSaveFileDevice(sead::SafeStringBase<char> const&);
    };
};