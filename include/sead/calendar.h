/**
 * @file calendar.h
 * @brief Classes for dates and times.
 */

#pragma once

#include "string.h"
#include "types.h"

namespace sead
{
    class CalendarTime
    {
    public:
        class Year
        {
        public:
            void setValue(u32 year);

            u32 mYear; // _0
        };

        class Month
        {
        public:
            Month(){
                mMonth = 0;
            };
            Month(u32 month);

            void setValueOneOrigin(u32 month);
            void addSelf(u32 month);
            void subSelf(u32 month);
            u32 sub(sead::CalendarTime::Month) const;
            sead::SafeStringBase<char>* makeStringOneOrigin(u32 month);
            u32 makeFromValueOneOrigin(u32 month);

            u32 mMonth; // _0
        };

        class Day
        {
        public:
            Day(){
                mDay = 0;
            };
            void setValue(u32 day);

            u32 mDay; // _0
        };

        class Hour
        {
        public:
            Hour(){
                mHour = 0;
            };
            void setValue(u32 hour);

            u32 mHour; // _0
        };

        class Minute
        {
        public:
            Minute(){
                mMinute = 0;
            };
            void setValue(u32 minute);

            u32 mMinute; // _0
        };

        class Second
        {
        public:
            Second(){
                mSecond = 0;
            };
            void setValue(u32 second);

            u32 mSecond; // _0
        };

        class Date
        {
        public:
            Date() {};
            Date(sead::CalendarTime::Year const &, sead::CalendarTime::Month const &, sead::CalendarTime::Day const &);
        
            sead::CalendarTime::Year mYear; // _0
            sead::CalendarTime::Month mMonth; // _4
            sead::CalendarTime::Day mDay; // _8
            u32 Date_xC;
        };

        class Time
        {
        public:
            Time(){};
            Time(sead::CalendarTime::Hour const &, sead::CalendarTime::Minute const &, sead::CalendarTime::Second const &);
        
            sead::CalendarTime::Hour mHour; // _0
            sead::CalendarTime::Minute mMinute; // _4
            sead::CalendarTime::Second mSecond; // _8
        };
        static sead::CalendarTime::Year cDefaultYear;
        static sead::CalendarTime::Month cDefaultMonth;
        static sead::CalendarTime::Day cDefaultDay;
        static sead::CalendarTime::Hour cDefaultHour;
        static sead::CalendarTime::Minute cDefaultMinute;
        static sead::CalendarTime::Second cDefaultSecond;
        CalendarTime(){
        };
        CalendarTime(sead::CalendarTime::Date const &, sead::CalendarTime::Time const &);
        CalendarTime(sead::CalendarTime::Year const&,sead::CalendarTime::Month const&,sead::CalendarTime::Day const&,sead::CalendarTime::Hour const&,sead::CalendarTime::Minute const&,sead::CalendarTime::Second const&);
        sead::CalendarTime::Date mDate;
        sead::CalendarTime::Time mTime;
    };
};