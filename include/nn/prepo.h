/**
 * @file prepo.h
 */

#pragma once

#include "os.h"
#include "types.h"
#include "nn/account.h"
#include "sead/string.h"

namespace nn
{
    namespace prepo
    {
        class PlayReport{
            public:
            void Save(nn::account::Uid const&);
        };
    };
};