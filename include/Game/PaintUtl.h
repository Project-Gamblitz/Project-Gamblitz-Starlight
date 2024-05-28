#pragma once

#include "types.h"
#include "Cmn/Def/Team.h"

namespace Game {
    class ObjPaintIndex{
        public:
        u32 *mIndex;
    };
    class RequestPaintArg{
        public:
        _BYTE _0[0x20];
        Cmn::Def::Team mTeam;
        _BYTE _24[0x14];
        u32 mActionType;
        _BYTE _3C[0x7C]; // idk how big it actually is
    };
    class PaintUtl {
        public:
        static void requestSendAndPaintImpl_(Game::RequestPaintArg const&, bool);
        static void requestAllPaint(unsigned int, Cmn::Def::Team, Game::ObjPaintIndex);
        static void requestAllPaintFloor(unsigned int, Cmn::Def::Team);
        static void requestAllPaintWall(unsigned int, Cmn::Def::Team);
        static void requestClearPaint(Game::ObjPaintIndex, bool);
    };
};