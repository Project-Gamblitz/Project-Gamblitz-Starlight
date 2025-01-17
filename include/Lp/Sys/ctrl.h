#pragma once 

#include "types.h"
#include "sead/vector.h"

namespace Lp
{
  namespace Sys {
      class Ctrl{
         public:
          enum Key {
              KEY_A = 1 << 0,
              KEY_B = 1 << 1,
              KEY_ZL = 1 << 2,
              KEY_Y = 1 << 3,
              KEY_X = 1 << 4,
              KEY_ZR = 1 << 5,
              KEY_RSTICK = 1 << 6,
              KEY_LSTICK = 1 << 7,

              KEY_UNK1 = 1 << 8,
              KEY_MINUS = 1 << 9,
              KEY_PLUS = 1 << 10,
              KEY_PLUS_ALT = 1 << 11,
              KEY_MINUS_ALT = 1 << 12,
              KEY_L = 1 << 13,
              KEY_R = 1 << 14,
              KEY_UNK2 = 1 << 15,

              KEY_DUP = 1 << 16,
              KEY_DDOWN = 1 << 17,
              KEY_DLEFT = 1 << 18,
              KEY_DRIGHT = 1 << 19,
              KEY_LSTICK_UP = 1 << 20,
              KEY_LSTICK_DOWN = 1 << 21,
              KEY_LSTICK_LEFT = 1 << 22,
              KEY_LSTICK_RIGHT = 1 << 23,

              KEY_RSTICK_UP = 1 << 24,
              KEY_RSTICK_DOWN = 1 << 25,
              KEY_RSTICK_LEFT = 1 << 26,
              KEY_RSTICK_RIGHT = 1 << 27,
          };

          bool isHoldContinue(unsigned int, int) const;
          bool isTrigWithRepeat(unsigned int, int, int) const;

          __int32 dword0;
          __int32 dword4;
          __int64 qword8;
          __int64 data;
          _BYTE _18[0x88];
          sead::Vector2<float> leftStick;
          sead::Vector2<float> rightStick;
      };
  };
};
