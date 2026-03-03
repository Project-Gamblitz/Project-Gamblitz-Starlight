#pragma once

#include "types.h"
#include "xlink2/handle.hpp"

namespace sead { class Heap; }

namespace Lp {
  namespace Sys {
      class XLinkIUser;

      class XLinkMgr{
        public:
        static void calcSystemELink();
      };

      class XLinkCreateArg {
      public:
          XLinkCreateArg(XLinkIUser *iuser, sead::Heap *heap);
          _BYTE _data[0x40]; // 64 bytes - fields set by constructor
          // After construction, offsets 0x30-0x34 can be patched:
          //   [0x30] u8  modelCount
          //   [0x31] u8  actionSlotCount
          //   [0x32] u8  propertyCount
          //   [0x33] bool createELink
          //   [0x34] bool createSLink
      };

      class XLink {
		  public:
          XLink();
          void create(const XLinkCreateArg &arg);
          void setupResource(sead::Heap *heap);
          void loadAndSetupSoundResource(bool);
          void calc();
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
      _BYTE _reserved[0x800]; // size reservation - actual game object may be smaller
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
    void fade(int);
  };
};