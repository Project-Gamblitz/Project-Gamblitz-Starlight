#pragma once

#include "types.h"
namespace Game {
    enum TwoShotScene {
    TwoShotScene_Invalid = -2,
    TwoShotScene_None = -1,
    TwoShotScene_Mono,
    TwoShotScene_Sepia,
    TwoShotScene_Osha,
    TwoShotScene_Ao,
    TwoShotScene_Dark,
    TwoShotScene_Romance,
    TwoShotScene_FishEye,
    TwoShotScene_End
    };
    class GfxEnvChanger {
        public:
        static Game::GfxEnvChanger *sInstance;
        _BYTE _0[0xE70];
        int mUpdateId;
        u32 _E74;
        u32 _E78;
        Game::TwoShotScene mTwoShotScene;
    };
};