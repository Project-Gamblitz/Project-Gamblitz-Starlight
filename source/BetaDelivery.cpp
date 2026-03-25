#include "flexlion/BetaDelivery.hpp"
#include "config.h"

#ifdef IS_BETA

#include "Cmn/SaveData.h"
#include "Cmn/StaticMem.h"
#include "Cmn/Mush/MushDataHolder.h"

namespace Flexlion {

// ============================================================
// Beta gear list — add your custom gear IDs here
// ============================================================
static const BetaGearEntry sBetaGear[] = {
    // { kind, id }
    // kind: 0 = Head, 1 = Clothes, 2 = Shoes
    // Example:
    // { 1, 10001 },  // Beta-exclusive clothing
};
static const int sBetaGearCount = sizeof(sBetaGear) / sizeof(sBetaGear[0]);

// ============================================================
// Ring buffer layout in StaticMem (from IDA analysis):
//   +0xDF8 (3576): pointer to buffer (array of {int kind, int id} pairs)
//   +0xE00 (3584): capacity
//   +0xE04 (3588): read cursor
//   +0xE08 (3592): count
// ============================================================

static bool isPlayerHaveGear(Cmn::SaveDataCmn *saveDataCmn, int kind, int gearId) {
    // Inline isHaveGear: scan the inventory for this gear ID.
    // SaveDataCmn+16 = pData. Inventory starts at pData + 77880 (0x13038).
    // Each GearKind has 512 entries of 48 bytes (HaveGear).
    // kind 0/1/2 offset = 77880 + kind * 24576.
    u64 pData = *(u64 *)((char *)saveDataCmn + 16);
    if (!pData) return false;

    int baseOffset = 77880 + kind * 24576;
    for (int i = 0; i < 512; i++) {
        int storedId = *(int *)(pData + baseOffset + i * 48);
        if (storedId == gearId) return true;
    }
    return false;
}

static void pushToDeliveryQueue(int kind, int id) {
    Cmn::StaticMem *sm = Cmn::StaticMem::sInstance;
    if (!sm) return;

    char *base = (char *)sm;
    int *buf = *(int **)(base + 3576);    // buffer pointer
    int capacity = *(int *)(base + 3584); // capacity
    int cursor = *(int *)(base + 3588);   // read cursor
    int count = *(int *)(base + 3592);    // count

    if (!buf || count >= capacity) return;

    // Write at (cursor + count) % capacity
    int writePos = (cursor + count) % capacity;
    buf[writePos * 2] = kind;
    buf[writePos * 2 + 1] = id;
    *(int *)(base + 3592) = count + 1;
}

void pushBetaGearDeliveries() {
    if (sBetaGearCount == 0) return;

    Cmn::StaticMem *sm = Cmn::StaticMem::sInstance;
    if (!sm) return;

    // Get SaveDataCmn to check ownership
    Cmn::SaveData *saveData = Cmn::GetSaveWrite();
    if (!saveData) return;
    Cmn::SaveDataCmn *saveDataCmn = saveData->pSaveDataCmn;
    if (!saveDataCmn) return;

    // Also verify gear exists in MushDataHolder
    Cmn::MushDataHolder *mushData = Cmn::MushDataHolder::sInstance;

    for (int i = 0; i < sBetaGearCount; i++) {
        int kind = sBetaGear[i].kind;
        int id = sBetaGear[i].id;

        // Validate gear exists (optional, skip if using custom IDs not in MushGearInfo)
        if (mushData) {
            void *gearData = mushData->mMushWeaponInfo->getById(
                (Cmn::Def::WeaponKind)(kind + 3), id); // GearKind maps to WeaponKind offset
            // If gear not in database, skip validation (custom gear)
        }

        // Skip if player already owns this gear
        if (isPlayerHaveGear(saveDataCmn, kind, id)) continue;

        // Push into delivery queue
        pushToDeliveryQueue(kind, id);
    }
}

} // namespace Flexlion

#else // !IS_BETA

namespace Flexlion {
void pushBetaGearDeliveries() {}
} // namespace Flexlion

#endif
