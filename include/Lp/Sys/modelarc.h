#pragma once

#include "types.h"
#include "sead/string.h"
#include "sead/heap.h"

namespace gsys{
    class ModelResource{
        public:
        class CreateArg;
        void countUpAnimationReferenceCounter();
    };
    class ModelShaderArchive;
};
namespace Lp {
  namespace Sys {
      class ModelArc{
          public:
          ModelArc(sead::SafeStringBase<char> const&,sead::Heap *,bool,gsys::ModelResource::CreateArg const*,gsys::ModelShaderArchive *);
          void load(sead::SafeStringBase<char> const&, sead::Heap *, sead::Heap *, bool, gsys::ModelResource::CreateArg const*, gsys::ModelShaderArchive *);
          _BYTE _0[0x20];
      };
  };
};