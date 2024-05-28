#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Cmn/ComponentBase.h"
#include "Cmn/ComponentHolder.h"
#include "Cmn/KDUtl/AttT.h"

namespace Game{
    class ModelCB;
namespace Cmp{
    class Rollable : public Cmn::ComponentBase{
        public:
        Rollable(Cmn::ComponentHolder *,Cmn::KDUtl::AttT const*,float,Game::ModelCB *);
        _BYTE _28[0xD8 - sizeof(Cmn::ComponentBase)];
    };
};
};