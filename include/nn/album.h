/**
 * @file nifm.h
 * @brief Network inferface module.
 */

#pragma once

#include "types.h"

namespace nn
{
    namespace album
    {
        enum ImageSize{
            is0
        };
        enum AlbumReportOption{
            aro0, aro1, aro2
        };
        Result SaveScreenshot(void const*,unsigned long, nn::album::ImageSize,nn::album::AlbumReportOption);
    };
};