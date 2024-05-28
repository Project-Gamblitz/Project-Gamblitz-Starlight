#pragma once

#include "types.h"
#include "sead/heap.h"
#include "agl/util.h"


namespace Cmn {
    class AppUBOMgr : sead::IDisposer {
        public:
        static Cmn::AppUBOMgr *sInstance;

        /*
        agl::UniformBlock agl::uniformblock20;
        agl::UniformBlock agl::uniformblock98;
        _BYTE byte110;
        sead::Color4f colors[18];
        */

        void updateUBO(bool);
		void setPaintSurface(float, float, float, float);
		void setPaintSurface2(float, float, float);
		void setPaintSurface3(float, float, float);
		void setPhantomFilter(float, float, float, sead::Color4f const&);
		void setRadialFogParam(sead::Vector4<float> const&, sead::Color4f const&);
		void setPaintAnimation(float, float, float, float);
        void setFogParam(sead::Vector4<float> const&, sead::Vector4<float> const&, sead::Color4f const&);
    };
};