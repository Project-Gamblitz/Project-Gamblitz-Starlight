#include "flexlion/ProcessMemory.hpp"
#include "flexlion/Utils.hpp"
#include "Cmn/Actor.h"
#include "Cmn/IPlayerCustomInfo.h"
#include "Lp/Sys/modelarc.h"

// ================================================================
// IDs:
//   0    = Tnk_Simple
//   1    = Tnk_Simple_Shot_First
//   2    = Tnk_SimpleG test
//   3000 = Tnk_BombLauncher
//   3001 = Tnk_WaterCutter
//   3002 = Tnk_Jetpack
//   4000 = Tnk_Coop
//   5000 = Tnk_OctLv0
// ================================================================

struct WeaponTankEntry {
    int weaponId;
    int tankId;
};

static const WeaponTankEntry sTankOverrides[] = {
	// idk if it works yet
    {10,  2},  // Shooter_First_00  -> Tnk_SimpleG
    {11,  2},  // Shooter_First_01  -> Tnk_SimpleG
    {12,  2},  // Shooter_First_02  -> Tnk_SimpleG
};

typedef void* (*GetWeaponEntryFn)(void* table, u64 unused, int weaponId);

int searchForVersusIdHook(void* mushTankInfo, int weaponId) {
    if (weaponId >= 0) {
        for (const auto& entry : sTankOverrides) {
            if (entry.weaponId == weaponId)
                return entry.tankId;
        }
    }

    // og impl
    if (weaponId < 0)
        return 0;
	
    auto getWeaponEntry = (GetWeaponEntryFn)ProcessMemory::MainAddr(0x9768);
    void* instance    = *(void**)ProcessMemory::MainAddr(0x2D56A30);  // *sInstance
    void* weaponTable = *(void**)((u8*)instance + 0x50);
    void* weaponData  = getWeaponEntry(weaponTable, 0, weaponId);

    if (!weaponData)
        return 0;  // Tnk_Simple

    const char* name   = *(const char**)((u8*)weaponData + 0x10);
    const char* prefix = "Shooter_First_";
    const char* n = name;
    const char* p = prefix;
    while (*p && *n == *p) { ++n; ++p; }

    if (*p == '\0')
        return 1;  // Tnk_Simple_Shot_First

    return 0;  // Tnk_Simple
}
