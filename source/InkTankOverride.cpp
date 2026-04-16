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
	{3,  2},  // Shooter_Short_03  -> Tnk_SimpleG
    {4,  2},  // Shooter_Short_04  -> Tnk_SimpleG
    {5,  2},  // Shooter_Short_05  -> Tnk_SimpleG
    {13,  2},  // Shooter_First_03  -> Tnk_SimpleG
    {14,  2},  // Shooter_First_04  -> Tnk_SimpleG
    {22,  2},  // Shooter_Precision_03  -> Tnk_SimpleG
    {23,  2},  // Shooter_Precision_04  -> Tnk_SimpleG
    {33,  2},  // Shooter_Blaze_03  -> Tnk_SimpleG
    {34,  2},  // Shooter_Blaze_04  -> Tnk_SimpleG
    {35,  2},  // Shooter_Blaze_05  -> Tnk_SimpleG
    {43,  2},  // Shooter_Normal_03  -> Tnk_SimpleG
    {44,  2},  // Shooter_Normal_04  -> Tnk_SimpleG
    {45,  2},  // Shooter_Normal_05  -> Tnk_SimpleG
    {48,  2},  // Shooter_Normal_H_01  -> Tnk_SimpleG
    {49,  3},  // Shooter_Normal_Oct_01  -> Tnk_Rvl00
    {53,  2},  // Shooter_Gravity_03  -> Tnk_SimpleG
    {54,  2},  // Shooter_Gravity_04  -> Tnk_SimpleG
    {63,  2},  // Shooter_QuickMiddle_03  -> Tnk_SimpleG
    {64,  2},  // Shooter_QuickMiddle_04  -> Tnk_SimpleG
    {65,  2},  // Shooter_QuickMiddle_05  -> Tnk_SimpleG
    {73,  2},  // Shooter_Expert_03  -> Tnk_SimpleG
    {74,  2},  // Shooter_Expert_04  -> Tnk_SimpleG
    {75,  2},  // Shooter_Expert_05  -> Tnk_SimpleG
    {82,  2},  // Shooter_Heavy_03  -> Tnk_SimpleG
    {83,  2},  // Shooter_Heavy_04  -> Tnk_SimpleG
    {92,  2},  // Shooter_Long_03  -> Tnk_SimpleG
    {93,  2},  // Shooter_Long_04  -> Tnk_SimpleG
    {94,  2},  // Shooter_QuickLong_03  -> Tnk_SimpleG
    {95,  2},  // Shooter_QuickLong_04  -> Tnk_SimpleG
    {203,  2},  // Shooter_BlasterShort_03  -> Tnk_SimpleG
    {204,  2},  // Shooter_BlasterShort_04  -> Tnk_SimpleG
    {205,  2},  // Shooter_BlasterShort_05  -> Tnk_SimpleG
    {212,  2},  // Shooter_BlasterMiddle_03  -> Tnk_SimpleG
    {213,  2},  // Shooter_BlasterMiddle_04  -> Tnk_SimpleG
    {223,  2},  // Shooter_BlasterLong_03  -> Tnk_SimpleG
    {224,  2},  // Shooter_BlasterLong_04  -> Tnk_SimpleG
    {225,  2},  // Shooter_BlasterLong_05  -> Tnk_SimpleG
    {243,  2},  // Shooter_BlasterLight_03  -> Tnk_SimpleG
    {244,  2},  // Shooter_BlasterLight_04  -> Tnk_SimpleG
    {252,  2},  // Shooter_BlasterLightLong_03  -> Tnk_SimpleG
    {253,  2},  // Shooter_BlasterLightLong_04  -> Tnk_SimpleG
    {303,  2},  // Shooter_TripleQuick_03  -> Tnk_SimpleG
    {304,  2},  // Shooter_TripleQuick_04  -> Tnk_SimpleG
    {313,  2},  // Shooter_TripleMiddle_03  -> Tnk_SimpleG
    {314,  2},  // Shooter_TripleMiddle_04  -> Tnk_SimpleG
    {315,  2},  // Shooter_TripleMiddle_05  -> Tnk_SimpleG
    {1002,  2},  // Roller_Compact_03  -> Tnk_SimpleG
    {1003,  2},  // Roller_Compact_04  -> Tnk_SimpleG
    {1013,  2},  // Roller_Normal_03  -> Tnk_SimpleG
    {1014,  2},  // Roller_Normal_04  -> Tnk_SimpleG
    {1015,  2},  // Roller_Normal_05  -> Tnk_SimpleG
    {1017,  2},  // Roller_Normal_H_01  -> Tnk_SimpleG
    {1023,  2},  // Roller_Heavy_03  -> Tnk_SimpleG
    {1024,  2},  // Roller_Heavy_04  -> Tnk_SimpleG
    {1025,  2},  // Roller_Heavy_05  -> Tnk_SimpleG
    {1103,  2},  // Roller_BrushMini_03  -> Tnk_SimpleG
    {1104,  2},  // Roller_BrushMini_04  -> Tnk_SimpleG
    {1105,  2},  // Roller_BrushMini_05  -> Tnk_SimpleG
    {1113,  2},  // Roller_BrushNormal_03  -> Tnk_SimpleG
    {1114,  2},  // Roller_BrushNormal_04  -> Tnk_SimpleG
    {2003,  2},  // Charger_Quick_03  -> Tnk_SimpleG
    {2004,  2},  // Charger_Quick_04  -> Tnk_SimpleG
    {2005,  2},  // Charger_Quick_05  -> Tnk_SimpleG
    {2013,  2},  // Charger_Normal_03  -> Tnk_SimpleG
    {2014,  2},  // Charger_Normal_04  -> Tnk_SimpleG
    {2015,  2},  // Charger_Normal_05  -> Tnk_SimpleG
    {2017,  2},  // Charger_Normal_H_01  -> Tnk_SimpleG
    {2023,  2},  // Charger_NormalScope_03  -> Tnk_SimpleG
    {2024,  2},  // Charger_NormalScope_04  -> Tnk_SimpleG
    {2025,  2},  // Charger_NormalScope_05  -> Tnk_SimpleG
    {2032,  2},  // Charger_Long_03  -> Tnk_SimpleG
    {2033,  2},  // Charger_Long_04  -> Tnk_SimpleG
    {2042,  2},  // Charger_LongScope_03  -> Tnk_SimpleG
    {2043,  2},  // Charger_LongScope_04  -> Tnk_SimpleG
    {2053,  2},  // Charger_Light_03  -> Tnk_SimpleG
    {2054,  2},  // Charger_Light_04  -> Tnk_SimpleG
    {2055,  2},  // Charger_Light_05  -> Tnk_SimpleG
    {3003,  2},  // Slosher_Strong_03  -> Tnk_SimpleG
    {3004,  2},  // Slosher_Strong_04  -> Tnk_SimpleG
    {3005,  2},  // Slosher_Strong_05  -> Tnk_SimpleG
    {3012,  2},  // Slosher_Diffusion_03  -> Tnk_SimpleG
    {3013,  2},  // Slosher_Diffusion_04  -> Tnk_SimpleG
    {3023,  2},  // Slosher_Launcher_03  -> Tnk_SimpleG
    {3024,  2},  // Slosher_Launcher_04  -> Tnk_SimpleG
    {4003,  2},  // Spinner_Quick_03  -> Tnk_SimpleG
    {4004,  2},  // Spinner_Quick_04  -> Tnk_SimpleG
    {4005,  2},  // Spinner_Quick_05  -> Tnk_SimpleG
    {4013,  2},  // Spinner_Standard_03  -> Tnk_SimpleG
    {4014,  2},  // Spinner_Standard_04  -> Tnk_SimpleG
    {4015,  2},  // Spinner_Standard_05  -> Tnk_SimpleG
    {4022,  2},  // Spinner_Hyper_03  -> Tnk_SimpleG
    {4023,  2},  // Spinner_Hyper_04  -> Tnk_SimpleG
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
