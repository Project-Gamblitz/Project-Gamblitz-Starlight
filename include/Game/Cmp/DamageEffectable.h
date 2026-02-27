#pragma once

#include "types.h"
#include "Cmn/ComponentBase.h"

namespace Game {
namespace Cmp {
    class DamageEffectable : public Cmn::ComponentBase {
    public:
        _BYTE unk28[0xD0 - 0x28];
        int mDamageFrame;
        int mEffectFrame;
        _BYTE unkD8[0xE0 - 0xD8];
        u8 mIsDamaged;
        _BYTE unkE1[0xF8 - 0xE1];
    };

    class Sinkable : public Cmn::ComponentBase {
    public:
        _BYTE unk28[0xD0 - 0x28];
        int mDamageFrame;
        int mEffectFrame;
        _BYTE unkD8[0xE0 - 0xD8];
        u8 mIsDamaged;
        _BYTE unkE1[0xF8 - 0xE1];
    };
};
};
