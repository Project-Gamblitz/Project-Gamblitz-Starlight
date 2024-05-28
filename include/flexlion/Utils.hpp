#pragma once

#include "types.h"
#include "string.h"
#include "Cmn/Def/Team.h"
#include "Cmn/SceneMgrFader.h"
#include "Lp/Sys/SceneMgr.h"
#include "Game/Player/Player.h"
#include "Game/PlayerMgr.h"
#include "Cmn/StaticMem.h"
#include "Game/MainMgr.h"
#include "Cmn/Def/Mode.h"
#include "Cmn/AppUBOMgr.h"
#include "Cmn/TeamColorMgr.h"
#include "Cmn/GfxSetting.h"
#include "Cmn/PlayerCtrl.h"
#include "math.h"
#include "gsys/model.h"
#include "Cmn/GfxUtl.h"
#include "Scene.h"
#include "Lp/Utl.h"
#include "Game/Enemy.h"
#include "Cmn/PlayerInfoAry.h"
#include "Cmn/SaveData.h"
#include "Game/VictoryClamDirector.h"
#include "Cmn/Singleton.h"
#include "Game/MapObjMgr.h"
#include "sead/time.h"
#include "gsys/SystemTask.h"
#include "Lp/Sys/processswitch.h"
#include "Game/CaptureIconPlayer.h"
#include "Cmn/PfxMgr.h"
#include "Cmn/Mush/MushMapInfo.h"
#include "flexlion/ProcessMemory.hpp"
#include "str.h"
#include "nn/oe.h"
#include "nn/err.h"
#include <cstdarg>
#include "flexlion/Json.hpp"

using rotateMtxFunc = void (*)(sead::Matrix34<float> *, float);
using osCrashHandler = void (*)(nn::os::UserExceptionInfo*);
namespace Starlion{
    enum MenuEntryType{
        DEFAULT, FLOAT, FASTFLOAT, INT, RGB, COLOR, MAP, VMODE, COOPMAP, BTYPE, MAINW, SUBW, SPECIALW, CLASSW, INKT, ANIMID, CAMFILTER, SCENEMODE
    };
};
class Utils
{
    public:
    static bool isPlayerClass(void *ptr);
    static void setStarlightHeap(sead::Heap *heap);
    static sead::Heap *getStarlightHeap();
    static int getPlayerNum();
    static void onLeaveRender();
    static bool isSceneLoaded();
    static bool isSceneJustUnloaded();
    static bool isSceneJustLoaded();
    static Cmn::Def::Team getControlledTeam();
    static Game::Player *getControlledPerformer();
    static Game::Player *getPerformerById(int id);
    static float getDistance(sead::Vector3<float> a1, sead::Vector3<float> a2);
    static sead::Vector3<float> getCameraPosByMtx(sead::Matrix34<float> mtx);
    static sead::Vector3<float> getCameraLookAtByMtx(sead::Matrix34<float> mtx);
    static agl::TextureData *getTextureDataFromModelArc(const char *arcname, const char *texname);
    static rotateMtxFunc rotateMtxX;
    static rotateMtxFunc rotateMtxY;
    static sead::Color4f getCurColor(Cmn::Def::Team team);
    static sead::Vector3<float> getMtxPos(sead::Matrix34<float> *mtx);
    static void setMtxPos(sead::Matrix34<float> *mtx, sead::Vector3<float> pos);
    static bool isValidWeapon(Cmn::Def::WeaponKind kind, int id);
    static void showAppError(const char *format, ...);
    static void showAppError(int errorcode, const char *format, ...);
    static bool isShowMinimap();
    static Game::MiniMap *getMinimap();
    static sead::Vector3<float> calcGroundPos(Game::Player *player, sead::Vector3<float> pos);
    static sead::Vector3<float> getRotFromMtx(sead::Matrix34<float> mtx);
    static int getGearIdByName(Cmn::Def::GearKind kind, char *name);
};