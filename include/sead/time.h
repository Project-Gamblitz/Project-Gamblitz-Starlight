#pragma once

#include "types.h"
#include "calendar.h"

namespace sead {
    class DateTime{
        public:
        DateTime(ulong date){
            datetime = date;
        }
        DateTime(){
            datetime = NULL;
        }
        void setNow();
        void getCalendarTime(sead::CalendarTime *) const;
        u64 datetime;
    };
};