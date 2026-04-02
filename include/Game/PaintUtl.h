#pragma once

#include "types.h"
#include "Cmn/Def/Team.h"

namespace Game {
    enum PaintTexType {
        Unknown = 0
    };
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
		void requestPaintImpl_(
			int paintCommandType, int playerIndex, int frame,
			const sead::Vector3<float>* pos, const sead::Vector3<float>* nrm,
			const sead::Vector2<float>* size, const sead::Vector2<float>* paintDir,
			int team, const char* paintType,
			int texType, unsigned int objPaintIdx,
			const int* alpha, bool overwrite, bool a14, bool a15, int a16);
		static void requestColAndPaint(sead::Vector3<float> const&, sead::Vector2<float> const&, sead::Vector3<float> const&, Game::PaintTexType, Cmn::Def::Team, sead::Vector3<float> const&, bool, int, float);
    };
};