#pragma once

#include "types.h"
#include "sead/bound.h"
#include "agl/DrawContext.h"
#include "sead/color.h"
#include "agl/utl/devtools.h"
#include "sead/matrix.h"
#include "sead/projection.h"
#include "sead/camera.h"
#include "Cmn/CameraUtl.h"
#include "Cmn/KDUtl/MtxT.h"
#include "sead/array.h"

namespace gsys {
    class ModelScene;
    class IModelProcedural{
        public:
       // virtual ~IModelProcedural();
        IModelProcedural();
        _BYTE _8[0xE0];
    };
    
    class ModelDebugInfo{
        _BYTE _0[0x200];
    };
    class ModelUnit{
        public:
        void setXluZPrePassID(int);
        void setXluAlpha(float,int,int);
    };
    class Model;
    class ModelNW : public gsys::ModelUnit{
        public:
        _BYTE _0[0x308];
        gsys::Model *mParent;
        void getBoneWorldMatrix(sead::Matrix34<float> *,int) const;
        int searchMaterialIndex(sead::SafeStringBase<char> const&)const;
    };
    class ModelRenderUnitNW{
        public:
        u64 _0;
        gsys::ModelNW *mModelUnit;
    };
    class FFLResource{
        public:
    };
    enum MaterialAccessKey{
        NONE = -1
    };
    class PartialSkeletalAnm{
        public:
    };
    class ModelResource;

    class AnmInfo{
        public:
        _BYTE _0[0x10];
        char *mName;
        _BYTE _18[0x10];
    };
    class AnmStruct{
        public:
        AnmInfo *info;
        _BYTE _8[0x20];
    };
    class ModelAnimation{
        public:
        class CreateArg{
            public:
            CreateArg(){
                memset(this, 0, sizeof(gsys::ModelAnimation::CreateArg));
                mSklAnimSlotNum = 4;
                mMatAnimSlotNum = 4;
                _8 = NULL;
            };
            u32 mSklAnimSlotNum;
            u32 mMatAnimSlotNum;
            u64 _8;
            u64 _10;
        };
        void *vftable;
        _BYTE _8[0x40];
        int *mSkelAnimKey;
        _BYTE _50[0x10];
        int mAnimationNum;
        AnmStruct **mAnims;
    };
    class IModelCallback{
        public:
        IModelCallback(){

        };
        u64 _0;
        u64 _8;
        u64 _10;
        u64 _18;
        u64 _20;
        u64 _28;
    };
    class Model {
        public:
            class CreateArg{
                public:
                CreateArg(){
                    memset(this, 0, sizeof(gsys::Model::CreateArg));
                    _0 = 0x40;
                    _4 = 1;
                    _0xC = 0;
                    _8 = 0x1010101;
                    _10 = NULL;
                };
                u32 _0;
                u32 _4;
                u32 _8;
                u32 _0xC;
                u64 _10;
                u32 _18;
                u32 _1C;
            };
            struct ModelUnitInfo{
                gsys::ModelUnit *mUnit;
                _BYTE _8[0x30];
            };
            void clearBoneLocalMatrix() const;
            void resetRenderToDepthShadow(int);
            void setCalcWeight(unsigned int,bool);
            void pushBack(gsys::ModelResource *,sead::SafeStringBase<char> const&,sead::Heap *,gsys::FFLResource const*);
			void calcBoundAABB(sead::BoundBox3<float> *, bool)const;
            bool isVisible(int) const;
            void setVisible(bool,int);
            void bind(gsys::ModelScene *);
            void updateBounding();
            void updateAnimationWorldMatrix(unsigned int);
            void updateAnimationWorldMatrix_(unsigned int);
            void requestDraw();
            void setXluAlpha(float,gsys::MaterialAccessKey,int);
            int searchBone(sead::SafeStringBase<char> const&) const;
            void calcDebug();
            void render(){
                this->mUpdateScale^=1;
                this->requestDraw();
            };
            void clearMaterialParameter();
            void clearMaterialTexture();
            void setEnableRenderToDepthSilhouette(bool,int);
            float getMaxViewDepth(int) const;
            void addModelUnit_(gsys::ModelUnit *,bool,sead::Heap *);
            void enableReverseCulling(bool);
			
            __int64 vtable;
			_BYTE _8[0x28];
			ModelUnitInfo *mModelInfoPool;
            sead::PtrArrayImpl mChildren;
            u64 _48;
            u64 _50;
            sead::Matrix34<float> mtx;
            sead::Vector3<float> mScale;
            sead::Vector3<float> mVel;
            bool mUpdateScale;
            bool _A1;
            bool mUpdate;
            bool _A3;
            u32 mModelNum;
            _BYTE _A8[0x28];
            gsys::ModelAnimation *mModelAnimation;
            IModelCallback *mModelCallback;
    };
    class ModelOcclusionQuery{
        public:
        void add(gsys::Model *);
    };
};
