#pragma once

#include "types.h"
#include "sead/vector.h"
#include "Lp/Sys/ctrl.h"

namespace Cmn {
    class PlayerCtrlData200Hz{
        public:
        void clear(bool);
        Lp::Sys::Ctrl *lpController;
    };
    class PlayerCtrl {
        public:
        static Cmn::PlayerCtrl* sInstance;

        sead::Vector3<float>* getAngleVel(void) const;
        sead::Vector3<float>* getPostureX(void) const;
        sead::Vector3<float>* getPostureY(void) const;
        sead::Vector3<float>* getPostureZ(void) const;
        sead::Vector2<float>* getRightStick(void) const;
        bool isHold(unsigned long) const;
        bool isHoldAllAndTrigAny(unsigned long) const;
        bool isHoldContinue(unsigned long, int) const;
        bool isRelease(unsigned long) const;
        bool isTrig(unsigned long) const;
        bool isTrigWithRepeat(unsigned long) const;
        bool isTrigWithRepeat(unsigned long, int, int) const;
        void calc();

        Lp::Sys::Ctrl *lpController;
        _BYTE _8[0x120];
        Cmn::PlayerCtrlData200Hz *mPlayerCtrlData200Hz;
    };
};