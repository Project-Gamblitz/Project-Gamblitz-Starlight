#pragma once

#include "types.h"

namespace Game
{
    namespace Coop
    {
        class Wave
        {
        public:
            _DWORD weather;
            _DWORD tide;
            _DWORD seed2;
            _DWORD gap0xC;
            _DWORD event;
        };

        class Setting : public Cmn::Actor, public sead::IDisposer
        {
        public:
            enum class WeatherChangePhase{
                Sunny = 0,
                Cloudy = 1,
                Foggy = 2,
            };
            void startChangeWaterLevel(int);
            void startChangeWeather(Game::Coop::Setting::WeatherChangePhase, int);

            _BYTE gap368[16];
            _QWORD qword378;
            _BYTE gap380[1144];
            _QWORD qword7F8;
            _BYTE gap800[568];
            _QWORD qwordA38;
            _BYTE gapA40[1864];
            _QWORD qword1188;
            _BYTE gap1190[40];
            _QWORD qword11B8;
            _BYTE gap11C0[96];
            _QWORD qword1220;
            _BYTE gap1228[96];
            _QWORD qword1288;
            _BYTE gap1290[40];
            _QWORD qword12B8;
            _BYTE gap12C0[40];
            _QWORD qword12E8;
            _BYTE gap12F0[96];
            _QWORD qword1350;
            _BYTE gap1358[40];
            _QWORD qword1380;
            _BYTE gap1388[40];
            _QWORD qword13B0;
            _BYTE gap13B8[40];
            _QWORD qword13E0;
            _BYTE gap13E8[40];
            _QWORD qword1410;
            _BYTE gap1418[96];
            _QWORD qword1478;
            _BYTE gap1480[96];
            _DWORD mDifficulty;
            Game::Coop::Wave present;
            Game::Coop::Wave preview;
            Game::Coop::Wave mWave[3];
        };

    }; // namespace Coop
};