/**
 * @file System.h
 * @brief VFX system implementation.
 */

#pragma once

#include "Config.h"
#include "Heap.h"
#include "nn/util/Float2.h"
#include "nn/util.h"
// this class is massive
namespace nn
{
    namespace vfx
    {
        enum BufferSwapMode{None};
        class EmitterSetInfo{
            public:
            _BYTE _0[0x41];
            bool mIsContinue;
        };
        class EmitterSet_150{
            public:
            _BYTE _0[0x78];
            EmitterSet_150 *mIterNode;
            _BYTE _80[0x420 - 0x80];
            sead::Color4f mColor1;
            sead::Color4f mColor2;
        };
        class EmitterSet{
            public:
            void SetMatrix(nn::util::neon::MatrixRowMajor4x3fType const&);
            _BYTE _0[0xA];
            bool mIsContinue;
            _BYTE _0xB[0x18 - 0xB];
            int mReqUpdatePos;
            _BYTE _1C[0x90 - 0x1C];
            sead::Vector3<float> mPos1;
            _BYTE _9C[0xD0 - 0x9C];
            sead::Vector3<float> mPos2;
            _BYTE _DC[0x150 - 0xDC];
            EmitterSet_150 *_150;
            _BYTE _158[0x1D0 - 0x158];
            nn::vfx::EmitterSet *emitterSetIter; // 1d0
        };
        class Emitter;
        class Handle{
            public:
            EmitterSet *GetEmitterSet();

            nn::vfx::Emitter *mEmitter;
            int mCreateId;
            int _0xC;
        };
        class Emitter{
            public:
            void SwapBuffer(nn::vfx::BufferSwapMode);
        };
        class RawEmitterSetBody{
            public:
            _BYTE _0[0x10];
            char mName;
        };
        class EmitterSetResource{
            public:
            char *getName(){
                return &mRawESETBody->mName;
            }
            RawEmitterSetBody *mRawESETBody;
        };
        class Resource{
            public:
            _BYTE _0[0x9];
            bool _9;
            _BYTE _A[0x42];
            u32 mEmitterSetCount;
            u64 _50;
            nn::vfx::EmitterSetResource **mEmitterSetResource;
        };
        class System
        {
        public:
            System(nn::vfx::Config const &);

            virtual ~System();
            virtual void Initialize(nn::vfx::Heap *,nn::vfx::Heap *, nn::vfx::Config const &);
            void Calculate(nn::vfx::EmitterSet *,float,nn::vfx::BufferSwapMode);

            _BYTE _8[0x5C];
            u32 mResourceCount;
            _BYTE _68[0x28];
            nn::vfx::Resource **mResources;
            _BYTE _98[0x18];
            nn::vfx::EmitterSet *mEmitterNodes[64];
            _BYTE _2B0[0x1450];
        };
        class detail{
            public:
            class ResAnimEmitterKeyParamSet;
            class EmitterCalculator{
                public:
                void UpdateEmitterMatrix(nn::vfx::Emitter *);
                void UpdateCurrentParticleGpuBufferForCpuEmitter(nn::vfx::Emitter *);
                void ApplyEmitterAnimation(nn::vfx::Emitter *, nn::util::neon::MatrixRowMajor4x3fType *, nn::util::neon::MatrixRowMajor4x3fType *);
                void CalculateParticleBehavior(nn::util::neon::Vector3fType *, nn::util::neon::Vector3fType *, float *, float *, nn::vfx::Emitter *, int, float, nn::util::neon::Vector3fType const&, nn::util::neon::Vector3fType const&);
            };
            static void CalculateEmitterKeyFrameAnimation(nn::util::Float3 *, bool *, nn::vfx::detail::ResAnimEmitterKeyParamSet const*, float);
        };
    };
};