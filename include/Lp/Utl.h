#pragma once

#include "types.h"
#include "Sys/ctrl.h"
#include "sead/color.h"
#include "sead/camera.h"
#include "agl/lyr/layer.h"
#include "agl/lyr/renderinfo.h"
#include "gsys/layer.h"
#include "sead/vector.h"
#include "gsys/model.h"
#include "agl/TextureData.h"

namespace sead{
    class DirectResource{
        public:
    };
    class ArchiveRes{
        public:
        class FileInfo{
        public:
        };
    };
};
namespace Lp
{
    namespace Sys{
        class Scene;
    };
    class Utl {
        public:
        class Matching{
            public:
            static bool isFixedPlayerID();
            static bool isSessionMaster();
        };
        class ModelCreateArg : public gsys::Model::CreateArg{
          public:
          ModelCreateArg(){
              this->_4 = mDefaultViewMax;
              this->_18 = mDefaultIsCreateDecalAOObject;
          };
          static int mDefaultViewMax;
          static int mDefaultIsCreateDecalAOObject;
        };
        class AnimCreateArg : public gsys::ModelAnimation::CreateArg{
            public:
            AnimCreateArg(){

            };
        };
        class DebugPanel{
            public:
            using debugPanelCallFunc = void (***)(u64 idk, Lp::Utl::DebugPanel *panel);
            void calc();
            void calcDrawDbg();
            _BYTE _0[0x522A0];
            debugPanelCallFunc callFunc;
        };
        class ShapeSphere{
        };
        class ShapeCapsule{
        };
        class CollisionResult{
        };
        static void *getBinFromArc(sead::DirectResource const*,sead::SafeStringBase<char> const&,sead::ArchiveRes::FileInfo *);
        static bool getSimpleTouchPos(sead::Vector2<float> *);
        static sead::Heap *getSceneHeap();
        static Lp::Sys::Scene *getCurScene();
        static gsys::Layer3D* getGfxLayer3D(int);
        static agl::lyr::Layer* getGfxLayer(int);
        static Sys::Ctrl* getCtrl(int);
		static void reqChangeScene(sead::SafeStringBase<char> const&, char const*);
        static void reqReconstructScene(char const*);
        static sead::Color4f hsb2rgb(sead::Vector3<float> const&);
        static sead::Vector3<float> rgb2hsb(sead::Color4f const&);
        static int getCurSceneId();
        static char* getCurSceneName();
        static bool isSessionOfflineEmulation();
        static sead::LookAtCamera *getLookAtCamFromGfxLayer(gsys::Layer *);
        static bool checkSphereCapsule(Lp::Utl::ShapeSphere const&,Lp::Utl::ShapeCapsule const&,Lp::Utl::CollisionResult *);
        static bool convertWorldToScreen(sead::Vector2<float> *,sead::Vector3<float> const&,agl::lyr::RenderInfo const&);
        static bool convertWorldToScreen(sead::Vector2<float> *,sead::Vector3<float> const&,agl::lyr::Layer const*,sead::Camera const*);
        static bool convertWorldToScreen(sead::Vector2<float> *,sead::Vector3<float> const&,sead::Camera const*,sead::Projection const*,sead::Viewport const*);
        static agl::TextureData *decodeJPEGAndCreateTexture(void *,ulong,sead::Heap *,sead::Heap *);
        class DebugMenu{
            public:
            static Lp::Utl::DebugMenu *sInstance;
            static void createInstance(sead::Heap *);
            void calc();
            void draw(agl::lyr::RenderInfo const&) const;
        };
        class IDebugMenu : public Lp::Utl::DebugMenu{
            public:
            void createDebugMenuBool(const char *, bool *, int);
        };
        class ByamlIter {
            public:
            ByamlIter(uchar const*);
            bool isValid() const;
            void tryGetIntByKey(int *,char const*) const;
            void tryGetStringByKey(char const**,char const*) const;
        };
        class ByamlWriter {

        };
         class YamlWriter {
            
        };
        class StateMachine{
            public:
            void changeState(int);
            void executeState();
            void registStateName(int stateId, sead::SafeStringBase<char> const&stateName);
            void initialize(int stateNum,int unk,sead::Heap *heap);
            _QWORD __vftable;
            _QWORD field_8;
            s32 mStateId;
            s32 mCurStateCounter;
            s32 mPrevIndex;
            s32 mPrevStateCounter;
            s32 mFirstIndex;
            s32 mFirstStateCounter;
            bool field_28;
            bool field_29;
            template<typename T>
            class Delegate
            {
                public:
                Delegate(T *arg, void (T::*enterCb)(), void (T::*execCb)(), void (T::*exitCb)(int)){
                    mArg = arg;
                    mEnterCB = enterCb;
                    mExecCB = execCb;
                    mExitCB = exitCb;
                };
                template<typename K>
                Delegate<T>& operator=(Delegate<K> const &from){
                    memcpy(this, from, sizeof(Delegate<T>));
                    return *this;
                };
                virtual void enter(){
                    (mArg->*mEnterCB)();
                };
                virtual void exec(){
                    (mArg->*mExecCB)();
                };
                virtual void exit(int res){
                    (mArg->*mExitCB)(res);
                };

                T *mArg;
                void (T::*mEnterCB)();
                void (T::*mExecCB)();
                void (T::*mExitCB)(int res);
            };
            class DelegateBase{
                public:
                template<typename T>
                DelegateBase& operator=(Delegate<T> const &from){
                    memcpy(this, &from, sizeof(DelegateBase));
                    return *this;
                };
                _BYTE _0[sizeof(Delegate<Lp::Sys::Actor>)];
            };
            sead::Buffer<Lp::Utl::StateMachine::DelegateBase> mStateBuffer;
            sead::Buffer<sead::SafeStringBase<char>> mStateNameBuffer;
        };
        class TextureHolder{
            public:
            _BYTE _0[0x200];
            agl::TextureData mTextureData;
        };
        class PlayReportWrapper{
            public:
            void addStringUTF8(sead::SafeStringBase<char> const&, sead::SafeStringBase<char> const&);
        };
    };
};
