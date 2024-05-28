#pragma once

#include "flexlion/Utils.hpp"

namespace Starlion{
    namespace Sp1WeaponLookup{
        void Initialize();
        void getLookupRefId(short *res, int playerWeaponId, int weaponid);
        const int lookupWeapIds[] = {3, 4, 5, 13, 14, 22, 23, 33, 34, 35, 43, 44, 45, 48, 49, 53, 54, 63, 64, 65, 73, 74, 75, 82, 83, 92, 93, 203, 204, 212, 213, 223, 224, 225, 243, 244, 252, 253, 303, 304, 313, 314, 315, 1002, 1003, 1013, 1014, 1015, 1017, 1023, 1024, 1025, 1103, 1104, 1105, 1113, 1114, 2003, 2004, 2005, 2013, 2014, 2015, 2017, 2023, 2024, 2025, 2032, 2033, 2042, 2043, 2053, 2054, 2055, 3003, 3004, 3005, 3012, 3013, 3023, 3024, 4003, 4004, 4005, 4013, 4014, 4015, 4022, 4023};
        const int lookupWeapNum = sizeof(lookupWeapIds) / sizeof(int);
        static int sp2WeapIds[lookupWeapNum];
    };
};