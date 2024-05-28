#pragma once

#include "types.h"

#include "sead/heap.h"
#include "agl/DrawContext.h"
#include "Cmn/CameraBase.h"
#include "gsys/model.h"
#include "Lp/Sys/modelarc.h"
#include "Lp/Utl.h"
#include "Cmn/GfxUtl.h"
#include "Lp/Sys/worker.h"
//#include "gsys/ModelScene.h"

namespace Lp{
    namespace Sys{
        class Scene;
    };
};
namespace Cmn{
    class ViewerEnvChanger
    {
        public:
        virtual ~ViewerEnvChanger(){
            //delete[] this;
        }
        ViewerEnvChanger(){
            //mLoadResourceEnvMapJob = Lp::Sys::WorkerJobBase();
            byte10 = 0;
            mDbgMapSort = 0LL;
            field_20 = 0;
            mModelScene = 0LL;
            memset(&this->field_28, 0, 128);
            dwordB8 = 0;
            dwordD0 = 0;
            qwordA8 = (u64)&this->qwordA8;
            qwordB0 = (u64)&this->qwordA8;
            qwordC0 = 0LL;
            qwordC8 = 0LL;
            dwordBC = 240;
        }
        /*__int64 __fastcall Cmn::ViewerEnvChanger::initialize(Cmn::ViewerEnvChanger *this, gsys::ModelScene *a2, sead::Heap *a3)
        {
        gsys::ModelScene *v3; // x22
        Cmn::ViewerEnvChanger *v4; // x19
        sead::Heap *v5; // x28
        __int64 v6; // x0
        DbgSetting::MapSort *v7; // x22
        int *v8; // x8
        signed __int64 v9; // x20
        signed __int64 v10; // x21
        int *v11; // x9
        signed __int64 v12; // x8
        _QWORD *v13; // x23
        const char *v14; // x24
        Cmn::ViewerEnvApplyerGame *v15; // x23
        __int64 v16; // x8
        signed int *v17; // x8
        signed __int64 v18; // x20
        signed __int64 v19; // x21
        signed int *v20; // x9
        signed __int64 v21; // x8
        _QWORD *v22; // x23
        const char *v23; // x24
        Cmn::ViewerEnvApplyerGame *v24; // x23
        __int64 v25; // x8
        __int64 v26; // x8
        signed __int64 v27; // x20
        signed __int64 v28; // x21
        _QWORD *v29; // x9
        signed __int64 v30; // x8
        _QWORD *v31; // x23
        const char *v32; // x24
        Cmn::ViewerEnvApplyerGame *v33; // x23
        __int64 v34; // x8
        int v35; // w23
        __int64 v36; // x20
        char *v37; // x22
        __int64 v38; // x24
        __int64 v39; // x8
        char *v40; // x26
        char *v41; // x27
        int v42; // w8
        signed __int64 v43; // x9
        bool v44; // nf
        unsigned __int8 v45; // vf
        int v46; // w9
        sead::Heap *v47; // x21
        __int64 v48; // x28
        _QWORD *v49; // x0
        __int64 v50; // x8
        __int64 v51; // x8
        __int64 v52; // x0
        __int64 v53; // x8
        __int64 v54; // x0
        __int64 v55; // x8
        __int64 v56; // x0
        __int64 v57; // x8
        Lp::Utl *v58; // x0
        __int64 v59; // x8
        __int64 (__fastcall *v60)(Cmn::PBRMgr *__hidden); // x9
        __int64 v61; // x0
        sead::Heap *v63; // [xsp+0h] [xbp-80h]
        char *v64; // [xsp+8h] [xbp-78h]
        char *v65; // [xsp+10h] [xbp-70h]
        char *v66; // [xsp+18h] [xbp-68h]
        sead::PtrArrayImpl *v67; // [xsp+20h] [xbp-60h]
        char v68; // [xsp+28h] [xbp-58h]

        v3 = a2;
        v4 = this;
        v5 = a3;
        Cmn::ProfilerScope::ProfilerScope((Cmn::ProfilerScope *)&v68, "ViewerEnvChanger");
        v6 = 1LL;
        if ( v5 )
            v6 = sead::HeapMgr::setCurrentHeap_((sead::HeapMgr *)*off_4156B70, v5);
        v63 = (sead::Heap *)v6;
        v4->mModelScene = (__int64)v3;
        v7 = (DbgSetting::MapSort *)operator new(0x32C000uLL, v5, 8);
        DbgSetting::MapSort::MapSort(v7);
        v4->mDbgMapSort = (__int64)v7;
        DbgSetting::MapSort::load(v7);
        sead::PtrArrayImpl::allocBuffer(&v4->mApplyerGameArry0, *(_DWORD *)(v4->mDbgMapSort + 12), v5, 8);
        v8 = (int *)v4->mDbgMapSort;
        if ( v8[3] >= 1 )
        {
            v9 = 0LL;
            v10 = 561152LL;
            do
            {
            v11 = &v8[v10];
            v12 = (signed __int64)(v8 + 561152);
            if ( (unsigned __int64)v9 >= 0x800 )
                v13 = (_QWORD *)v12;
            else
                v13 = v11;
            (*(void (__fastcall **)(_QWORD *))(*v13 + 24LL))(v13);
            v14 = (const char *)v13[1];
            v15 = (Cmn::ViewerEnvApplyerGame *)operator new(0x42E0uLL, v5, 8);
            Cmn::ViewerEnvApplyerGame::ViewerEnvApplyerGame(v15, (gsys::ModelScene *)v4->mModelScene, v14);
            v16 = *(_QWORD *)&v4->mApplyerGameArry0.mLength;
            if ( (signed int)v16 < SHIDWORD(v16) )
            {
                *(_QWORD *)(v4->mApplyerGameArry0.mPtr + (v16 << 32 >> 29)) = v15;
                ++v4->mApplyerGameArry0.mLength;
            }
            v8 = (int *)v4->mDbgMapSort;
            ++v9;
            v10 += 22LL;
            }
            while ( v9 < v8[3] );
        }
        sead::PtrArrayImpl::allocBuffer(&v4->mApplyerGameArry1, *v8, v5, 8);
        v17 = (signed int *)v4->mDbgMapSort;
        if ( *v17 >= 1 )
        {
            v18 = 0LL;
            v19 = 425984LL;
            do
            {
            v20 = &v17[v19];
            v21 = (signed __int64)(v17 + 425984);
            if ( (unsigned __int64)v18 >= 0x800 )
                v22 = (_QWORD *)v21;
            else
                v22 = v20;
            (*(void (__fastcall **)(_QWORD *))(*v22 + 24LL))(v22);
            v23 = (const char *)v22[1];
            v24 = (Cmn::ViewerEnvApplyerGame *)operator new(0x42E0uLL, v5, 8);
            Cmn::ViewerEnvApplyerGame::ViewerEnvApplyerGame(v24, (gsys::ModelScene *)v4->mModelScene, v23);
            v25 = *(_QWORD *)&v4->mApplyerGameArry1.mLength;
            if ( (signed int)v25 < SHIDWORD(v25) )
            {
                *(_QWORD *)(v4->mApplyerGameArry1.mPtr + (v25 << 32 >> 29)) = v24;
                ++v4->mApplyerGameArry1.mLength;
            }
            v17 = (signed int *)v4->mDbgMapSort;
            ++v18;
            v19 += 22LL;
            }
            while ( v18 < *v17 );
        }
        sead::PtrArrayImpl::allocBuffer(&v4->mApplyerGameArry2, v17[1], v5, 8);
        v26 = v4->mDbgMapSort;
        if ( *(_DWORD *)(v26 + 4) >= 1 )
        {
            v27 = 0LL;
            v28 = 1884160LL;
            do
            {
            v29 = (_QWORD *)(v26 + v28);
            v30 = v26 + 1884160;
            if ( (unsigned __int64)v27 >= 0x800 )
                v31 = (_QWORD *)v30;
            else
                v31 = v29;
            (*(void (__fastcall **)(_QWORD *))(*v31 + 24LL))(v31);
            v32 = (const char *)v31[1];
            v33 = (Cmn::ViewerEnvApplyerGame *)operator new(0x42E0uLL, v5, 8);
            Cmn::ViewerEnvApplyerGame::ViewerEnvApplyerGame(v33, (gsys::ModelScene *)v4->mModelScene, v32);
            v34 = *(_QWORD *)&v4->mApplyerGameArry2.mLength;
            if ( (signed int)v34 < SHIDWORD(v34) )
            {
                *(_QWORD *)(v4->mApplyerGameArry2.mPtr + (v34 << 32 >> 29)) = v33;
                ++v4->mApplyerGameArry2.mLength;
            }
            v26 = v4->mDbgMapSort;
            ++v27;
            v28 += 88LL;
            }
            while ( v27 < *(signed int *)(v26 + 4) );
        }
        v67 = &v4->mApplyerShopArry;
        sead::PtrArrayImpl::allocBuffer(&v4->mApplyerShopArry, 4, v5, 8);
        v35 = (unsigned __int8)*off_4156128[0];
        v66 = (char *)off_41594E8 + 16;
        v65 = (char *)off_4156498 + 16;
        v64 = (char *)off_4156848 + 16;
        v36 = 0LL;
        v37 = (char *)off_41565A0 + 16;
        do
        {
            v38 = operator new(0x68uLL, v5, 8);
            v39 = v4->mModelScene;
            *(_QWORD *)(v38 + 24) = v38 + 36;
            *(_QWORD *)v38 = v66;
            *(_QWORD *)(v38 + 8) = v39;
            *(_QWORD *)(v38 + 16) = v65;
            v40 = off_3E0B780[v36];
            *(_DWORD *)(v38 + 32) = 64;
            sead::BufferedSafeStringBase<char>::assureTerminationImpl_(v38 + 16);
            *(_QWORD *)(v38 + 16) = v64;
            v41 = *(char **)(v38 + 24);
            if ( v41 != v40 )
            {
            v42 = 0;
            v43 = 2LL;
            while ( (unsigned __int8)v40[v43 - 2] != v35 )
            {
                if ( (unsigned __int8)v40[v43 - 1] == v35 )
                {
                ++v42;
                break;
                }
                if ( (unsigned __int8)v40[v43] == v35 )
                {
                v42 = v43;
                break;
                }
                v42 += 3;
                v45 = __OFSUB__(v43, 0x80000LL);
                v44 = v43 - 0x80000 < 0;
                v43 += 3LL;
                if ( !(v44 ^ v45) )
                {
                v42 = 0;
                break;
                }
            }
            v46 = *(_DWORD *)(v38 + 32);
            if ( v42 >= v46 )
                v42 = v46 - 1;
            v47 = v5;
            v48 = v42;
            sub_1BD0240(*(_QWORD *)(v38 + 24), v40, v42);
            v41[v48] = v35;
            v5 = v47;
            }
            *(_QWORD *)(v38 + 16) = v37;
            if ( (signed int)*(_QWORD *)&v67->mLength < (signed int)(*(_QWORD *)&v67->mLength >> 32) )
            {
            *(_QWORD *)(v4->mApplyerShopArry.mPtr + (*(_QWORD *)&v67->mLength << 32 >> 29)) = v38;
            ++v4->mApplyerShopArry.mLength;
            }
            ++v36;
        }
        while ( v36 != 4 );
        sead::PtrArrayImpl::allocBuffer(&v4->mApplyerSubstanceArry, 1, v5, 8);
        v49 = (_QWORD *)operator new(0x10uLL, v5, 8);
        v50 = v4->mModelScene;
        *v49 = (char *)off_4159508 + 16;
        v49[1] = v50;
        v51 = *(_QWORD *)&v4->mApplyerSubstanceArry.mLength;
        if ( (signed int)v51 < SHIDWORD(v51) )
        {
            *(_QWORD *)(v4->mApplyerSubstanceArry.mPtr + (v51 << 32 >> 29)) = v49;
            ++v4->mApplyerSubstanceArry.mLength;
        }
        sead::PtrArrayImpl::allocBuffer(&v4->mApplyerArry, 4, v5, 8);
        v52 = Cmn::ViewerEnvChanger::createApplyerByName_(v4, "Fld_Plaza00_Plz", v5);
        v53 = *(_QWORD *)&v4->mApplyerArry.mLength;
        if ( (signed int)v53 < SHIDWORD(v53) )
        {
            *(_QWORD *)(v4->mApplyerArry.mPtr + (v53 << 32 >> 29)) = v52;
            ++v4->mApplyerArry.mLength;
        }
        v54 = Cmn::ViewerEnvChanger::createApplyerByName_(v4, "Fld_Tutorial00_Ttr", v5);
        v55 = *(_QWORD *)&v4->mApplyerArry.mLength;
        if ( (signed int)v55 < SHIDWORD(v55) )
        {
            *(_QWORD *)(v4->mApplyerArry.mPtr + (v55 << 32 >> 29)) = v54;
            ++v4->mApplyerArry.mLength;
        }
        v56 = Cmn::ViewerEnvChanger::createApplyerByName_(v4, "Fld_ShootingRange_Shr", v5);
        v57 = *(_QWORD *)&v4->mApplyerArry.mLength;
        if ( (signed int)v57 < SHIDWORD(v57) )
        {
            *(_QWORD *)(v4->mApplyerArry.mPtr + (v57 << 32 >> 29)) = v56;
            ++v4->mApplyerArry.mLength;
        }
        v58 = (Lp::Utl *)Cmn::ViewerEnvChanger::createApplyerByName_(v4, "Customize", v5);
        v59 = *(_QWORD *)&v4->mApplyerArry.mLength;
        if ( (signed int)v59 < SHIDWORD(v59) )
        {
            *(_QWORD *)(v4->mApplyerArry.mPtr + (v59 << 32 >> 29)) = v58;
            ++v4->mApplyerArry.mLength;
        }
        v60 = off_4159510;
        v4->mLoadResourceEnvMapJob.field_18 = *off_41586A0;
        v4->mLoadResourceEnvMapJob.field_20 = (__int64)v60;
        v4->mLoadResourceEnvMapJob.field_28 = 0LL;
        v61 = Lp::Utl::getCurScene(v58);
        Lp::Sys::SceneGeneralThread::add(*(_QWORD *)(v61 + 240), &v4->mLoadResourceEnvMapJob, 2LL, 0LL);
        *(_BYTE *)(*(_QWORD *)(v4->mModelScene + 23968) + 424LL) = 1;
        if ( v63 != (sead::Heap *)((char *)&xmmword_0 + 1) )
            sead::HeapMgr::setCurrentHeap_((sead::HeapMgr *)*off_4156B70, v63);
        return Cmn::ProfilerScope::~ProfilerScope((Cmn::ProfilerScope *)&v68);
        }*/
        //u64 __vftable;
        gsys::ModelScene *mModelScene;
        _BYTE byte10;
        u64 mDbgMapSort; // DbgSetting::MapSort *
        char field_20;
        char field_24;
        u64 field_28;
        char field_30;
        sead::PtrArrayImpl mApplyerSubstanceArry;
        sead::PtrArrayImpl mApplyerShopArry;
        sead::PtrArrayImpl mApplyerGameArry0;
        sead::PtrArrayImpl mApplyerGameArry1;
        sead::PtrArrayImpl mApplyerGameArry2;
        _BYTE gap88[16];
        sead::PtrArrayImpl mApplyerArry;
        u64 qwordA8;
        u64 qwordB0;
        u32 dwordB8;
        u32 dwordBC;
        u64 qwordC0;
        u64 qwordC8;
        u32 dwordD0;
        Lp::Sys::WorkerJobBase mLoadResourceEnvMapJob;
    };
    class GfxMgr :  sead::IDisposer {
        public:
        class CreateArg{
            public:
            CreateArg(Lp::Sys::Scene *);
            _BYTE _0[0x98];
        };
        GfxMgr();
        gsys::Model *createModel(char* mModelName, sead::Heap *heap){
            Lp::Sys::ModelArc modelArc = *(new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create(mModelName), heap, 0, NULL, NULL));
            Lp::Utl::ModelCreateArg createArg = (*(new Lp::Utl::ModelCreateArg));
            gsys::Model *mModel = Cmn::GfxUtl::createModel(Cmn::Def::Team::Alpha, sead::SafeStringBase<char>::create("Field"), modelArc, createArg, heap);
            mModel->bind(this->modelScene);
            return mModel;
        };
        void load(const Cmn::GfxMgr::CreateArg&); // 20
        void postLoad(); // 28
        void enter(); // 30
        void reset(){}; // 38
        void exit(); // 40
        void destroy(); // 48
        void calcFrame();// 50
        void calcView(); // 58
        void preActorCalc(); // 60
        void postActorCalcDraw(agl::DrawContext *); // 68
        Cmn::CameraBase *getCurr3DCamera(); // 70
        int getLyrIdx_3D_Main() const; // 78;
        int getLyrIdx_2D_Main() const; // 80;
        void setEnableScreenshotMode(bool);
        void setAlternateDrawMode(bool);
        // void applyEnvSet_();
        _BYTE _20[0x10];
        gsys::ModelScene *modelScene; //gsys::ModelScene *
        sead::BufferedSafeStringBase<char> qword38;
        _BYTE gap50[44];
        sead::SafeStringBase<char> mEnvSetName;
        _DWORD dword80;
        _BYTE gap84[36];
        unsigned int hour;
        u32 _AC;
        u64 _B0;
        u64 _B8;
        u64* gpuPerfController; //Cmn::GPUPerfController *
        u64 qwordC8;
        u64* debugTexturePage; //agl::utl::DebugTexturePage *
        _BYTE _D8[0x230];
    };
};