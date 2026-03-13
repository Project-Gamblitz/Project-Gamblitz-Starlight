#pragma once

#include "types.h"
#include "Cmn/Def/PlayerModelType.h"
#include "sead/heap.h"

namespace Cmn {
    class CustomizeSeqBase;
    class CustomizePlayerBase{
        public:
        _BYTE _0[0x478];
        sead::Heap *mGearHeap;
        _BYTE _480[0x6480 - 0x480];
        _BYTE _6480[0x6490 - 0x6480];
        Cmn::Def::PlayerModelType mModelType;
        _BYTE _6494[0x64B0 - 0x6494];
        Cmn::CustomizeSeqBase *mCustomizeSeqBase;
    };
	class CustomizePlayer : public Cmn::CustomizePlayerBase {
		public:
	};
};
namespace Lobby {
    class Player : public Cmn::CustomizePlayerBase {
        public:
    };
};