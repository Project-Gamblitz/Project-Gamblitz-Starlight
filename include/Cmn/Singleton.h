#pragma once

#include "types.h"

namespace Cmn
{
  template<typename T>
  class Singleton{
      public:

		static T* GetInstance_() {
      static T* sInstance;
      return sInstance;
    };
  };
};