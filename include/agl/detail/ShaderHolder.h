/**
 * @file ShaderHolder.h
 * @brief Defines classes for shader storage.
 */

#pragma once

#include "types.h"
#include "sead/heap.h"
#include "agl/DrawContext.h"

namespace agl
{
    class UniformLocation{
        public:
        void setUniformNVN(agl::DrawContext *, unsigned int, const void *) const;
        u32 mIdk;
    };
    namespace detail
    {
        class ShaderHolder
        {
        public:
            static agl::detail::ShaderHolder *sInstance;
            class SingletonDisposer_
            {
            public:
                virtual ~SingletonDisposer_();

                static SingletonDisposer_ sStaticDisposer;
            };
            class SomeShaderLol{
                public:
                _BYTE _0[0x288];
                u32 mSomeInt;
                agl::UniformLocation *mUniformLocation;
            };
            class SomethingHoldingShadersLol{
                public:
                _BYTE _0[0x1B8];
                SomeShaderLol *devToolsShader;
            };

            ShaderHolder();

            static agl::detail::ShaderHolder* createInstance(sead::Heap *);
            static void deleteInstance();
            _BYTE _0[0x30];
            SomethingHoldingShadersLol *mShaders;
        };
    };
};