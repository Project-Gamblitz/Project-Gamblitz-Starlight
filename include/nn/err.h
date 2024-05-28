#pragma once
#include "types.h"
#include "nn/settings.h"

namespace nn
{
    typedef u32 Result;
    
    class err
    {
        public:
        class ApplicationErrorArg
        {
            public:
            ApplicationErrorArg();
            ApplicationErrorArg(unsigned int, char const*, char const*, nn::settings::LanguageCode const&);
            int ApplicationErrorArg_x0; //unknown, sdk sets it always to 0x01020000
            int ApplicationErrorArg_x4; //never used
            int ErrorCodeNumber;
            u64 LanguageCode;
            char DialogMessage[2048];
            char FullScreenMessage[2048];
        } PACKED;

        static void ShowError(nn::Result);
        static void ShowApplicationError(nn::err::ApplicationErrorArg const&);
    };
};