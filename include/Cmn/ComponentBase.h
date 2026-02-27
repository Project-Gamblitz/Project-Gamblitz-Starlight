#pragma once

#include "types.h"

namespace Cmn{
    class ComponentBase{
        public:
        virtual ~ComponentBase();
        virtual bool checkDerivedRuntimeTypeInfo(void const*) const;
        virtual void* getRuntimeTypeInfo() const;
        virtual void load();
        virtual void postLoad();
        virtual void enter();
        virtual void postEnter();
        virtual void reset();
        virtual void postReset();
        virtual void onActivate();
        virtual void onSleep();
        virtual void exit();
        virtual void firstCalc();
        virtual void secondCalc();
        virtual void thirdCalc();
        virtual void preFourthCalc();
        virtual void fourthCalc();
        virtual void onEnterCulling();
        virtual void onExitCulling();
        virtual void actorCBBeforeModelApplyAnimation(void*);
        virtual void actorCBBeforeModelUpdateWorldMatrix(void*);
        virtual void actorCBAfterModelUpdateWorldMatrix(void*);
        virtual void onParamsChanged();
        virtual void onXLinkSetupActionSlot(void*, void*);
        virtual void setXLinkLocalPropertyDefinition(void*, void*, int);
        virtual int countXLinkLocalProperty() const;
        virtual int getPriority() const;
        _BYTE _8[0x20];
    };
};