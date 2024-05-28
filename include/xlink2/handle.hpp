#pragma once

#include "types.h"

namespace xlink2 {

    struct Event;

    struct Handle {
          xlink2::Event* mEvent;
          uint mEventId;
    };
}