#include "flexlion/BetaDelivery.hpp"
#include "config.h"

#if IS_BETA

#include "Cmn/SaveData.h"
#include "Cmn/StaticMem.h"
#include "Game/MainMgr.h"
#include "flexlion/FsLogger.hpp"
#include "flexlion/ProcessMemory.hpp"
#include <string.h>

namespace Flexlion {

// ============================================================
// Beta gear list
// ============================================================
static const BetaGearEntry sBetaGear[] = {
    { 1, 29020 },  // Clt_SUP000
};
static const int sBetaGearCount = sizeof(sBetaGear) / sizeof(sBetaGear[0]);

typedef bool (*IsHaveGearFunc)(u64 saveDataCmn, u32 gearKind, u32 gearId, int *outIdx);
static IsHaveGearFunc sIsHaveGear = NULL;

// ============================================================
// Hook: DeliveryBox setup (vtable[4] = sub_7101158688)
// Makes the delivery box appear when beta gear is missing.
// ============================================================
static void (*sDeliverySetupOriginal)(u64 a1) = NULL;

void deliverySetupHook(u64 a1) {
    sDeliverySetupOriginal(a1);
    if (*(u8 *)(a1 + 944))
        return;

    u64 saveDataPtr = *(u64 *)ProcessMemory::MainAddr(0x2D70A00);
    if (!saveDataPtr) return;
    u64 saveDataCmn = *(u64 *)(saveDataPtr + 24);
    if (!saveDataCmn) return;

    for (int i = 0; i < sBetaGearCount; i++) {
        if (!sIsHaveGear(saveDataCmn, sBetaGear[i].kind, sBetaGear[i].id, NULL)) {
            *(u8 *)(a1 + 944) = 1;
            FsLogger::LogFormatDefaultDirect("[BetaDelivery] Box flagged for beta gear\n");
            return;
        }
    }
}

// ============================================================
// Install hooks
// ============================================================
void installDeliveryBoxHook() {
    sIsHaveGear = (IsHaveGearFunc)ProcessMemory::MainAddr(0x307150);

    // Hook DeliveryBox setup vtable
    u64 *setupVtEntry = (u64 *)ProcessMemory::MainAddr(0x2C1B980);
    sDeliverySetupOriginal = (void (*)(u64))(*setupVtEntry);
    *setupVtEntry = (u64)&deliverySetupHook;

    // searchHappi002Hook BL redirects are in 552.slpatch
    // The hook function is defined in main.cpp (always compiled)

    FsLogger::LogFormatDefaultDirect("[BetaDelivery] Hooks installed\n");
}

void pushBetaGearDeliveries() {}

} // namespace Flexlion

#else // IS_BETA == 0

namespace Flexlion {
void pushBetaGearDeliveries() {}
void installDeliveryBoxHook() {}
} // namespace Flexlion

#endif
