// =============================================================================
// StageSelectPageExpand.hpp — UI capacity expansion for the Shoal stage
// select page (64 → 128 entries) to match the savefile cap extension.
// =============================================================================

#pragma once

#include "types.h"

// Wraps every BL to `sub_7100410AF0` (the makeStageList helper, 5.5.2
// 0x00410AF0). Overrides the page's UI-list / free-list pointers to use
// our 128-slot external buffer BEFORE the helper runs, then forwards the
// call to the vanilla helper. See source/StageSelectPageExpand.cpp.
void* makeStageListHelperHook(void* page);

// Wraps every BL to `sub_71004130D0` at 5.5.2 0x004130D0 (the
// updateStageListForRandomFromOnOffList_ analog). Caps the page count
// to a safe value before the call so the static global random-rotation
// list doesn't overflow when the page has >64 UI entries.
void* updateStageListForRandomHook(void* page);
