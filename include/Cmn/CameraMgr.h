#pragma once

#include "types.h"
#include "sead/vector.h"
#include "gsys/layer.h"


namespace Cmn {
    class CameraBase;
    class CameraMgr;
    class CameraInterpolation{
        public:
        enum Type{
            m0, m1, m2, m3
        };
    };
    class CameraMgr{
        public:
        enum LayerId{
            Layer1, Layer2, Layer3
        };
        void pushCamera(Cmn::CameraBase *);
        void swapCamera(Cmn::CameraBase*, Cmn::CameraBase*);
        void fifthCalc();
        void flushRenderCameraToGfx();
        _BYTE _0[0x348];
        gsys::Layer3D *mLayer;
        u32 mLayerId;
        u32 _354;
        u32 mCamNum;
        u32 _35C;
        _BYTE _360[0x28];
        Cmn::CameraBase *mCam;
        u64 _390;
        u32 mInterPolType;
        u32 mInterPolSmth;
        _BYTE _3A0[0x220];
        sead::Vector4<float> mRenderProportions;
    };
};