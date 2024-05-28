#pragma once

#include "Cmn/GfxUtl.h"
namespace Cmn{
    class PlayerHairBodyAnimSlot{
        public:
        PlayerHairBodyAnimSlot(gsys::ModelAnimation *,int);
        virtual void getCurrentAnimKey() const;
        virtual void getCurrentFrame() const;
        virtual void getCurrentAnimNumFrame() const;
        virtual void getCurrentAnimBlendRate() const;
        gsys::ModelAnimation *mModelAnimation;
        int _10;
        int _14;
    };
    class PlayerHairAnimController{
        public:
        class CreateArg{
            public:
            CreateArg(){
                mModel = NULL;
                _8 = -1;
                mPlayerHairBodyAnimSlot = NULL;
                mModelType = 0xFFFFFFFF;
                mHairType = 0;
            };
            gsys::Model* mModel;
            u32 _8;
            Cmn::PlayerHairBodyAnimSlot *mPlayerHairBodyAnimSlot;
            u32 mModelType;
            u32 mHairType;
        };
        void reset();
        void calc();
        void callbackBeforeModelUpdateWorldMatrix();
        static Cmn::PlayerHairAnimController *create(Cmn::PlayerHairAnimController::CreateArg const&);
        _BYTE _0[0x44];
        int mHairId;
        bool mIsReqUpdate;
    };
};