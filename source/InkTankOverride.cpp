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
    {43,  2},  // Shooter_First_00  -> Tnk_SimpleG
    {44,  2},  // Shooter_First_01  -> Tnk_SimpleG
    {45,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {48,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {49,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {75,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {98,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {203,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {204,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {205,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {1013,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {1014,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {1015,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {1017,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {1025,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {2013,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {2014,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {2015,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {2017,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {2023,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {2024,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {2025,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {2032,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {2033,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {2042,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {2043,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {4003,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {4004,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {4005,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {4013,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {4014,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {4015,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {94,  2},  // Shooter_First_02  -> Tnk_SimpleG
    {95,  2},  // Shooter_First_02  -> Tnk_SimpleG
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
