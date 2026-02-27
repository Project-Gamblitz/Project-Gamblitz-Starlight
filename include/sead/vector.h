/**
 * @file vector.h
 * @brief Vector implementation.
 */

#pragma once

namespace sead
{
    template<typename T>
    class Vector2
    {
    public:
        static Vector2<T> zero; 
        T mX;
        T mY;
    };

    template<typename T>
    class Vector3
    {
    public:
        static Vector3<T> zero; 
        static Vector3<T> ey; 
        T mX;
        T mY;
        T mZ;
    };

    template<typename T>
    class Vector4
    {
    public:
        
        T mX;
        T mY;
        T mZ;
        T mH;
    };
};