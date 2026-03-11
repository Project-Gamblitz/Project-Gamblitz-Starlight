#include "flexlion/Utils.hpp"
#include "flexlion/InkstrikeMgr.hpp"
#include "Cmn/KDGndCol/Manager.h"
static bool wasSceneLoaded;
static bool *isEffectPlay;
rotateMtxFunc Utils::rotateMtxY = (rotateMtxFunc)(NULL);
rotateMtxFunc Utils::rotateMtxX = (rotateMtxFunc)(NULL);
static bool initsd = 0;

static sead::Heap *mStarlightHeap;

bool Utils::isShowMinimap(){
	Game::MainMgr *mainMgr = Game::MainMgr::sInstance;
	if(mainMgr == NULL){
		return 0;
	}
	Game::HudMgr *hudMgr = mainMgr->mHudMgr;
	if(hudMgr == NULL){
		return 0;
	}
	return hudMgr->isShowMiniMap();
}

Game::MiniMap *Utils::getMinimap(){
	Game::MainMgr *mainMgr = Game::MainMgr::sInstance;
	if(mainMgr == NULL){
		return NULL;
	}
	Game::HudMgr *hudMgr = mainMgr->mHudMgr;
	if(hudMgr == NULL){
		return NULL;
	}
	return hudMgr->mMiniMap;
}

sead::Vector3<float> Utils::calcGroundPos(Game::Player *player, sead::Vector3<float> pos, bool *outFound){
	if(player->mPlayerSuperLanding == NULL){
		if(outFound) *outFound = false;
		return pos;
	}
	sead::Vector3<float> oldpos = player->mPosition;
	// calcLandingPos raycasts down ~500 units from player position.
	// Iterate from high to low in 450-unit steps to cover the full vertical range.
	// mLandingDist == 500.0 means no ground was found (max distance reached).
	sead::Vector3<float> probe = pos;
	for(float startY = 3000.0f; startY >= -500.0f; startY -= 450.0f){
		probe.mY = startY;
		player->mPosition = probe;
		player->mPlayerSuperLanding->calcLandingPos();
		if(player->mPlayerSuperLanding->mLandingDist < 499.0f){
			player->mPosition = oldpos;
			if(outFound){
				sead::Vector3<float> landPos = player->mPlayerSuperLanding->mLandingPos;
				bool valid = true;

				// 1) Check floor material via downward sweep.
				// kindFloor bit 0x100 must be clear (Manager skips the
				// static block when set and CheckIF has no block filter).
				// bit 0x80 enables attribute storage in entryGeomL.
				// Floor attr at HitInfoImpl+140 as u16, masked to 6 bits.
				Cmn::KDGndCol::CheckIF colCheck((Cmn::Actor*)player);
				sead::Vector3<float> sweepStart = landPos;
				sweepStart.mY += 100.0f;
				sead::Vector3<float> sweepDir;
				sweepDir.mX = 0.0f;
				sweepDir.mY = -1.0f;
				sweepDir.mZ = 0.0f;
				colCheck.checkMoveSphere(
					sweepStart, sweepDir, 200.0f, 6.0f,
					0xFFFFFEFF, 0xFFFFFFFF,
					Cmn::KDGndCol::Manager::cWallNrmY_L, 1.0f);
				int rf = colCheck.mResultFlags;
				if(rf != 0){
					u8 *hi = (u8*)colCheck.mHitInfoImpl;
					u16 attr;
					bool isWall;
					if(rf & 1){
						attr = *(u16*)(hi + 140);
						isWall = false;
					} else {
						attr = *(u16*)(hi + 224);
						isWall = true;
					}
					if(Flexlion::InkstrikeMgr::sInstance != NULL){
						Flexlion::InkstrikeMgr::sInstance->mDbgColAttr = attr;
						Flexlion::InkstrikeMgr::sInstance->mDbgColIsWall = isWall;
					}
					int mat = attr & 0x3F;
					if(mat == 11 || mat == 12
						|| mat == 14 || mat == 15 || mat == 16
						|| mat == 20 || mat == 24 || mat == 26
						|| mat == 27 || mat == 33 || mat == 34)
						valid = false;
				} else {
					if(Flexlion::InkstrikeMgr::sInstance != NULL){
						Flexlion::InkstrikeMgr::sInstance->mDbgColAttr = 0xFFFF;
						Flexlion::InkstrikeMgr::sInstance->mDbgColIsWall = false;
					}
					valid = false;
				}

				// 2) Check for keepout walls between player and target.
				// Out-of-bounds ground has normal material but sits behind
				// keepout walls. Sweep horizontally from player toward the
				// landing pos; if a keepout wall (kind 0x20000) blocks the
				// path, reject the position.
				// kindFloor must have wall-detection bits set (0xCA mask in
				// entryGeomL); 0x33 matches calcLandingPos.
				// Sweep at player Y so the probe is at keepout wall height.
				if(valid){
					float dx = landPos.mX - oldpos.mX;
					float dz = landPos.mZ - oldpos.mZ;
					float hDist = sqrtf(dx * dx + dz * dz);
					if(hDist > 1.0f){
						Cmn::KDGndCol::CheckIF colCheck2((Cmn::Actor*)player);
						sead::Vector3<float> hStart = oldpos;
						hStart.mY += 50.0f;
						sead::Vector3<float> hDir;
						hDir.mX = dx / hDist;
						hDir.mY = 0.0f;
						hDir.mZ = dz / hDist;
						colCheck2.checkMoveSphere(
							hStart, hDir, hDist, 6.0f,
							0x33, 0x20000,
							Cmn::KDGndCol::Manager::cWallNrmY_L, 1.0f);
						if(colCheck2.mResultFlags != 0)
							valid = false;
					}
				}

				*outFound = valid;
			}
			return player->mPlayerSuperLanding->mLandingPos;
		}
	}
	player->mPosition = oldpos;
	if(outFound) *outFound = false;
	return player->mPlayerSuperLanding->mLandingPos;
}

sead::Vector3<float> Utils::getRotFromMtx(sead::Matrix34<float> mtx){
	/* Matrix Template:
	mtx = {{ 
    [0][0]        cosf(rot.mY) * cosf(rot.mZ),
    [0][1]        sinf(rot.mX) * sinf(rot.mY) * cosf(rot.mZ) - sinf(rot.mZ) * cosf(rot.mX),
    [0][2]        cosf(rot.mX) * sinf(rot.mY) * cosf(rot.mZ) + sinf(rot.mZ) * sinf(rot.mX),
    [0][3]        pos.mX,
    [1][0]        sinf(rot.mZ) * cosf(rot.mY),
    [1][1]        sinf(rot.mX) * sinf(rot.mY) * sinf(rot.mZ) + cosf(rot.mZ) * cosf(rot.mX),
    [1][2]        cosf(rot.mX) * sinf(rot.mY) * sinf(rot.mZ) - cosf(rot.mZ) * sinf(rot.mX),
    [1][3]        pos.mY,
    [2][0]        -sinf(rot.mY),
    [2][1]        sinf(rot.mX) * cosf(rot.mY),
    [2][2]        cosf(rot.mX) * cosf(rot.mY),
    [2][3]        pos.mZ
        }};
	*/
	sead::Vector3<float> res;
	res.mX = atan2f(mtx.matrix[2][1], mtx.matrix[2][2]);
	res.mZ = atan2f(mtx.matrix[1][0], mtx.matrix[0][0]);
	res.mY = atan2f(-mtx.matrix[2][0], mtx.matrix[2][1] / sinf(res.mX));
	return res;
}

void Utils::showAppError(int errorcode, const char* format, ...) {
    char buff[0x1000];
    va_list args;
	memset(buff, 0, sizeof(buff));
	va_start(args, format);
    vsnprintf(buff, sizeof(buff), format, args);
	static nn::settings::LanguageCode LanguageCode = {};
	static bool initLang = 0;
    if (!initLang){
		LanguageCode = nn::settings::LanguageCode::Make(nn::settings::Language::Language_Chinese);
		initLang = 1;
	}
    nn::err::ApplicationErrorArg arg(errorcode, buff, buff, LanguageCode);
    nn::err::ShowApplicationError(arg);
	va_end (args);
}

void Utils::showAppError(const char* format, ...){
    char buff[0x1000];
    va_list args;
	memset(buff, 0, sizeof(buff));
	va_start(args, format);
    vsnprintf(buff, sizeof(buff), format, args);
	static nn::settings::LanguageCode LanguageCode = {};
	static bool initLang = 0;
    if (!initLang){
		LanguageCode = nn::settings::LanguageCode::Make(nn::settings::Language::Language_Chinese);
		initLang = 1;
	}
    nn::err::ApplicationErrorArg arg(0, buff, buff, LanguageCode);
    nn::err::ShowApplicationError(arg);
	va_end (args);
}

double strtod_impl(const char *nptr, char **endptr){
	static strtodptr sdkImpl;
	if(sdkImpl == NULL){
		sdkImpl = (strtodptr)(((u64)&nn::oe::GetDisplayVersion) + (0x396400 - 0x14F5C4));
	}
	return sdkImpl(nptr, endptr);
}

void Utils::setStarlightHeap(sead::Heap *heap){
	mStarlightHeap = heap;
}

sead::Heap *Utils::getStarlightHeap(){
	return mStarlightHeap;
}

bool Utils::isValidWeapon(Cmn::Def::WeaponKind kind, int id){
	Cmn::MushDataHolder *mushData = Cmn::MushDataHolder::sInstance;
	if(mushData != NULL){
		return mushData->mMushWeaponInfo->getById(kind, id) != NULL;
	}
	return 0;
}

sead::Color4f Utils::getCurColor(Cmn::Def::Team team){
    sead::Color4f col = sead::Color4f::cWhite;
    Cmn::GfxSetting *gfxSetting = Cmn::GfxSetting::sInstance;
	if(gfxSetting != NULL){
        Cmn::TeamColorMgr *TeamColorMgr = gfxSetting->mTeamColorMgr;
        if(TeamColorMgr != NULL){
            switch(team){
            case Cmn::Def::Team::Alpha:
                col = TeamColorMgr->teamColorSet.mAlpha;
                break;
            case Cmn::Def::Team::Bravo:
                col = TeamColorMgr->teamColorSet.mBravo;
                break;
            case Cmn::Def::Team::Neutral:
                col = TeamColorMgr->teamColorSet.mNeutral;
                break;
            case Cmn::Def::Team::Ex:
                col = TeamColorMgr->teamColorSet.mEx;
                break;
            };
        }
    }
    return col;
}

int Utils::getPlayerNum(){
	Cmn::StaticMem *mem = Cmn::StaticMem::sInstance;
	if(mem == NULL){
		return 0;
	}
	Cmn::PlayerInfoAry *infoAry = mem->mPlayerInfoAry;
	if(infoAry == NULL){
		return 0;
	}
	return infoAry->getValidInfoNum();
}

bool Utils::isSceneLoaded(){
    Lp::Sys::SceneMgr *sceneMgr = Lp::Sys::SceneMgr::sInstance;
	if(sceneMgr != NULL){
		Cmn::SceneMgrFader *mgr = sceneMgr->mSceneMgrFader;
		if(mgr != NULL){
			return mgr->getState() != 2;
		}
	}
	return 0;
}

bool Utils::isSceneJustLoaded(){
	if(Utils::isSceneLoaded() and !wasSceneLoaded){
		return 1;
	}
	return 0;
}

void Utils::onLeaveRender(){
	wasSceneLoaded = Utils::isSceneLoaded();
}

bool Utils::isSceneJustUnloaded(){
	if(wasSceneLoaded and !Utils::isSceneLoaded()){
		return 1;
	}
	return 0;
}

Cmn::Def::Team Utils::getControlledTeam(){
	Game::PlayerMgr *playerMgr = Game::PlayerMgr::sInstance;
	if(playerMgr != NULL){
		Game::Player *player = playerMgr->getControlledPerformer();
		if(player != NULL){
			return player->mTeam;
		}
	}
	return Cmn::Def::Team::Alpha;
}

Game::Player *Utils::getControlledPerformer(){
	Game::PlayerMgr *playerMgr = Game::PlayerMgr::sInstance;
	if(playerMgr == NULL){
		return NULL;
	}
	return playerMgr->getControlledPerformer();
}

Game::Player *Utils::getPerformerById(int id){
	Game::PlayerMgr *playerMgr = Game::PlayerMgr::sInstance;
	if(playerMgr == NULL){
		return NULL;
	}
	return playerMgr->getPerformerAt(id);
}

float Utils::getDistance(sead::Vector3<float> a1, sead::Vector3<float> a2){
	float x = abs(a1.mX - a2.mX);
	float y = abs(a1.mY - a2.mY);
	float z = abs(a1.mZ - a2.mZ);
	return sqrtf(x * x + y * y + z * z);
}

sead::Vector3<float> Utils::getCameraPosByMtx(sead::Matrix34<float> mtx){
	sead::Camera camera;
	camera.mMtx = mtx;
	sead::Vector3<float> vec;
	camera.getWorldPosByMatrix(&vec);
	return vec;
}

sead::Vector3<float> Utils::getCameraLookAtByMtx(sead::Matrix34<float> mtx){
	sead::Camera camera;
	camera.mMtx = mtx;
	sead::Vector3<float> vec;
	camera.getLookVectorByMatrix(&vec);
	return vec;
}

agl::TextureData *Utils::getTextureDataFromModelArc(const char *arcname, const char *texname){
	Lp::Sys::ModelArc arc(sead::SafeStringBase<char>::create((char*)arcname), NULL, 0, NULL, NULL);
	agl::TextureData *data = new agl::TextureData();
	Cmn::GfxUtl::createTexture(&arc, texname, data);
	return data;
}

sead::Vector3<float> Utils::getMtxPos(sead::Matrix34<float> *mtx){
	sead::Vector3<float> pos;
	pos.mX = mtx->matrix[0][3];
	pos.mY = mtx->matrix[1][3];
	pos.mZ = mtx->matrix[2][3];
	return pos;
}

void Utils::setMtxPos(sead::Matrix34<float> *mtx, sead::Vector3<float> pos){
	mtx->matrix[0][3] = pos.mX;
	mtx->matrix[1][3] = pos.mY;
	mtx->matrix[2][3] = pos.mZ;
}

int Utils::getGearIdByName(Cmn::Def::GearKind kind, char *name){
	Cmn::MushDataHolder *mushData = Cmn::MushDataHolder::sInstance;
	if(mushData == NULL){
		return 0;
	}
	Cmn::MushGearInfo *gearInfo = mushData->mMushGearInfo;
	if(gearInfo == NULL){
		return 0;
	}
	Cmn::GearData *data = gearInfo->searchByName(kind, sead::SafeStringBase<char>::create(name));
	if(data == NULL){
		return 0;
	}
	return data->mId;
}

bool Utils::isPlayerClass(void *ptr){
	if(ptr == NULL){
		return 0;
	}
	return ((*(u64*)ptr) == ProcessMemory::MainAddr(0x2C0BAD8));
}