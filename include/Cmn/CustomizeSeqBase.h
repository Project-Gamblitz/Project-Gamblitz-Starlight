#pragma once

#include "types.h"
#include "Cmn/CustomizePlayer.h"
#include "sead/heap.h"

namespace Cmn {
	class CustomizeSeqBase {
		public:
        _BYTE _0[0x3D8];
        Cmn::CustomizePlayer *mRivalCustomizePlayer; // originally GearHeap (relocated to 0x470)
        sead::Heap *mGearHeapForEachModel;
        _BYTE _3E8[0x470 - 0x3E8];
        sead::Heap *mGearHeap;
	};
};