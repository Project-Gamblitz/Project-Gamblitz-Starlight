#include "flexlion/Utils.hpp"
#include "flexlion/InkstrikeMgr.hpp"
#include "flexlion/FsLogger.hpp"
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

	// Pierce loop. calcLandingPos sweeps ~500 units down from
	// player.mPosition.Y. We:
	//   - step probe.Y down by 450 if it didn't hit anything (covers
	//     stacked geometry / huge vertical drops)
	//   - pierce 15u below the reported landPos if it hit something
	//     invalid (bad mat / bad bits / no real surface info), so the
	//     next sweep starts cleanly below the rejected surface
	//   - bail if probe.Y falls below -500 or we run out of attempts
	//
	// Start Y comes from mSpawnY (team respawn centroid Y + 150,
	// captured once at match start by InkstrikeMgr::tryCaptureSpawnY).
	//
	// Pierce step (-15u) is sized just larger than calcLandingPos's
	// internal sphere radius (~6-10u) so the next sweep doesn't overlap
	// the surface we just rejected.
	sead::Vector3<float> probe = pos;
	probe.mY = Flexlion::InkstrikeMgr::sInstance->mSpawnY;

	sead::Vector3<float> landPos = player->mPlayerSuperLanding->mLandingPos;
	u16 lastAttr = 0xFFFF;
	bool lastIsWall = false;
	bool gotValidHit = false;
	int attemptsUsed = 0;
	const char *lastReason = "INIT";

	const int kMaxAttempts = 10;
	for(int attempt = 0; attempt < kMaxAttempts; attempt++){
		attemptsUsed = attempt + 1;

		if(probe.mY < -500.0f){
			lastReason = "BAIL_BELOW_MAP";
			break;
		}

		player->mPosition = probe;
		player->mPlayerSuperLanding->calcLandingPos();
		// Restore mPosition immediately so the re-sweep below sees the
		// player at their real location, and so we never leak a deep
		// probe Y into other systems if we end up bailing out.
		player->mPosition = oldpos;

		if(player->mPlayerSuperLanding->mLandingDist >= 499.0f){
			lastReason = "NOHIT_STEPDOWN";
			probe.mY -= 450.0f;
			continue;
		}

		landPos = player->mPlayerSuperLanding->mLandingPos;

		// NOTE: we DO NOT early-return here when outFound is NULL.
		// Both validation calls (every frame, with outFound) and commit
		// calls (once at A-release, without outFound) must run the same
		// pierce-and-validate logic. Otherwise the commit would just
		// return the first calcLandingPos hit — which on a bit-9/10
		// kill-plane surface is the kill plane itself, not the safe
		// floor that validation pierced past. The Inkstrike then lands
		// on the kill plane and the shooter dies. That was the visible
		// "raycast says one thing, Inkstrike lands somewhere else"
		// desync bug.

		// Re-sweep at the landing position to fetch the KCL attribute.
		// calcLandingPos itself doesn't expose the attr.
		//
		// Sweep geometry tuned to be a tight probe of one point:
		//   offset +10u: starts just above the reported landing without
		//     re-hitting parent geometry above (which +100u tended to do).
		//   dist 30u: only need to reach the surface calcLandingPos
		//     already found; longer sweeps risk catching unrelated stuff.
		//   radius 2u: smaller sphere = less chance of grazing sibling
		//     geometry. We're probing a point, not physically moving.
		Cmn::KDGndCol::CheckIF colCheck((Cmn::Actor*)player);
		sead::Vector3<float> sweepStart = landPos;
		sweepStart.mY += 10.0f;
		sead::Vector3<float> sweepDir;
		sweepDir.mX = 0.0f;
		sweepDir.mY = -1.0f;
		sweepDir.mZ = 0.0f;
		colCheck.checkMoveSphere(
			sweepStart, sweepDir, 30.0f, 2.0f,
			0xFFFFFEFF, 0xFFFFFFFF,
			Cmn::KDGndCol::Manager::cWallNrmY_L, 1.0f);

		bool valid = true;
		u16 attr = 0xFFFF;
		bool isWall = false;
		bool hadBombBits = false;  // bit 9 or 10 = death-volume / bomb-only

		int rf = colCheck.mResultFlags;
		if(rf == 0){
			// No material info — old code rejected this case.
			valid = false;
			lastReason = "NOMATERIAL";
		} else {
			u8 *hi = (u8*)colCheck.mHitInfoImpl;

			// HitInfoImpl layout (RE'd from Cmn::KDGndCol::HitInfoImpl):
			//   +140 (0x8C): u16 floor attr  (written by entryGeomL only
			//                                 if a floor was actually hit)
			//   +144 (0x90): float floor distance (clearImpl resets to
			//                                     -3.4e38 sentinel)
			//   +224 (0xE0): u16 wall attr   (only written if wall hit)
			//   +228 (0xE4): float wall distance (sentinel reset)
			//
			// The KEY insight: clearImpl does NOT clear +140 or +224
			// directly — those retain STALE attr values from previous
			// sweeps if the current sweep didn't write them. So picking
			// the right field via mResultFlags & 1 (which only means
			// "any hit") is wrong: when we hit only a wall, +140 still
			// holds whatever floor attr a previous sweep left there.
			//
			// Correct test: check the distance fields. If floorDist is
			// still the sentinel (-3.4e38), the floor attr was NOT
			// written this sweep, so don't trust it. Same for wall.
			float floorDist = *(float*)(hi + 144);
			float wallDist  = *(float*)(hi + 228);
			bool gotFloor = (floorDist > -1.0e30f);
			bool gotWall  = (wallDist  > -1.0e30f);

			if(gotFloor){
				// Prefer floor attr — calcLandingPos found us a floor,
				// this is the matching surface.
				attr = *(u16*)(hi + 140);
				isWall = false;
			} else if(gotWall){
				// No floor populated, but a wall was. Sphere likely
				// brushed past a wall corner during descent.
				attr = *(u16*)(hi + 224);
				isWall = true;
			} else {
				// rf & 1 was set but neither floor nor wall got written
				// (paint-only hit, or some other category). Can't trust
				// any attr.
				valid = false;
				lastReason = "NO_GEOM";
			}

			if(valid){
				// Attr layout:
				//   bits 0-5  (0x003F): material ID
				//   bits 6-8  (0x01C0): GeomKind (0-3 = floor, 4-7 = wall)
				//   bit 9  (0x0200): Inkjet-only modifier  -> reject
				//   bit 10 (0x0400): bomb-invisible        -> reject
				//   bit 11 (0x0800): grate modifier        -> reject (floor)
				//   bit 12 (0x1000): grate modifier        -> reject (floor)
				//   bit 13 (0x2000): slippery surface (OK)
				//   bit 14 (0x4000): grate (OK)
				//   bit 15 (0x8000): grate modifier        -> reject (floor)
				static const u16 kBadAttrBits = 0x9E00;

				// Material blacklist — inverse of our previous allowlist
				// {0..8,10,13,17,23,25,28,31,32}, then with 14 and 15
				// re-allowed (they pass paint-landing checks despite
				// being rejected by the old code's blacklist).
				int mat = attr & 0x3F;
				int kind = (attr >> 6) & 7;
				u16 badBits = attr & kBadAttrBits;
				bool isWallKind = (kind >= 4);

				// Bit 9 (0x0200, Inkjet-only) and bit 10 (0x0400,
				// bomb-invisible). On many maps these mark death-volume
				// surfaces — landing the Inkstrike on one kills the
				// shooter. We always reject these via the bits check
				// below, but ALSO use a bigger pierce step afterward
				// (20u instead of 15u) so the next sweep cleanly clears
				// the kill plane and finds the real geometry below.
				hadBombBits = ((attr & 0x0600) != 0);

				bool matIsBad = (mat == 9
					|| mat == 11 || mat == 12
					|| mat == 16
					|| mat == 18 || mat == 19
					|| mat == 20 || mat == 21 || mat == 22
					|| mat == 24 || mat == 26 || mat == 27
					|| mat == 29 || mat == 30
					|| mat == 33 || mat == 34);

				if(attr == 0x0000){
					// Special case: attr is literally all zeros (mat=0,
					// kind=0, no flags). Kind bits being zero means we
					// have no surface-type info — likely a no-data hit
					// rather than a real paintable surface. Reject.
					valid = false;
					lastReason = "ATTR_ZERO";
				} else if(isWallKind){
					// Walls legitimately have grate/modifier bits set,
					// so only the material blacklist applies.
					if(matIsBad){ valid = false; lastReason = "BAD_MAT"; }
					else { lastReason = "ACCEPT_WALL"; }
				} else {
					// Floors: bits first, then mat blacklist.
					if(badBits != 0){ valid = false; lastReason = "BAD_BITS"; }
					else if(matIsBad){ valid = false; lastReason = "BAD_MAT"; }
					else { lastReason = "ACCEPT_FLOOR"; }
				}
			}
		}

		lastAttr = attr;
		lastIsWall = isWall;

		if(valid){
			gotValidHit = true;
			break;
		}

		// Pierce: step probe just below this hit and try again.
		// Keep the original X/Z so we stay on the cursor ray. When the
		// rejected surface had bit 9 or 10 set (death-volume kill plane),
		// drop a bit further so we cleanly clear it.
		float pierceStep = hadBombBits ? 20.0f : 15.0f;
		probe.mY = landPos.mY - pierceStep;
	}

	// Loop exited. mPosition is already restored (we restore it after
	// every calcLandingPos call inside the loop), so no extra work here.

	if(Flexlion::InkstrikeMgr::sInstance != NULL){
		Flexlion::InkstrikeMgr::sInstance->mDbgColAttr = lastAttr;
		Flexlion::InkstrikeMgr::sInstance->mDbgColIsWall = lastIsWall;
		Flexlion::InkstrikeMgr::sInstance->mDbgColY = landPos.mY;
	}

	if(!gotValidHit){
		// Rate-limit: skip log if both cursor and reason are unchanged
		// from previous call (avoids 60 lines/sec when holding still
		// over an invalid spot).
		static float sLastX = -99999.0f;
		static float sLastZ = -99999.0f;
		static const char *sLastReason = "";
		float dx = pos.mX - sLastX;
		float dz = pos.mZ - sLastZ;
		bool cursorMoved = (dx*dx + dz*dz) > 1.0f;
		bool reasonChanged = (sLastReason != lastReason);
		if(cursorMoved || reasonChanged){
			FsLogger::LogFormatDefaultDirect(
				"[Ray] REJECT cursor=(%.1f,%.1f) atts=%d reason=%s lastAttr=%04X\n",
				pos.mX, pos.mZ, attemptsUsed, lastReason, lastAttr);
			sLastX = pos.mX;
			sLastZ = pos.mZ;
			sLastReason = lastReason;
		}
		if(outFound) *outFound = false;
		return player->mPlayerSuperLanding->mLandingPos;
	}

	if(outFound) *outFound = true;
	return landPos;
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