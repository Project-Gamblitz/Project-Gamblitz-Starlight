#pragma once

#define IS_DUMP_AC 0

#include "GameIncludes.h"
#include "flexlion/Utils.hpp"
#include "flexlion/KingSquidMgr.hpp"
#include "nn/socket.h"
#include "flexlion/S1Inkstrike.h"
#include "Cmn/PlayerCustomUtl.h"
#include "flexlion/Sp1WeaponLookup.hpp"
#include "flexlion/FsLogger.hpp"
#include "Cmn/EffectManualHandle.h"
#include "flexlion/InkstrikeMgr.hpp"
#include "xlink2/handle.hpp"
#include "sead/random.h"
#include "gamblitz/automatch.hpp"
//#include "EffectMgr.hpp"

ushort GetCharKindHook(uintptr_t _this, ushort charKind);
void handleSupershot();
void handlePlayerEffects();
int getSuperShotBurstWaitFrameHook(Game::BulletGachihoko *bullet);
int getSuperShotBurstWarnFrameHook(Game::BulletGachihoko *bullet);
void supershotJumpHook();
void tornadoJumpHook();
//int getBombThrowSpanFrmHook(Game::PlayerInkAction *inkAction, bool a);
u64 specialSetupWithoutModelHook();
void playerModelResourceLoadHook(Game::PlayerModelResource *res, sead::Heap *heap);
gsys::Model *createPlayerModelHook(Cmn::Def::Team team,sead::SafeStringBase<char> const& name,Lp::Sys::ModelArc &arc,Lp::Utl::ModelCreateArg const&createArg,sead::Heap *heap);
gsys::Model *createHumanModelHook(sead::SafeStringBase<char> const& name, Cmn::Def::Team team, Game::PlayerModelResource &res, const Lp::Utl::ModelCreateArg &arg, Lp::Utl::AnimCreateArg const&animarg, Cmn::Def::PlayerModelType modeltype, sead::RingBuffer<int> const&hairinfo);
int *custommgrjptHook();
bool barrierEffectHook(Game::PlayerEffect *peffect, bool isEmit);
int calcAquaBallDamageHook(Game::BulletSpAquaBall *bullet, int armortype, Cmn::Def::Team team, sead::Vector3<float> const& pos);
int calcHokoDamageHook(Game::BulletGachihoko *bullet, int armortype, Cmn::Def::Team team, sead::Vector3<float> const& pos);
int stepPaintTypeHook(Game::PlayerStepPaint *step);
bool fixEffHook(Game::Player *player);
void miniMapCamCalcHook(Game::MiniMapCamera *_this);
void handleBulletCloneEventHook(Game::BulletCloneHandle *cloneHandle, Game::Player *player, Game::BulletCloneEvent *event, int clonefrm);
bool isInInkstrikeCarryHook(Game::Player *player);

void damageReasonHook(Game::VersusBeatenPage *page, Game::DamageReason &reason, Cmn::PlayerInfo const*attackerInfo, int dieFrm, int uiAppearFrm, bool iwannaslep);
void init_starlion();
CURLcode curl_easy_perform_hook(CURL *curl);
void FlyingGearMtxCalc(Cmn::PlayerCustomPart *part, sead::Matrix34<float> *mtx);
void viewerFileHook(sead::SafeStringBase<char> const& name, void* data, unsigned int size);

void playerKingSquidCalcHook(Game::Player *player);
void registKingSquidAnimHook(Game::PlayerAnimCtrlSet *animCtrlSet, int a1,char const*a2,bool a3,bool a4);
void setupKingSquidAnimHook(Game::PlayerAnimCtrlSet *animCtrlSet, Game::PlayerJoint *joint);
void kingSquidAnimSetControllerHook(Game::AnimSetController *anim, gsys::PartialSkeletalAnm const* cool);
const char *bcatDirHook1();
const char *bcatDirHook2();
void bcatHook(int *res);
sead::HeapMgr *npcHeapFix(sead::HeapMgr *a1, sead::Heap *a2);
sead::Heap *npcHeapHook(sead::HeapMgr *a1, sead::Heap *a2);
void playerModelSetupHook(Game::PlayerModel *pmodel);

void curl_perform_hook(int res);
int curl_pause_hook();
float instaCoverHook(float turf);

void performanceImprTestHook();
int performanceImprTestHook2();
void isInLauncherHook(Game::Player *player);
void handleDisplayVersion(nn::oe::DisplayVersion *ver);
void agentThreeHandle();
void agentThreeGearHook(Cmn::MushGearInfo *gearInfo);
bool isWorld00();
int msnArmorHook(Cmn::MushGearInfo *gearInfo, Cmn::Def::GearKind kind, int armorLv);
void startAllMarking_ImplHook(Game::Player *player, int a1);
void markedHook(Game::Player *player, int a1,int a2,Game::Player::MarkingType a3,int a4, unsigned int a5);
xlink2::UserInstanceSLink *startSkill_DeathMarkingHook(Game::Player *player, unsigned int a2, char a3);
void inkstrikeNetHook(u64 *x0, u32 w1, u64 *x2, u64 *x3, u32 w4, u32 w5);
void shotBowHook();
void playerFirstCalcHook(Game::Player *player);
void playerFourthCalcHook(Game::Player *player);
sead::Vector3<float> inkstrikeBombVelHook(Game::PlayerInkAction *inkAction);
void inkstrikeShotHook(Game::BulletSpSuperBall *ball, Game::Player *sender, int senderId, int senderId2, sead::Vector3<float> *startpos, Game::SuperBallShotArg2 *arg2, int poop);
int swimHook(Game::Player *player);
bool krakenDiveHook(Game::Player *player);
bool isInKingSquidHook(Game::Player *player);
void barrierEffectNameHook();
Cmn::Def::Mode shotPropertiesHook();
Game::BulletMgr *extraBigLaserBulletHook(Game::BulletMgr *mgr);
bool princessCannonHook(bool val);
void showFps();
void actorDbHook(Lp::Utl::ByamlIter *iter, const char **a, const char *b);

int weaponFixHook(gsys::Model *model, sead::SafeStringBase<char> lol);
void playerModelDrawHook(Cmn::PlayerWeapon *playerWeapon, sead::Matrix34<float> *mtx);

float hokoFlyVelHook(float val);
int hokoFrmHook(int val);
int startfrm;
void handleCreateObj();

void autoMatchTest();
bool rivalOctohook(Cmn::Def::PlayerModelType modeltyape);

// Main
void renderEntrypoint(agl::DrawContext *drawContext, sead::TextWriter *textWriter);
void hooks_init();

void allocHeap();

void setPlayerInfoBySaveDataHook(Cmn::PlayerInfo *playerInfo, Cmn::Def::Mode mode);
bool respawnPointHook(Lp::Utl::ShapeSphere const& arg1,Lp::Utl::ShapeCapsule const& arg2,Lp::Utl::CollisionResult *arg3);
bool jetPackJetHook(Game::Player *player);
void sceneChanger();
void PlayEnable();
void PlaySuperArmorUse();
void PlaySuperArmorSt();
void PlayBarrierOn();
void PlaySuperArmorVanish();
void PlayFreeBombsEffect();
void healPlayerSuperLandingHook(Game::Player *player);
void PlayDisable();
void handlePlayerControl();

void dbgInfoSetHook(Cmn::PlayerInfoAry *playerInfoAry, Cmn::Def::Mode mode, sead::SafeStringBase<char> arg3);