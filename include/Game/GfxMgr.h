#pragma once

#include "types.h"
#include "Cmn/GfxMgr.h"
#include "Game/GfxEnvChanger.h"
#include "Game/FieldEnvSet.h"

namespace Game {
    class GfxMgr : public Cmn::GfxMgr {
        public:
            void applyEnvSet_();
            void createVRModel_();
            int getInkType() const;
        _BYTE _318[0x188];
        gsys::Model *mVRModel;
    };
};