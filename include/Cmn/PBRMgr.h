#pragma once

#include "types.h"
#include "sead/heap.h"
#include "agl/texturesampler.h"

namespace Cmn {
    class MainLight{
        public:
        _BYTE _0[0x1C0];
        sead::Vector3<float> mPos;
    };
    class PBRMgr {
        public:
        static Cmn::PBRMgr *sInstance;
        void loadResourceEnvMap_();
        void loadCustomizeEnvMap(sead::Heap *);
        void setupResourceEnvMap(const char*);
        void resetResourceEnvMap();
        agl::TextureSampler *getPrefilEnvMapSampler() const;
        void setArchiveEnvMap(agl::TextureSampler const*);
        void recreate();
        void retakeEnvMap();

        _BYTE _0[0x38];
        MainLight *mLight;
        _BYTE _40[0x83F8];
        agl::TextureSampler mCreatedSet;
    };
};