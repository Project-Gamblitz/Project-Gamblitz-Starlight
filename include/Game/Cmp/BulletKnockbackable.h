#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/ComponentBase.h"
#include "Cmn/ComponentHolder.h"
#include "gsys/model.h"

namespace Game{
namespace Cmp{
    class BulletKnockbackableParams{
        public:

    };
    class BulletKnockbackable : public Cmn::ComponentBase{
        public:
        BulletKnockbackable(Cmn::ComponentHolder *, const BulletKnockbackableParams *);
        _BYTE _28[0x18];
    };
};
};