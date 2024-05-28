#pragma once

#include "types.h"

namespace enl
{
    class PiaMatchmakeCondition
    {
    public:
        void makeRandomCryptoKey(int MatchmakeCondition,const u64[], int);

        const u64 deword[64] = {0x56CB956F, 0x7B50EEC6, 0x234D1A63, 0x1C691A6B, 0xD2D9C482, 0xCFE21965,
                                0xB32DF99, 0xB32AFE44, 0xB15DA3D7, 0x86588505, 0x4FC8CD8B, 0xC30F864B,
                                0x8D4D3BE, 0xEFDEC6CA, 0x63A1D53F, 0xC545538D, 0x715E27A2, 0x4818A005,
                                0x8C28D9F8, 0xC303EABF, 0xF1D847ED, 0xE837F303, 0xE68981E8, 0x63E2F9BC,
                                0xC320F7E1, 0x5E0B4084, 0x502B2A2D, 0x65D36579, 0xD169E46, 0x65AB445D,
                                0xFDF0678B, 0x26167D3E, 0xFE5025A0, 0x4EB0EA8, 0xC048B044, 0xF858002E,
                                0x6725F7D6, 0xD4086AA8, 0xF216DE10, 0xF1807E6, 0xD3614878, 0x34A2FEE6,
                                0xA69AE3DE, 0xED8518EF, 0x6FCCB7A5, 0x7F8D0E40, 0x9B72BFA8, 0x87C669D4,
                                0x5BF80652, 0x9A71383F, 0xBA3E7A7A, 0x1ABA65A3, 0xA9A16DFF, 0xD07B9E3C,
                                0x11C9BD45, 0xF14A6D81, 0x78516ECD, 0x53445C15, 0xC86E9942, 0x5501D2C9,
                                0xD0D4ECB3, 0x38F5C341, 0xC4A16155, 0x42F1F406};
    };
};