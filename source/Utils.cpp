#include "flexlion/Utils.hpp"
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

sead::Vector3<float> Utils::calcGroundPos(Game::Player *player, sead::Vector3<float> pos){
	if(player->mPlayerSuperLanding == NULL) return pos;
	sead::Vector3<float> oldpos = player->mPosition;
	player->mPosition = pos;
	player->mPlayerSuperLanding->calcLandingPos();
	player->mPosition = oldpos;
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