#pragma once

#include "types.h"

namespace Cmn
{
    namespace Def
    {
        class Gear
        {
        public:
            Gear();

            void resetExpAndExSkillAll();

            s32 mGearId;
            u32 mUnlockedSkillCount;
            u32 mUnlockedSlotCount;
            s32 mMainSkillId;
            s32 mSubSkillIds[3];
            u32 mGearExp;
        };

        enum class GearKind : int { cShoes, cClothes, cHead, cNone };
        enum class SpecialSkill: int {StartAllUp ,EndAllUp ,MinorityUp ,ComeBack ,SquidMoveSpatter_Reduction ,DeathMarking ,ThermalInk ,Exorcist ,ExSkillDouble ,SuperJumpSign_Hide ,ObjectEffect_Up ,SomersaultLanding ,RespawnRadar};
        enum class BasicSkill: int {SubInk_Save ,MainInk_Save ,InkRecovery_Up ,HumanMove_Up ,SquidMove_Up ,SpecialIncrease_Up ,RespawnSpecialGauge_Save ,SpecialTime_Up ,RespawnTime_Save ,JumpTime_Save ,BombDistance_Up ,OpInkEffect_Reduction ,BombDamage_Reduction ,MarkingTime_Reduction};
    };
};