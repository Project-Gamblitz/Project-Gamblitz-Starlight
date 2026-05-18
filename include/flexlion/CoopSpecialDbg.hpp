// =============================================================================
// CoopSpecialDbg.hpp — Salmon Run special-pool shuffle hook.
// =============================================================================
//
// Replaces the BL inside CustomRuleModerator and NormalRuleModerator that
// calls Game::Coop::CommonRuleModerator::setupChangeWeapon_ (5.5.2:
// 0x73110C). Pre-shuffles RuleConfig's special-weapon pool at
// (RuleConfig+0x160) using the SR shift seed (the sead::Random passed in
// W1/X1), then forwards to the vanilla picker. After this, the picker's
// `pool[0..3]` slice holds 4 random distinct entries from the full pool.
//
// See source/CoopSpecialDbg.cpp.
// =============================================================================

#pragma once

#include "types.h"

void coopSpecialShufflerHook(void* moderatorState, void* rng);
