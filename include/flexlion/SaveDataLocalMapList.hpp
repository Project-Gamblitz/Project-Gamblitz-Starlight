// =============================================================================
// SaveDataLocalMapList.hpp — extended Shoal/Private-match map list (128 slots).
// =============================================================================
//
// Hook declarations for the u16-pack treatment of the SaveDataLocal map array.
// See source/SaveDataLocalMapList.cpp for full design notes.
// =============================================================================

#pragma once

#include "types.h"

// Replaces `Cmn::SaveDataLocal::setHavePrivateStage` (5.5.2 sub_710030C4D0)
// at its entry point. Implements the u16-packed 128-slot logic AND runs the
// one-shot u32→u16 migration on first call after the mod is installed.
//
// NOT `extern "C"` — slpatch's genPatch resolves hook symbols via their
// C++ mangled name; matching declaration + definition without extern "C".
__int64 saveDataLocalSetHavePrivateStageHook(void* this_, int mapId, char haveValue);
