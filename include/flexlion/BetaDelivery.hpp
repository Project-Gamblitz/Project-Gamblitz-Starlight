#pragma once

#include "config.h"
#include "types.h"
#include "Cmn/Def/Gear.h"

namespace Flexlion {

struct BetaGearEntry {
    int kind; // 0=Head, 1=Clothes, 2=Shoes
    int id;
};

// Push beta-exclusive gear into save data.
// Called during plaza setup. Only delivers gear the player doesn't already own.
void pushBetaGearDeliveries();

// Hook the plaza delivery box setup to show the box for beta gear.
void installDeliveryBoxHook();

} // namespace Flexlion
