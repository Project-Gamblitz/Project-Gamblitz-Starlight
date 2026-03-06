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
        void setVisible(bool);
        void setLayerVisible(bool);
        void fadeAllEffect();
        _BYTE _0[0x368];
        Game::MiniMapCamera *mMiniMapCamera;
        float mOrthoHalfW;  // 0x370 - set by setOrthoTBLRScale
        float mOrthoHalfH;  // 0x374
        _BYTE _378[0xE98 - 0x378];
        sead::Vector2<float> mCursorPos; // 0xE98 - final cursor position in canvas coords
        sead::Vector2<float> mPlayerCanvasCenter; // 0xEA0 - player's position on the minimap canvas
    };
    class HudMgr{
        public:
		bool isShowMiniMap() const;
        _BYTE _0[0x358];
        Game::MiniMap *mMiniMap;
    };
};