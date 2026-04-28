#pragma once

#include "types.h"
#include "Cmn/Def/Team.h"
#include "sead/vector.h"

namespace Cmn { namespace KDGndCol { namespace HitInfo {
    enum PaintType {};
}}}

namespace Game {
    enum PaintTexType {
        Unknown = 0
    };
    class ObjPaintIndex{
        public:
        u32 *mIndex;
    };
    // Opaque — accessed through known byte offsets from sInstance:
    //   +0xE9B8           : embedded ObjPaintMgr
    //   +0xE9B8 + 0x18    : u32 registered-paint-object count
    //   +0xE9B8 + 0x20    : void** array of paint-object pointers (one per paintIdx)
    class PaintMgr {
    public:
        static PaintMgr* sInstance;
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
		// 9-arg user-facing wrapper. Computes sphere radius = max(size.x,size.y)/2
		// internally; the trailing float is the threshold (defaults to sphere
		// radius when negative). Use this when you don't need to tune sphere
		// reach independently of texture size.
		static void requestColAndPaint(sead::Vector3<float> const&, sead::Vector2<float> const&, sead::Vector3<float> const&, Game::PaintTexType, Cmn::Def::Team, sead::Vector3<float> const&, bool, int, float);
		// 10-arg internal form. Sphere radius (a2) and threshold (last arg) are
		// passed independently — required when you want the collision sphere to
		// reach further than the texture stamp size (e.g. tall walls inside a
		// wide-but-short paint cylinder).
		static void requestColAndPaint(sead::Vector3<float> const& pos, float sphereRadius, sead::Vector2<float> const& size, sead::Vector3<float> const& vel, Game::PaintTexType texType, Cmn::Def::Team team, sead::Vector3<float> const& normal, bool overwrite, int playerIdx, float threshold);
		static void requestIndependentHeightRangePaint(
			unsigned int frame,
			sead::Vector2<float> const& heightRange,
			sead::Vector3<float> const& pos,
			sead::Vector3<float> const& upAxis,
			sead::Vector2<float> const& extent,
			sead::Vector2<float> const& paintDir,
			Cmn::Def::Team team,
			Game::ObjPaintIndex paintIdx,
			Game::PaintTexType texType,
			Cmn::KDGndCol::HitInfo::PaintType paintType);
    };
};