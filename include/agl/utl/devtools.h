#pragma once

#include "types.h"
#include "agl/DrawContext.h"
#include "agl/IndexStream.h"
#include "agl/detail/ShaderHolder.h"
#include "sead/vector.h"
#include "sead/color.h"
#include "sead/bound.h"
#include "sead/matrix.h"
#include "sead/camera.h"
#include "sead/projection.h"
#include "agl/texturesampler.h"
#include "pfnc.h"
#include "flexlion/ProcessMemory.hpp"

namespace agl {
    namespace utl {
        class DevTools{
            public:
            enum CameraControlType{
                mNormal
            };
            void static beginDrawImm(agl::DrawContext *,sead::Matrix34<float> const&,sead::Matrix44<float> const&);
            void static drawTriangleImm(agl::DrawContext*ctx, sead::Vector3<float> const&v1, sead::Vector3<float> const&v2, sead::Vector3<float> const&v3, sead::Color4f const&color);
            void static drawLineImm(agl::DrawContext*, sead::Vector3<float> const&, sead::Vector3<float> const&, sead::Color4f const&, float);
			void static drawBoundBoxImm(agl::DrawContext *, sead::BoundBox3<float> const&, sead::Color4f const&, float);
            //float static getFrameSpeed();
            //void static setFrameSpeed(float);
            //static void drawCursor(agl::DrawContext *a, sead::Vector2<float> const& a2, sead::Vector2<float> const& a3, float a4)
            static void drawCameraAndFrustum(agl::DrawContext *, sead::Camera const&, sead::Projection const&, sead::Matrix34<float> const&, sead::Matrix44<float> const&, bool, sead::Color4f const&, sead::Color4f const&, float);
            void static drawAxisImm(agl::DrawContext *, sead::Matrix34<float> const&, float, float, float);
            //void static drawArrow(agl::DrawContext*, sead::Vector3<float> const&, sead::Vector3<float> const&, sead::Color4f const&, sead::Color4f const&, float, sead::Matrix34<float> const&, sead::Matrix44<float> const&);
            void static controlCamera(sead::LookAtCamera *,sead::Vector2<float> const&,sead::Vector2<float> const&,float,float,float,bool,agl::utl::DevTools::CameraControlType);
            void static drawTexture(agl::DrawContext *, agl::TextureSampler const&, sead::Matrix34<float> const&, sead::Matrix44<float> const&, sead::Color4f const&);
            void static drawTextureTexCoord(agl::DrawContext*, agl::TextureSampler const&, sead::Matrix34<float> const&, sead::Matrix44<float> const&, sead::Vector2<float> const&, float, sead::Vector2<float> const&);
        };
    };
};