#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/ComponentBase.h"
#include "Cmn/ComponentHolder.h"
#include "gsys/model.h"

namespace Game{
    class Bullet;
namespace Cmp{
    class BulletKnockbackableParams;
    class BulletKnockbackable : public Cmn::ComponentBase{
        public:
        BulletKnockbackable(Cmn::ComponentHolder *, const BulletKnockbackableParams *);
        void onInteractColBullet(Game::Bullet *bullet, sead::Vector3<float> const&, bool);
        void updatePos(sead::Vector3<float> *);
        sead::Vector3<float> mVelocity;
        _BYTE unk34[0x40 - 0x34];
    };
};
};