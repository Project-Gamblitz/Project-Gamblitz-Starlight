#pragma once

#include "types.h"
#include "xlink2/handle.hpp"

namespace Lp {
  namespace Sys {
      class XLinkMgr{
        public:
        static void calcSystemELink();
      };
      class XLink {
		  public:
			xlink2::Handle searchAndEmit(const char *, bool);
      xlink2::Handle searchAndPlay(const char *, bool);
      void setIsActive(bool);
      bool isSleep() const;
      int getLocalPropertyValueS(unsigned int);
      void setLocalPropertyValue(unsigned int, float);
      void killAllEffect();
      void killAllSound();
      ALWAYS_INLINE void searchAndEmitWrap(const char* name, bool hold, xlink2::Handle* out) {
        asm("mov x8, %0" : : "r"(out));
        searchAndEmit(name, hold);
      };
      ALWAYS_INLINE void searchAndPlayWrap(const char* name, bool hold, xlink2::Handle* out) {
        asm("mov x8, %0" : : "r"(out));
        searchAndPlay(name, hold);
      };
    };
  };
};
namespace xlink2{
  class System{
    public:
    void calc();
  };
  class Event{
    public:
    bool calc();
  };
};