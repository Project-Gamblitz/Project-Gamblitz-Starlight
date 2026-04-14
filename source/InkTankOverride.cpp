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
	{3,  2},  // Shooter_Short_00  -> Tnk_SimpleG
    {4,  2},  // Shooter_Short_01  -> Tnk_SimpleG
    {5,  2},  // Shooter_Short_02  -> Tnk_SimpleG
    {13,  2},  // Shooter_First_00  -> Tnk_SimpleG
    {14,  2},  // Shooter_First_01  -> Tnk_SimpleG
    {22,  2},  // Shooter_Precision_00  -> Tnk_SimpleG
    {23,  2},  // Shooter_Precision_01  -> Tnk_SimpleG
    {33,  2},  // Shooter_Blaze_00  -> Tnk_SimpleG
    {34,  2},  // Shooter_Blaze_01  -> Tnk_SimpleG
    {35,  2},  // Shooter_Blaze_02  -> Tnk_SimpleG
    {43,  2},  // Shooter_Normal_00  -> Tnk_SimpleG
    {44,  2},  // Shooter_Normal_01  -> Tnk_SimpleG
    {45,  2},  // Shooter_Normal_02  -> Tnk_SimpleG
    {48,  2},  // Shooter_Normal_H  -> Tnk_SimpleG
    {49,  2},  // Shooter_Normal_Oct  -> Tnk_SimpleG
    {53,  2},  // Shooter_Gravity_00  -> Tnk_SimpleG
    {54,  2},  // Shooter_Gravity_01  -> Tnk_SimpleG
    {63,  2},  // Shooter_QuickMiddle_00  -> Tnk_SimpleG
    {64,  2},  // Shooter_QuickMiddle_01  -> Tnk_SimpleG
    {65,  2},  // Shooter_QuickMiddle_02  -> Tnk_SimpleG
    {73,  2},  // Shooter_Expert_00  -> Tnk_SimpleG
    {74,  2},  // Shooter_Expert_01  -> Tnk_SimpleG
    {75,  2},  // Shooter_Expert_02  -> Tnk_SimpleG
    {82,  2},  // Shooter_Heavy_00  -> Tnk_SimpleG
    {83,  2},  // Shooter_Heavy_01  -> Tnk_SimpleG
    {92,  2},  // Shooter_Long_00  -> Tnk_SimpleG
    {93,  2},  // Shooter_Long_01  -> Tnk_SimpleG
    {94,  2},  // Shooter_QuickLong_00  -> Tnk_SimpleG
    {95,  2},  // Shooter_QuickLong_01  -> Tnk_SimpleG
    {203,  2},  // Shooter_BlasterShort_00  -> Tnk_SimpleG
    {204,  2},  // Shooter_BlasterShort_01  -> Tnk_SimpleG
    {205,  2},  // Shooter_BlasterShort_02  -> Tnk_SimpleG
    {212,  2},  // Shooter_BlasterMiddle_00  -> Tnk_SimpleG
    {213,  2},  // Shooter_BlasterMiddle_01  -> Tnk_SimpleG
    {223,  2},  // Shooter_BlasterLong_00  -> Tnk_SimpleG
    {224,  2},  // Shooter_BlasterLong_01  -> Tnk_SimpleG
    {225,  2},  // Shooter_BlasterLong_02  -> Tnk_SimpleG
    {243,  2},  // Shooter_BlasterLight_00  -> Tnk_SimpleG
    {244,  2},  // Shooter_BlasterLight_01  -> Tnk_SimpleG
    {252,  2},  // Shooter_BlasterLightLong_00  -> Tnk_SimpleG
    {253,  2},  // Shooter_BlasterLightLong_01  -> Tnk_SimpleG
    {303,  2},  // Shooter_TripleQuick_00  -> Tnk_SimpleG
    {304,  2},  // Shooter_TripleQuick_01  -> Tnk_SimpleG
    {313,  2},  // Shooter_TripleMiddle_00  -> Tnk_SimpleG
    {314,  2},  // Shooter_TripleMiddle_01  -> Tnk_SimpleG
    {315,  2},  // Shooter_TripleMiddle_02  -> Tnk_SimpleG
    {1002,  2},  // Roller_Compact_00  -> Tnk_SimpleG
    {1003,  2},  // Roller_Compact_01  -> Tnk_SimpleG
    {1013,  2},  // Roller_Normal_00  -> Tnk_SimpleG
    {1014,  2},  // Roller_Normal_01  -> Tnk_SimpleG
    {1015,  2},  // Roller_Normal_02  -> Tnk_SimpleG
    {1017,  2},  // Roller_Normal_H  -> Tnk_SimpleG
    {1023,  2},  // Roller_Heavy_00  -> Tnk_SimpleG
    {1024,  2},  // Roller_Heavy_01  -> Tnk_SimpleG
    {1025,  2},  // Roller_Heavy_02  -> Tnk_SimpleG
    {1103,  2},  // Roller_BrushMini_00  -> Tnk_SimpleG
    {1104,  2},  // Roller_BrushMini_01  -> Tnk_SimpleG
    {1105,  2},  // Roller_BrushMini_02  -> Tnk_SimpleG
    {1113,  2},  // Roller_BrushNormal_00  -> Tnk_SimpleG
    {1114,  2},  // Roller_BrushNormal_01  -> Tnk_SimpleG
    {2003,  2},  // Charger_Quick_00  -> Tnk_SimpleG
    {2004,  2},  // Charger_Quick_01  -> Tnk_SimpleG
    {2005,  2},  // Charger_Quick_02  -> Tnk_SimpleG
    {2013,  2},  // Charger_Normal_00  -> Tnk_SimpleG
    {2014,  2},  // Charger_Normal_01  -> Tnk_SimpleG
    {2015,  2},  // Charger_Normal_02  -> Tnk_SimpleG
    {2017,  2},  // Charger_Normal_H  -> Tnk_SimpleG
    {2023,  2},  // Charger_NormalScope_00  -> Tnk_SimpleG
    {2024,  2},  // Charger_NormalScope_01  -> Tnk_SimpleG
    {2025,  2},  // Charger_NormalScope_02  -> Tnk_SimpleG
    {2032,  2},  // Charger_Long_00  -> Tnk_SimpleG
    {2033,  2},  // Charger_Long_01  -> Tnk_SimpleG
    {2042,  2},  // Charger_LongScope_00  -> Tnk_SimpleG
    {2043,  2},  // Charger_LongScope_01  -> Tnk_SimpleG
    {2053,  2},  // Charger_Light_00  -> Tnk_SimpleG
    {2054,  2},  // Charger_Light_01  -> Tnk_SimpleG
    {2055,  2},  // Charger_Light_02  -> Tnk_SimpleG
    {3003,  2},  // Slosher_Strong_00  -> Tnk_SimpleG
    {3004,  2},  // Slosher_Strong_01  -> Tnk_SimpleG
    {3005,  2},  // Slosher_Strong_02  -> Tnk_SimpleG
    {3012,  2},  // Slosher_Diffusion_00  -> Tnk_SimpleG
    {3013,  2},  // Slosher_Diffusion_01  -> Tnk_SimpleG
    {3023,  2},  // Slosher_Launcher_00  -> Tnk_SimpleG
    {3024,  2},  // Slosher_Launcher_01  -> Tnk_SimpleG
    {4003,  2},  // Spinner_Quick_00  -> Tnk_SimpleG
    {4004,  2},  // Spinner_Quick_01  -> Tnk_SimpleG
    {4005,  2},  // Spinner_Quick_02  -> Tnk_SimpleG
    {4013,  2},  // Spinner_Standard_00  -> Tnk_SimpleG
    {4014,  2},  // Spinner_Standard_01  -> Tnk_SimpleG
    {4015,  2},  // Spinner_Standard_02  -> Tnk_SimpleG
    {4022,  2},  // Spinner_Hyper_00  -> Tnk_SimpleG
    {4023,  2},  // Spinner_Hyper_01  -> Tnk_SimpleG
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
