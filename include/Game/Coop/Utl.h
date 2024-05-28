#pragma once

#include "types.h"
#include "Game/Player/Player.h"

namespace Game
{
    namespace Coop
    {
        class Utl
        {
        public:
            static bool IsControlledPlayer(Game::Player const &);
            static u64 GetWaterLevel();
            static u64 GetWaterLevelDiff();
            static u64 GetWeather();
            static u64 GetDifficulty();
            static u64 GetRoundNum();
            static u64 GetEventType();
            static u64 GetGameFrame();
            static u64 GetGameRandomSeed();
            static u64 GetGameResult();
            static u64 GetRoundIdx();
            static u64 GetRoundTotalFrame();
            static u64 CountActivePlayer();
            static u64 IsSpectatorStation();
            static u64 GetControlledPlayer();
            static bool IsControlledPlayer(Game::Player *);
            static bool IsRoundPlaying();
            /* ... */
        };
    }; // namespace Coop
};     // namespace Game