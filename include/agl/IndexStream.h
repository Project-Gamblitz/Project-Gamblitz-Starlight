#include "types.h"

#pragma once
namespace agl {
  class IndexStream{
    public:
    IndexStream();
    ~IndexStream();
    _BYTE _0[0xC];
    u32 mType;
    _BYTE _10[0x20];
  };
};