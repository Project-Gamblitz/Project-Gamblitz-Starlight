#include "types.h"
#include "Cmn/StaticMem.h"
#include "sead/random.h"
#include "flexlion/ProcessMemory.hpp"
#include "flexlion/Utils.hpp"

static inline u16 getPrivateOptionFlag() {
    uintptr_t staticMem = *(uintptr_t *)ProcessMemory::MainAddr(0x2D590F0);
    if (!staticMem) return 0;
    return *(u16 *)(staticMem + 0x3E8);
}

static inline bool isPrivateOptionActive(u16 bit) {
    return (getPrivateOptionFlag() & bit) != 0;
}

typedef void (*InitFlagFn)(uintptr_t ctrl, u32 flagVal);
typedef void (*SetInputFn)(uintptr_t ctrl, u32 idx, bool enable);
typedef void (*ResetFn)   (uintptr_t ctrl, bool a, bool b);
typedef void (*SetNextFn) (uintptr_t target, u64 dir, uintptr_t next);

void onExeCallbackBtnEventHook(uintptr_t _this, uintptr_t arg1) {
    if (!_this || !arg1) return;
    if (*(u32*)arg1 != 5) return;

    uintptr_t* btnArr = *(uintptr_t**)(_this + 0x4E8);
    uintptr_t  ctrl   = *(uintptr_t*) (arg1  + 0x18);
    u32        count  = *(u32*)       (_this + 0x4E0);

    if (!btnArr) return;

    if (ctrl == btnArr[0]) {
        *(u16*)(_this + 0x500) ^= 1u;
        ctrl = *(uintptr_t*)(arg1 + 0x18);
    }
    uintptr_t bX = (count > 1) ? btnArr[1] : btnArr[0];
    if (ctrl == bX) {
        *(u16*)(_this + 0x500) ^= 2u;
        ctrl = *(uintptr_t*)(arg1 + 0x18);
    }
    bX = (count > 2) ? btnArr[2] : btnArr[0];
    if (ctrl == bX) {
        *(u16*)(_this + 0x500) ^= 4u;
        ctrl = *(uintptr_t*)(arg1 + 0x18);
    }
    bX = (count > 3) ? btnArr[3] : btnArr[0];
    if (ctrl == bX) {
        *(u16*)(_this + 0x500) ^= 8u;
        ctrl = *(uintptr_t*)(arg1 + 0x18);
    }

    if (btnArr[4] && ctrl == btnArr[4]) {
        *(u16*)(_this + 0x500) ^= 0x10u;
    }
}

void onExePostWakeHook(uintptr_t _this) {
    if (!_this) return;

    InitFlagFn initFlag = (InitFlagFn)ProcessMemory::MainAddr(0x3C0BE4);
    if (!initFlag) return;

    u32        count  = *(u32*)       (_this + 0x4E0);
    uintptr_t* btnArr = *(uintptr_t**)(_this + 0x4E8);
    if (!btnArr) return;
    u16        flag   = *(u16*)       (_this + 0x500);

    initFlag(btnArr[0],                      flag & 1u);
    initFlag(btnArr[count > 1 ? 1 : 0], (flag >> 1) & 1u);
    initFlag(btnArr[count > 2 ? 2 : 0], (flag >> 2) & 1u);
    initFlag(btnArr[count > 3 ? 3 : 0], (flag >> 3) & 1u);

    if (count > 4 && btnArr[4])
        initFlag(btnArr[4], (flag >> 4) & 1u);
}

void onExeFadeInInitHook(uintptr_t _this) {
    if (!_this) return;

    typedef bool (*IsInetModeFn)(uintptr_t);
    typedef void (*StopAnimFn)  (uintptr_t);

    IsInetModeFn isInetModeFn = (IsInetModeFn)ProcessMemory::MainAddr(0x19F29E4);
    StopAnimFn   stopAtMax    = (StopAnimFn)  ProcessMemory::MainAddr(0x1998188);
    StopAnimFn   stopAtMin    = (StopAnimFn)  ProcessMemory::MainAddr(0x1998150);
    SetInputFn   setInput     = (SetInputFn)  ProcessMemory::MainAddr(0x19C90B8);
    ResetFn      resetBtn     = (ResetFn)     ProcessMemory::MainAddr(0x19DE190);
    SetNextFn    setNext      = (SetNextFn)   ProcessMemory::MainAddr(0x19BB6A4);

    u32        count      = *(u32*)       (_this + 0x4E0);
    uintptr_t* btnArr     = *(uintptr_t**)(_this + 0x4E8);
    uintptr_t  confirmBtn = *(uintptr_t*) (_this + 0x4F0);
    uintptr_t  animHandle = *(uintptr_t*) (_this + 0x4F8);

    if (!btnArr) return;

    bool inet  = isInetModeFn ? isInetModeFn(_this) : false;
    bool isLAN = !inet;
    if (animHandle) {
        if (inet) { if (stopAtMax) stopAtMax(animHandle); }
        else       { if (stopAtMin) stopAtMin(animHandle); }
    }

    if (setInput) setInput(btnArr[0], 0, true);
    if (resetBtn) resetBtn(btnArr[0], true, false);

    for (int i = 1; i <= 3; i++) {
        uintptr_t b = btnArr[count > (u32)i ? i : 0];
        if (setInput) setInput(b, 0, isLAN);
        if (resetBtn) resetBtn(b, true, false);
    }

    if (count > 4 && btnArr[4] && setInput && resetBtn) {
        setInput(btnArr[4], 0, isLAN);
        resetBtn(btnArr[4], true, false);
    }

    if (confirmBtn && resetBtn) resetBtn(confirmBtn, true, false);

    if (!setNext || !confirmBtn) return;

    uintptr_t tConf = *(uintptr_t*)(confirmBtn + 0x1F0);

    // lan
    if (isLAN) {
        uintptr_t t0 = *(uintptr_t*)(btnArr[0]                      + 0x1F0);
        uintptr_t t1 = *(uintptr_t*)(btnArr[count > 1 ? 1 : 0]      + 0x1F0);
        uintptr_t t2 = *(uintptr_t*)(btnArr[count > 2 ? 2 : 0]      + 0x1F0);
        uintptr_t t3 = *(uintptr_t*)(btnArr[count > 3 ? 3 : 0]      + 0x1F0);

        setNext(t0, 1, t1); setNext(t1, 0, t0);  // btn[0] ↕ btn[1]
        setNext(t1, 1, t2); setNext(t2, 0, t1);  // btn[1] ↕ btn[2]
        setNext(t2, 1, t3); setNext(t3, 0, t2);  // btn[2] ↕ btn[3]

        setNext(t0, 2, tConf);
        setNext(t1, 2, tConf);
        setNext(t2, 2, tConf);
        setNext(tConf, 0, t2);  // confirm → dir0 → btn[2]

        if (count > 4 && btnArr[4]) {
            uintptr_t t4 = *(uintptr_t*)(btnArr[4] + 0x1F0);
            setNext(t3, 1, t4); setNext(t4, 0, t3);  // btn[3] ↕ btn[4]
            setNext(t3, 2, tConf);  // btn[3] RIGHT → confirm
            setNext(t4, 2, tConf);  // btn[4] RIGHT → confirm
            setNext(tConf, 0, t4);  // confirm UP → btn[4]
        } else {
            setNext(t3, 2, tConf);
        }
    }

    uintptr_t tLast;
    if (isLAN) {
        if (count > 4 && btnArr[4]) tLast = *(uintptr_t*)(btnArr[4] + 0x1F0);
        else                        tLast = *(uintptr_t*)(btnArr[count > 3 ? 3 : 0] + 0x1F0);
    } else {
        tLast = *(uintptr_t*)(btnArr[0] + 0x1F0);  // inet: btn[0]
    }
    setNext(tLast, 2, tConf);
    setNext(tConf, 3, tLast);

}

static float (*lerpNOriginal)(float, float) = NULL;

float calcDraw_SuperJumpSign_Hide_AlwaysHook(float weight, float ratio) {
    if (!lerpNOriginal)
        lerpNOriginal = (float(*)(float, float))ProcessMemory::MainAddr(0x159C90);

    if (isPrivateOptionActive(0x10))
        return 1.0f;  // 1.0 - 1.0 = 0.0 → alpha = 0

    return lerpNOriginal(weight, ratio);
}

// would be better if we just do this via eligibility instead of kill count
static float (*calcValueRespawnOriginal)(uintptr_t a1, int paramId, unsigned int a3, int a4) = NULL;

float calcValue_RespawnTime_Save_AlwaysHook(uintptr_t a1, int paramId, unsigned int a3, int a4) {
    if (!calcValueRespawnOriginal)
        calcValueRespawnOriginal = (float(*)(uintptr_t, int, unsigned int, int))ProcessMemory::MainAddr(0x1103538);

    if (!calcValueRespawnOriginal)
        return 0.0f;

    if (isPrivateOptionActive(0x10)) {
        a3 = a3 & ~0x80000000u;
        if (a4 < 2) a4 = 2;
    }

    return calcValueRespawnOriginal(a1, paramId, a3, a4);
}
