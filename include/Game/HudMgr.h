#pragma once

#include "types.h"
#include "Cmn/CameraBase.h"
#include "sead/vector.h"

namespace Game {
    class MiniMapCamera : public Cmn::CameraBase{
        public:
        sead::Vector3<float> calcTargetPos_() const;
        sead::Vector3<float> calcAt_();

        _BYTE _178[0x1D0 - 0x178];
        sead::Vector3<float> mMiniMapAt;
    };
    class MiniMap{
        public:
        _BYTE _0[0x368];
        Game::MiniMapCamera *mMiniMapCamera;
        _BYTE _370[0xE98 - 0x370];
        sead::Vector2<float> mCursorPos;
    };
    class HudMgr{
        public:
		bool isShowMiniMap() const;
        _BYTE _0[0x358];
        Game::MiniMap *mMiniMap;
    };
};