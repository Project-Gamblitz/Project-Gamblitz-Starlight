#pragma once
#include "types.h"

// Hook for `Cmn::TeamColorMgr::load` (5.5.2 0x14221C). Patched in at the
// BL site 0x10D238 (inside the GfxSetting-equivalent setup that builds
// the manager singleton). After original runs, the hook appends 14
// Splatoon 1 leftover ink-color combos to entries[1] (Vss_Regular) and
// entries[2] (Vss_Gachi) via the engine's own .bprm loader. See
// source/InkColorExtension.cpp for the full mechanism.
__int64 teamColorMgrLoadHook(__int64 mgr);

// Wires up the original-function pointer used by the hook. Called once
// from main.cpp::init_starlion before any BL site fires.
extern "C" void inkColorExtensionInit();
