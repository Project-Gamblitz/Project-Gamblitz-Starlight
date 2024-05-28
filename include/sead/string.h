/**
 * @file string.h
 * @brief Protected String classes.
 */

#pragma once

#include "types.h"
#include <cstring>

namespace sead
{
    class Heap;
	
    template<typename T>
    class SafeStringBase
    {
    public:
        virtual ~SafeStringBase() {};
        virtual sead::SafeStringBase<T> operator=(sead::SafeStringBase<T> const &);
        virtual sead::SafeStringBase<T> operator=(T *cool){
            this->mCharPtr = cool;
            return *this;
        }
        virtual void assureTerminationImpl_() const{};
        int findIndex(sead::SafeStringBase<T> const&) const;
        SafeStringBase(){
            this->mCharPtr = NULL;
        };
        SafeStringBase(const T *cool){
            this->mCharPtr = (char*)cool;
        };
        SafeStringBase(T *cool){
            this->mCharPtr = cool;
        };
        static sead::SafeStringBase<T> create(T* str) {
            sead::SafeStringBase<T> string;
            string = str;
            return string;
        };
        sead::SafeStringBase<T> operator=(const T *cool){
            this->mCharPtr = (char*)cool;
            return *this;
        };

        char* mCharPtr; // _8
    };

    template<typename T>
    class BufferedSafeStringBase : public sead::SafeStringBase<T>
    {
    public:
        BufferedSafeStringBase(char* buffer, int buffersize) {
            this->mCharPtr = buffer;
            mBufferSize = buffersize;
        };
        virtual ~BufferedSafeStringBase() {};
        virtual sead::SafeStringBase<T> operator=(sead::SafeStringBase<T> const &);
        virtual void assureTerminationImpl_() const;

        s32 formatImpl_(T *, s32, T const *, s32);
        s32 formatV(T const *, s32);
        s32 format(T const *, ...);
        s32 appendWithFormatV(T const *, s32);
        s32 appendWithFormat(T const *, ...);

        //char* mFormattedString; // _10
        u32 mBufferSize;
    };

    template<int T>
    class FixedSafeString : public sead::BufferedSafeStringBase<char>
    {
    public:
        FixedSafeString() {
            mCharPtr = (char*)buffer;
            mBufferSize = T;
        };
        virtual void assureTerminationImpl_();
        _BYTE buffer[T];
    };

    template <int T>
    class WFixedSafeString : public sead::BufferedSafeStringBase<char>
    {
    public:
        virtual ~WFixedSafeString();
        virtual sead::WFixedSafeString<T> operator=(sead::SafeStringBase<char> const &);
    };

    template <int T>
    class WFormatFixedSafeString : public sead::WFixedSafeString<T>
    {
    public:
        virtual ~WFormatFixedSafeString();
        virtual sead::WFormatFixedSafeString<T> operator=(sead::SafeStringBase<char> const &);
    };

    template<typename T>
    class HeapSafeStringBase : public sead::BufferedSafeStringBase<char>
    {
    public:
        HeapSafeStringBase(sead::Heap *,sead::SafeStringBase<char> const &, s32);
        virtual ~HeapSafeStringBase();
        virtual sead::SafeStringBase<T> operator=(sead::SafeStringBase<T> const &);
    };
    class StringUtil{
        public:
        static void convertUtf16ToUtf8(char *, unsigned int, const char16_t *, int);
        static void convertUtf8ToUtf16(char16_t *, unsigned int, const char *, int);
    };
};