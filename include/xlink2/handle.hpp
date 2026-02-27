#pragma once

#include "types.h"
#include "sead/string.h"

namespace xlink2 {

    struct Event;

    struct Handle {
          xlink2::Event* mEvent;
          uint mEventId;
    };

      class PropertyDefinition {
            public:
            virtual ~PropertyDefinition();
            sead::FixedSafeString<64> mName;
            int mType;
            bool mIsGlobal;
            float mRangeF[2];
      };
}