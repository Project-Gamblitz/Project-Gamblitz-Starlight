#pragma once

#include "types.h"
#include "sead/vector.h"
#include "sead/string.h"
#include "CameraMgr.h"
#include "sead/camera.h"
#include "Lp/Sys/ctrl.h"
#include "starlight/input.hpp"
#include "math.h"
#include "Lp/Sys/camera.h"

namespace Cmn {
	class CameraBase {
		public:
        void lookAt(sead::Vector3<float> const&, sead::Vector3<float> const&, sead::Vector3<float> const&);
        void setPos(sead::Vector3<float> const&);
        void setAt(sead::Vector3<float> const&);
        void setUp(sead::Vector3<float> const&);
        void setFovy(float);
        void setNear(float);
        void setFar(float);
        void setOrthoProjectionMode();
        u64* getProjection();
        CameraBase(sead::SafeStringBase<char> const&, bool);
        void push(Cmn::CameraMgr::LayerId);
        sead::Camera *getCamera() const;
        void setMtx(sead::Matrix34<float> const&);
        _BYTE _0[0x10];
        Cmn::CameraMgr *cameraMgr;
        _BYTE _18[0x20];
        u32 _38;
        u32 _3C;
        u32 _40;
        u32 _44;
        u32 _48;
        u32 _4C;
        sead::Vector3<float> mPosition;
        sead::Vector3<float> mAt;
        sead::Vector3<float> mUp;
        sead::Projection *mProjection;
        _BYTE _80[0x120 - 0x80];
        sead::Vector4<float> mProjVec;
        _BYTE _120[0x178 - 0x128];
	};
};