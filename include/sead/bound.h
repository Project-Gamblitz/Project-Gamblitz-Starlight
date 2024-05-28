/**
 * @file bound.h
 * @brief Classes for sizeable bounding boxes.
 */

#pragma once

#include "types.h"
#include "sead/vector.h"

namespace sead
{
    template<typename T>
    class BoundBox2
    {
    public:
        T mTop;
        T mLeft;
        T mRight;
        T mBottom;
    };

    template<typename T>
    class BoundBox3
    {
    public:
        sead::Vector3<T> mMin;
        sead::Vector3<T> mMax;
    };
};