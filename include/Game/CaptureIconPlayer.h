#pragma once
#include "types.h"
#include "Cmn/Actor.h"
#include "Lp/Utl.h"

namespace Game{
    class CaptureIconPlayer : public Cmn::Actor{
        public:
        static Lp::Sys::ActorClassIterNodeBase* getClassIterNodeStatic();
        Game::CaptureIconPlayer *getCaptureIconPlayer(int) const;
        u64 _348;
        u64 _350;
        Lp::Utl::TextureHolder *textureHolder;
    };
    class CaptureIconMgr{
        public:
        Game::CaptureIconPlayer *getCaptureIconPlayer(int) const;
    };
};