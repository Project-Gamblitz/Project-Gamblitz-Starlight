/**
 * @file camera.h
 * @brief Basic camera implementation.
 */

#pragma once

#include "matrix.h"
#include "runtime.h"
#include "vector.h"

namespace sead
{
    class OrthoProjection;

    class Camera
    {
    public:
        Camera(){
            this->mMtx = sead::Matrix34<float>::ident;
        };
        ~Camera();
        virtual void coolVtableFunc();

        bool checkDerivedRuntimeTypeInfo(sead::RuntimeTypeInfo::Interface const *);
        sead::RuntimeTypeInfo::Interface* getRuntimeTypeInfo() const;
        void doUpdateMatrix(sead::Matrix34<float> *) const;

        void getWorldPosByMatrix(sead::Vector3<float> *) const;
        void getLookVectorByMatrix(sead::Vector3<float> *) const;
        void getRightVectorByMatrix(sead::Vector3<float> *) const;
        void getUpVectorByMatrix(sead::Vector3<float> *) const;
        void worldPosToCameraPosByMatrix(sead::Vector3<float> *, sead::Vector3<float> const &) const;
        void cameraPosToWorldPosByMatrix(sead::Vector3<float> *, sead::Vector3<float> const &) const;

        //u64 vtable;
        sead::Matrix34<float> mMtx;
		sead::Vector3<float> mPos;
    };

    class LookAtCamera : public sead::Camera
    {
    public:
        LookAtCamera(sead::Vector3<f32> const &, sead::Vector3<f32> const &, sead::Vector3<f32> const &);

         ~LookAtCamera();

        bool checkDerivedRuntimeTypeInfo(sead::RuntimeTypeInfo::Interface const *);
        sead::RuntimeTypeInfo::Interface* getRuntimeTypeInfo() const;

        void doUpdateMatrix(sead::Matrix34<f32> *) const;
        sead::Vector3<float> mLookAtPos;//mLookAtPos?
    };

    class DirectCamera : public sead::Camera
    {
    public:
        ~DirectCamera();

        void doUpdateMatrix(sead::Matrix34<f32> *) const;
    };

    class OrthoCamera : public sead::LookAtCamera
    {
    public:
        OrthoCamera();
        OrthoCamera(sead::Vector2<f32> const &, f32);
        OrthoCamera(sead::OrthoProjection const &);

        ~OrthoCamera();

        bool checkDerivedRuntimeTypeInfo(sead::RuntimeTypeInfo::Interface const *);
        sead::RuntimeTypeInfo::Interface* getRuntimeTypeInfo() const;

        void setByOrthoProjection(sead::OrthoProjection const &);
        void setRotation(f32 rotation);
    };
};