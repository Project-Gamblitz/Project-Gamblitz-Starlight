#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/ComponentBase.h"
#include "Cmn/ComponentHolder.h"

namespace Game{
namespace Cmp{
    class Movable : public Cmn::ComponentBase{
        public:
        Movable(Cmn::ComponentHolder *);
        _BYTE _28[0x50 - sizeof(Cmn::ComponentBase)];
    };
};
};