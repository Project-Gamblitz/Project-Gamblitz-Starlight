#include "main.hpp"

using namespace starlight;

extern "C" {
    extern void (*__preinit_array_start__[])(void) __attribute__((weak));
    extern void (*__preinit_array_end__[])(void) __attribute__((weak));
    extern void (*__init_array_start__[])(void) __attribute__((weak));
    extern void (*__init_array_end__[])(void) __attribute__((weak));
    extern void (*__fini_array_start__[])(void) __attribute__((weak));
    extern void (*__fini_array_end__[])(void) __attribute__((weak));
}

static void __run_init_array(void) {
    size_t count;
    size_t i;

    count = __preinit_array_end__ - __preinit_array_start__;
    for (i = 0; i < count; i++)
        __preinit_array_start__[i]();

    count = __init_array_end__ - __init_array_start__;
    for (i = 0; i < count; i++)
        __init_array_start__[i]();
}

static void __run_fini_array(void) {
    size_t count = __fini_array_end__ - __fini_array_start__;
    for (size_t i = count; i > 0; i--)
        __fini_array_start__[i - 1]();
}

// Needed on old versions of rtld that doesn't check for DT_INIT existance.
extern "C" void __custom_init(void) {
    __run_init_array();
}

// DT_FINI here for completeness.
extern "C" void __custom_fini(void) {
    __run_fini_array();
}

static agl::DrawContext *mDrawContext;
static sead::TextWriter *mTextWriter;
static sead::ExpHeap* mStarlightHeap;
static int mode;
static bool showMenu;

extern "C" sead::ExpHeap *flexGetStarlightHeap(){
	return mStarlightHeap;
};

//static bool justSwitched;

static Starlion::KingSquidMgr *kingSquidMgr = NULL;
static Cmn::CtrlChecker *ctrlChecker = NULL;
static Starlion::S1Inkstrike *mS1Inkstrike = NULL;
static Flexlion::InkstrikeMgr *tornadoMgr = NULL;
template<>
sead::Vector2<float> sead::Vector2<float>::zero = sead::Vector2<float>(0.0f, 0.0f);
template<>
sead::Vector3<float> sead::Vector3<float>::zero = sead::Vector3<float>(0.0f, 0.0f, 0.0f);
template<>
sead::Vector3<float> sead::Vector3<float>::ey = sead::Vector3<float>(0.0f, 1.0f, 0.0f);
bool Game::Utl::ActorFactoryBase::isNoActor() const{
	return false;
}

static bool tsthk = 0;

static int custommgrjpt[27];
Cmn::EffectManualHandle *mBarrierEffectHandles[10];

/*static void (*stateVictoryOrDefeatImpl)(Game::SeqVersusResult *);
void stateVictoryOrDefeatHook(Game::SeqVersusResult *vres){
	stateVictoryOrDefeatImpl(vres);
	serverClient->changeState(ServerClient::CalcState::ServerMatchRes);
};*/

static void (*miniMapCamCalcImpl)(Game::MiniMapCamera *_this);
static xlink2::UserInstanceSLink *(*startSkill_DeathMarkingImpl)(Game::Player*, unsigned int, char);
static void (*startAllMarking_ImplOrig)(Game::Player*, int);
void miniMapCamCalcHook(Game::MiniMapCamera *_this){
	miniMapCamCalcImpl(_this);
	float anim = tornadoMgr->cameraanim;
	_this->mPosition.mX *= 1.0f - anim;
	_this->mPosition.mX+=0.05f * anim;
	_this->mPosition.mY *= 1.0f - anim;
	_this->mPosition.mY+=tornadoMgr->cameraheight * anim;
	_this->mPosition.mZ *= 1.0f - anim;
	_this->mPosition.mZ+=0.05f * anim;
	_this->mAt.mX *= 1.0f - anim;
	_this->mAt.mY *= 1.0f - anim;
	_this->mAt.mZ *= 1.0f - anim;
	_this->mUp.mY *= 1.0f - anim;
}

static void (*handleBulletCloneEventImpl)(Game::BulletCloneHandle *cloneHandle, Game::Player *player, Game::BulletCloneEvent *event, int clonefrm);
void handleBulletCloneEventHook(Game::BulletCloneHandle *cloneHandle, Game::Player *player, Game::BulletCloneEvent *event, int clonefrm){
	int paintfrm = (*((int (**)(Game::SeqMgrBase*, uint))((*(u64*)Game::MainMgr::sInstance->seqMgr) + (78 * 8))))(Game::MainMgr::sInstance->seqMgr, clonefrm); // im lazy to define vtable
	if(event->mType == Game::BulletCloneEvent::Type::BulletTypeInkstrike) tornadoMgr->informShotInkstrike(player, event->mPos, event->mVel, paintfrm);
	//FsLogger::LogFormatDefaultDirect("\nEvent:\nType: %i\nPos: %.2f %.2f %.2f\nVel: %.2f %.2f %.2f\nPlayerId: %i\n_unk: %i\n\n", 
	//event->mType, event->mPos.mX, event->mPos.mY, event->mPos.mZ, event->mVel.mX, event->mVel.mY, event->mVel.mZ, event->mPlayerId, event->_unk);
	handleBulletCloneEventImpl(cloneHandle, player, event, clonefrm);
}

static void (*playerFirstCalcOg)(Game::Player*);
static void (*playerFourthCalcOg)(Game::Player*);
void playerFirstCalcHook(Game::Player *player){
	playerFirstCalcOg(player);
	tornadoMgr->playerFirstCalc(player);
}

void playerFourthCalcHook(Game::Player *player){
	playerFourthCalcOg(player);
	tornadoMgr->playerFourthCalc(player);
}

void handleDisplayVersion(nn::oe::DisplayVersion *ver){
	if(ver != NULL){
		nn::oe::GetDisplayVersion(ver);
		const char *sversion = "6.3 Final";
		size_t ogLen = strlen(ver->name);
		size_t stLen = strlen(sversion);
		memcpy(ver->name, sversion, stLen);
	}
}

xlink2::PropertyDefinition::~PropertyDefinition(){};

void renderEntrypoint(agl::DrawContext *drawContext, sead::TextWriter *textWriter)
{
	mDrawContext = drawContext;
	mTextWriter = textWriter;
	DrawUtils::setDrawContext(mDrawContext);
	//DrawUtils::setTextWriter(mTextWriter);
	DrawUtils::setColor(sead::Color4f::cWhite);
	mTextWriter->mColor = sead::Color4f::cWhite;
	sead::Heap *oldHeap;

	Collector::init();
	Collector::collect();

	if(mStarlightHeap == NULL){
		allocHeap();
	} 
	if(mStarlightHeap != NULL){
		oldHeap = Collector::mHeapMgr->setCurrentHeap_(mStarlightHeap);
	} else{
		return;
	}
	Utils::setStarlightHeap(mStarlightHeap);
	static bool init = false;
	if(!init){
		init_starlion();
		init = true;
	}

    if(Collector::mController.isPressed(Controller::Buttons::LStick))
        showMenu = !showMenu;
	
	/*if(showMenu){
		DrawUtils::drawBackground();
	}*/

	static bool initcustommushtable;
	if(!initcustommushtable){
		if(Collector::mMushDataHolder != NULL){
			Starlion::Sp1WeaponLookup::Initialize();
			custommgrjptHook();
			if(Utils::isValidWeapon(Cmn::Def::WeaponKind::cSpecial, 24)){
				*(u64*)ProcessMemory::MainAddr(0x2A32000) = (u64)&getSuperShotBurstWaitFrameHook;
				*(u64*)ProcessMemory::MainAddr(0x2A32008) = (u64)&getSuperShotBurstWarnFrameHook;
				*(u64*)ProcessMemory::MainAddr(0x2A31F40) = (u64)&calcHokoDamageHook;
			}
			if(Utils::isValidWeapon(Cmn::Def::WeaponKind::cSpecial, 22)){
				*(u64*)ProcessMemory::MainAddr(0x2A3F910) = (u64)&calcAquaBallDamageHook;
			}
			initcustommushtable = 1;
		}
	}
	//serverClient->onCalc();
	ctrlChecker->calc();
	handleSupershot();
	agentThreeHandle();
	handlePlayerEffects();
	kingSquidMgr->onCalc();
	tornadoMgr->onCalc();

	if(IS_DEV){
	  //sceneChanger();
	  if(showMenu){
		mTextWriter->printf("Gamblitz Dev Build v.%s\n", DEV_VER);
		mTextWriter->printf("Mod Version: %s\n", MOD_VER);
		mTextWriter->printf("Current heap name: %s\n", Collector::mHeapMgr->getCurrentHeap()->mName.mCharPtr);
		mTextWriter->printf("Current heap free space: 0x%x\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());
		mTextWriter->printf("Current scene name: %s\n", Lp::Utl::getCurSceneName());
		if(Cmn::StaticMem::sInstance){
			mTextWriter->printf("Loaded Stage: %s\n", Cmn::StaticMem::sInstance->mMapFileName1.mCharPtr);
		}
		Game::Player *player = Utils::getControlledPerformer();
		if(player){
			handlePlayerControl();
			mTextWriter->printf("Pos: %.2f %.2f %.2f\n", player->mPosition.mX, player->mPosition.mY, player->mPosition.mZ);
			mTextWriter->printf("Vel: %.2f ", player->mMoveVel.mX, player->mMoveVel.mY, player->mMoveVel.mZ);
			mTextWriter->printf("Vel.y: %.2f\n", player->mJumpVel);
	  }
    }
}
	//mS1Inkstrike->onRender();
	static int renderstate = 0;
	static int renderctr = 0;
	char *scenename = Lp::Utl::getCurSceneName();
	switch(renderstate){
	case 0:
		if(scenename != NULL and strcmp(scenename, "Boot") == 0){
			renderctr+=1;
			if(renderctr > 60){
				renderstate+=1;
			}
		}
		break;
	case 1:
		renderstate+=DrawUtils::drawLogo(strcmp(scenename, "Boot") != 0);
		break;
	case 2:
		break;
	}
	static int mControlState = 0;
	Collector::mController.update();
	Collector::mControllerDebug.update();
	Collector::mHeapMgr->setCurrentHeap_(oldHeap);
	Utils::onLeaveRender();
}

void agentThreeHandle(){
	Game::PlayerMgr *playerMgr = Collector::mPlayerMgrInstance;
	Cmn::Def::Mode mode = Cmn::Def::getCurMode();
	Cmn::PlayerInfo::setInfoType setType = Cmn::PlayerInfo::cAgent3;
	if(playerMgr != NULL){
		Game::Player* player = playerMgr->getControlledPerformer();
		if(player != NULL and isWorld00()){
			Cmn::PlayerInfo *ctrlPlayerInfo = player->mPlayerInfo;
			Cmn::PlayerCustomMgr *ctrlPlayerCustomMgr = player->mPlayerCustomMgr;
			player->mPlayerInfo->setInfoByPlayerType(0, setType, mode);
		}
	}
}

bool isWorld00(){
	if(Cmn::StaticMem::sInstance) return (Cmn::Def::getCurMode() == Cmn::Def::Mode::cWorld and strcmp(Cmn::StaticMem::sInstance->mMapFileName1.mCharPtr, "Fld_World00_Wld") == 0);
	return 0;
}

int msnArmorHook(Cmn::MushGearInfo *gearInfo, Cmn::Def::GearKind kind, int armorLv){
	if(isWorld00()) return Utils::getGearIdByName(kind, "MSN000");
	return gearInfo->searchIdArmor(kind, armorLv);
}

void healPlayerSuperLandingHook(Game::Player *player){
    player->resetDamage();
    player->dropHoldingClamAll_ForSpecial();
}

void handleSupershot(){
	Game::Player *player = Collector::mControlledPlayer;
	if(player == NULL or !Utils::isSceneLoaded()){
		return;
	}
	if(player->isInSpecial() and player->mSpecialWeaponId == 24){
		player->mPlayerInkAction->mChargerChargeState = 100;
		Prot::ObfStore(&player->mPlayerInkAction->mChargerChargeFrame, 1000);
	}
	auto iterNode = Game::BulletGachihoko::getClassIterNodeStatic();
	for(Game::BulletGachihoko *ita = (Game::BulletGachihoko*)iterNode->derivedFrontActiveActor(); ita != NULL; ita = (Game::BulletGachihoko*)iterNode->derivedNextActiveActor(ita)){
		if(!Utils::isPlayerClass(ita->mSender)){
			continue;
		}
	}
}

void handlePlayerEffects(){
	static bool isInSpecialFreebombs[10] = {};
	if(!Utils::isSceneLoaded()){
		memset(isInSpecialFreebombs, 0, sizeof(isInSpecialFreebombs));
		return;
	}
	xlink2::Handle tmp;
	auto playerit = Game::Player::getClassIterNodeStatic();
	for(Game::Player *ita = (Game::Player*)playerit->derivedFrontActor(); ita != NULL; ita = (Game::Player*)playerit->derivedNextActor(ita)){
		if(ita->mSpecialWeaponId == 20 and ita->isInSpecial()){
			ita->mBarrierEndFrm = Game::MainMgr::sInstance->mPaintGameFrame + Prot::ObfLoad(&ita->mSpecialLeftFrame);
			ita->_B80 = -10;
			//PlaySuperArmorVanish();
		}
		//Bomb Rush
		if(ita->isInSpecial_FreeBombs() && !isInSpecialFreebombs[ita->mIndex]){
			// ita->mXLink->searchAndPlayWrap("VNice_Girl", true, &tmp2); slink test
			ita->mXLink->searchAndEmitWrap("RollerDash", true, &tmp);
			ita->mXLink->searchAndEmitWrap("SpecialMode_00", true, &tmp);
			ita->mXLink->searchAndEmitWrap("SpecialModeHead", true, &tmp);	
			isInSpecialFreebombs[ita->mIndex] = true;
		} 
		else if(!ita->isInSpecial_FreeBombs() && isInSpecialFreebombs[ita->mIndex]) {
			ita->mXLink->killAllEffect();
			isInSpecialFreebombs[ita->mIndex] = false;
		}
		//Inkzooka
		if(ita->isInSpecial() and ita->mSpecialWeaponId == 24){
			ita->mXLink->searchAndEmitWrap("SpecialMode_00", true, &tmp);
			ita->mXLink->searchAndEmitWrap("SpecialModeHead", true, &tmp);	
		}
		else if(ita->mSpecialWeaponId == 24 and !ita->isInSpecial()){
			// ita->mXLink->searchAndPlayWrap("WaterCutter_PutBack", true, &tmp); //need to fix 
		}
		//Inkstrike
		if(ita->isInSpecial() and ita->mSpecialWeaponId == 25){
			ita->mXLink->searchAndEmitWrap("SpecialMode_00", true, &tmp);
			ita->mXLink->searchAndEmitWrap("SpecialModeHead", true, &tmp);	
		}
		//Killer Wail
		if(ita->isInSpecial() and ita->mSpecialWeaponId == 19){
			ita->mXLink->searchAndEmitWrap("SpecialMode_00", true, &tmp);
			ita->mXLink->searchAndEmitWrap("SpecialModeHead", true, &tmp);	
		}
		//Bubbler
		if(ita->isInSpecial() and ita->mSpecialWeaponId == 20){
			ita->mXLink->searchAndEmitWrap("SpecialMode_00", true, &tmp);
			ita->mXLink->searchAndEmitWrap("SpecialModeHead", true, &tmp);	
			// ita->mXLink->searchAndEmitWrap("SWpInkArmorSt", true, &tmp);	
		}
	};
}

/*void FreeBombEffecthook(){
	Game::Player *player = Collector::mControlledPlayer;
	if(player->isInSpecial_FreeBombs() and player->mSpecialWeaponId == 23){
		xlink2::Handle tmp;
        player->mXLink->searchAndEmitWrap("SWpInkArmor", false, &tmp);
		}
	return;
}*/

void supershotJumpHook(){
	asm("CMP W20, #0");
	asm("B.EQ #8");
	asm("B _ZN3Cmn15PlayerCustomMgr27checkAndCreateSpecialWeaponEiNS_12PlayerCustom4KindEb_CC");
	asm("MOV X0, X19");
	asm("MOV X1, XZR");
	asm("BL _ZN2Lp3Sys5Actor6createIN3Cmn19PlayerWeaponShooterEEEPT_PS1_PN4sead4HeapE");
	asm("B _ZN3Cmn15PlayerCustomMgr27checkAndCreateSpecialWeaponEiNS_12PlayerCustom4KindEb_1D4");
}

void tornadoJumpHook(){
	asm("CMP W20, #0");
	asm("B.EQ #8");
	asm("B _ZN3Cmn15PlayerCustomMgr27checkAndCreateSpecialWeaponEiNS_12PlayerCustom4KindEb_CC");
	asm("MOV X0, X19");
	asm("MOV X1, XZR");
	asm("BL _ZN2Lp3Sys5Actor6createIN3Cmn19PlayerWeaponShooterEEEPT_PS1_PN4sead4HeapE");
	asm("B _ZN3Cmn15PlayerCustomMgr27checkAndCreateSpecialWeaponEiNS_12PlayerCustom4KindEb_1D4");
}

u64 specialSetupWithoutModelHook(){
	return 0xa582438000; // byte array for which specials mush model is created
	// cTornado, cSupershot + og: 0xa582438000
	// cSupershot + og: 0xa582418000
}

int *custommgrjptHook(){
	custommgrjpt[0] = ((u64)&supershotJumpHook) - ((u64)custommgrjpt);
	int *oldJptable = (int*)ProcessMemory::MainAddr(0x24BE358);
	for(int i = 1; i < 27; i++){
		custommgrjpt[i] = oldJptable[i - 1];
		if(custommgrjpt[i] != 0){
			custommgrjpt[i]+=((u64)oldJptable) - ((u64)custommgrjpt);
		}
	}
	custommgrjpt[2] = ((u64)&tornadoJumpHook) - ((u64)custommgrjpt);
	return custommgrjpt;
}

int getSuperShotBurstWaitFrameHook(Game::BulletGachihoko *bullet){
	if(bullet == NULL){
		return ((int (*)())ProcessMemory::MainAddr(0x4D7D84))();
	}
	if(!Utils::isPlayerClass(bullet->mSender)){
		return ((int (*)())ProcessMemory::MainAddr(0x4D7D84))();
	}
	Game::Player *player = (Game::Player*)bullet->mSender;
	if(player->isInSpecial() and player->mSpecialWeaponId == 24){
		return 0;
	}
	return ((int (*)())ProcessMemory::MainAddr(0x4D7D84))();
}

int getSuperShotBurstWarnFrameHook(Game::BulletGachihoko *bullet){
	if(bullet == NULL){
		return ((int (*)())ProcessMemory::MainAddr(0x4D7DB4))();
	}
	if(!Utils::isPlayerClass(bullet->mSender)){
		return ((int (*)())ProcessMemory::MainAddr(0x4D7DB4))();
	}
	Game::Player *player = (Game::Player*)bullet->mSender;
	if(player->isInSpecial() and player->mSpecialWeaponId == 24){
		return 0;
	}
	return ((int (*)())ProcessMemory::MainAddr(0x4D7DB4))();
}

CURLcode curl_easy_perform_hook(CURL *curl){
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
	curl_easy_setopt(curl, CURLOPT_USE_SSL, 0L);
	return curl_easy_perform(curl);
}

void registKingSquidAnimHook(Game::PlayerAnimCtrlSet *animCtrlSet, int a1,char const*a2,bool a3,bool a4){
	register Game::PlayerMotion *motion asm("x19");
	Starlion::PlayerKingSquid* kingsquid = ((Starlion::PlayerKingSquid*)motion->mPlayer->mPlayerKingSquid);
	animCtrlSet->registAnim(a1, a2, a3, a4);
	if(kingsquid == NULL){
		return;
	}
	if(kingsquid->mKingSquidAnim == NULL){
		return;
	}
	kingsquid->mKingSquidAnim->registAnim(a1, a2, a3, a4);
}

void setupKingSquidAnimHook(Game::PlayerAnimCtrlSet *animCtrlSet, Game::PlayerJoint *joint){
	animCtrlSet->setup(joint);
	Starlion::PlayerKingSquid* kingsquid = ((Starlion::PlayerKingSquid*)joint->mPlayer->mPlayerKingSquid);
	if(kingsquid == NULL){
		return;
	}
	if(kingsquid->mKingSquidAnim == NULL or kingsquid->mPlayerJointKingSquid == NULL){
		return;
	}
	kingsquid->mKingSquidAnim->setup(kingsquid->mPlayerJointKingSquid);
}

void kingSquidAnimSetControllerHook(Game::AnimSetController *anim, gsys::PartialSkeletalAnm const* cool){
	anim->setPartialSkeletalAnm(cool);
	register Game::PlayerMotion *motion asm("x19");
	Starlion::PlayerKingSquid* kingsquid = ((Starlion::PlayerKingSquid*)motion->mPlayer->mPlayerKingSquid);
	if(kingsquid == NULL){
		return;
	}
	if(kingsquid->mKingSquidAnim == NULL){
		return;
	}
	kingsquid->mSquidAnimCtrl = anim;
	kingsquid->mKingSquidAnimCtrl = new Game::AnimSetController(3, NULL);
	kingsquid->mKingSquidAnimCtrl->_4 = 1;
	kingsquid->mKingSquidAnimCtrl->mModel = kingsquid->mKingSquidModel;
	//*(u64*)(0x69) = 0x69;
	kingsquid->mKingSquidAnimCtrl->setSkelSlot(0);
	kingsquid->mKingSquidAnimCtrl->setMatSlot(1);
	kingsquid->mKingSquidAnimCtrl->load();
	kingsquid->mKingSquidAnimCtrl->setPartialSkeletalAnm(cool);
}

void init_starlion(){
	exl::util::impl::InitMemLayout();
	FsLogger::Initialize();
    FsLogger::LogFormatDefaultDirect("[Gamblitz] 5.5.1 test\n");
    FsLogger::LogFormatDefaultDirect("[Gamblitz] a\n");
	FsLogger::LogFormatDefaultDirect("[Gamblitz] Initialized funcs/vars!\n");
	if(IS_DEV){
		FsLogger::LogFormatDefaultDirect("[Gamblitz Development Build]\nDev Build v.%s ", DEV_VER);
		FsLogger::LogFormatDefaultDirect("Mod Version: %s\n", MOD_VER);
	}
	Utils::rotateMtxY = (rotateMtxFunc)(ProcessMemory::MainAddr(0x159904));
	Utils::rotateMtxX = (rotateMtxFunc)(ProcessMemory::MainAddr(0x1599B4));
	//stateVictoryOrDefeatImpl = *(void (**)(Game::SeqVersusResult*))ProcessMemory::MainAddr(0x2D35E58);
	//*(void (**)(Game::SeqVersusResult*))ProcessMemory::MainAddr(0x2D35E58) = stateVictoryOrDefeatHook;
	miniMapCamCalcImpl = *(void (**)(Game::MiniMapCamera*))ProcessMemory::MainAddr(0x2AE8518);
	*(void (**)(Game::MiniMapCamera*))ProcessMemory::MainAddr(0x2AE8518) = miniMapCamCalcHook;
	Cmn::ActorVtable *playerVtable = (Cmn::ActorVtable*)ProcessMemory::MainAddr(0x2C0BAD8);
	playerFirstCalcOg = (void (*)(Game::Player*))playerVtable->firstCalc;
	playerFourthCalcOg = (void (*)(Game::Player*))playerVtable->fourthCalc;
	playerVtable->firstCalc = (u64)playerFirstCalcHook;
	playerVtable->fourthCalc = (u64)playerFourthCalcHook;
	handleBulletCloneEventImpl = (void (*)(Game::BulletCloneHandle *, Game::Player *, Game::BulletCloneEvent *, int))ProcessMemory::MainAddr(0x4CF54C);
	DrawUtils::makeTudou();
	FsLogger::LogFormatDefaultDirect("[Gamblitz] Initialized DrawUtils, 0x%x free RAM.\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());
	kingSquidMgr = new Starlion::KingSquidMgr();
	FsLogger::LogFormatDefaultDirect("[Gamblitz] Created KingSquidMgr, 0x%x free RAM.\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());
	mS1Inkstrike = new Starlion::S1Inkstrike();
	//curlClient = new Flexlion::CurlClient();
	//FsLogger::LogFormatDefaultDirect("[Gamblitz] Created CurlClient, 0x%x free RAM.\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());
	tornadoMgr = new Flexlion::InkstrikeMgr();
	FsLogger::LogFormatDefaultDirect("[Gamblitz] Created InkstrikeMgr, 0x%x free RAM.\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());
	//serverClient = new ServerClient();
	//FsLogger::LogFormatDefaultDirect("[Gamblitz] Created ServerClient, 0x%x free RAM.\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());
	_BYTE randomBuf[0x28];
	memset(randomBuf, 0, sizeof(randomBuf));
	sead::TaskConstructArg arg;
	memset(&arg, 0, sizeof(arg));
	arg._0 = (u64)(&randomBuf);
	ctrlChecker = new Cmn::CtrlChecker(arg);
	FsLogger::LogFormatDefaultDirect("[Gamblitz] Created Cmn::CtrlChecker, 0x%x free RAM.\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());
	ctrlChecker->calc();
	FsLogger::LogFormatDefaultDirect("[Gamblitz] Initialized, 0x%x free RAM.\n", Collector::mHeapMgr->getCurrentHeap()->getFreeSize());

	startSkill_DeathMarkingImpl = (xlink2::UserInstanceSLink *(*)(Game::Player*, unsigned int, char))
		ProcessMemory::MainAddr(0x01011268);

	startAllMarking_ImplOrig = (void (*)(Game::Player*, int))
		ProcessMemory::MainAddr(0x010197F0);

}

static bool isEmitting[10];

void playerModelSetupHook(Game::PlayerModel *pmodel){
	pmodel->setup();
	pmodel->mPlayer->mPlayerKingSquid = new Starlion::PlayerKingSquid(pmodel->mPlayer);
	tornadoMgr->registerPlayer(pmodel->mPlayer);
	int idx = pmodel->mPlayer->mIndex;
	mBarrierEffectHandles[idx] = new Cmn::EffectManualHandle; 
	mBarrierEffectHandles[idx]->createReservationInfo(0x100);
	mBarrierEffectHandles[idx]->searchAndEmit("GuGachihokoBarrier", 0x100, 0);
	mBarrierEffectHandles[idx]->setTeamColor(pmodel->mPlayer->mTeam);
	isEmitting[idx] = 0;
	//todo: add bubbler activation effect "GuGachiHokoAppear" or "BuSwpAquaBallMounting", should appear everytime u activate bubbler and not just once
}

int calcHokoDamageHook(Game::BulletGachihoko *bullet, int armortype, Cmn::Def::Team team, sead::Vector3<float> const& pos){
	int res = ((int (*)(Game::BulletGachihoko *, int, Cmn::Def::Team, sead::Vector3<float> const&))ProcessMemory::MainAddr(0x4DB3FC))(bullet, armortype, team, pos);
	if(!Utils::isPlayerClass(bullet->mSender)){
		return res;
	}
	Game::Player *player = (Game::Player*)bullet->mSender;
	if(player->isInSpecial() and player->mSpecialWeaponId == 24 and res != 0){
		return 1500;
	}
	return res;
}

int calcAquaBallDamageHook(Game::BulletSpAquaBall *bullet, int armortype, Cmn::Def::Team team, sead::Vector3<float> const& pos){
	int res = ((int (*)(Game::BulletSpAquaBall *, int, Cmn::Def::Team, sead::Vector3<float> const&))ProcessMemory::MainAddr(0x53FF70))(bullet, armortype, team, pos);
	if(!Utils::isPlayerClass(bullet->mSender)){
		return res;
	}
	Game::Player *player = (Game::Player*)bullet->mSender;
	if(player->mPlayerKingSquid == NULL){
		return res;
	}
	Starlion::PlayerKingSquid *kingsquid = (Starlion::PlayerKingSquid*)player->mPlayerKingSquid;
	if(kingsquid->mBullet == bullet and team != bullet->mTeam){
		return 1600;
	}
	return res;
}

bool barrierEffectHook(Game::PlayerEffect* peffect, bool isEmit) {
	int idx = peffect->mPlayer->mIndex;
	nn::vfx::EmitterSet* set = NULL;
	nn::vfx::Handle* handle = *(nn::vfx::Handle**)(((u64)mBarrierEffectHandles[idx]) + 0x20);
	//PlaySuperArmorSt();
	//PlaySuperArmorUse();
	if ((*((u64*)handle)) != NULL) {
		set = handle->GetEmitterSet();
	}
	if (isEmit) {
		if (isEmitting[idx]) {
			if (set != NULL) {
				sead::Vector3<float> pos = peffect->mPlayer->mPosition;
				pos.mY = peffect->mPlayer->getHeadPos().mY - 4.0f;
				set->mPos1 = pos;
				set->mPos2 = pos;
				const float barrierscale = 0.8f;
				nn::util::neon::MatrixRowMajor4x3fType mtx = { {
					barrierscale,    0.0f,       		 0.0f, 				0.0f,
					0.0f,    		 barrierscale,       0.0f,				0.0f,
					0.0f,    		 0.0f,       		 barrierscale,		0.0f,
					pos.mX, 		 pos.mY,			 pos.mZ,			0.0f,
				} };
				set->SetMatrix(mtx);
				auto unkInfoHolder = set->_150;
				for (auto unkInfoHolder = set->_150; unkInfoHolder != NULL; unkInfoHolder = unkInfoHolder->mIterNode) {
					sead::Color4f color = Utils::getCurColor(peffect->mPlayer->mTeam);
					unkInfoHolder->mColor1 = color;
					unkInfoHolder->mColor2 = color;
				}
			}
		}
		else {
			mBarrierEffectHandles[idx]->emitParticle(sead::Vector3<float>::zero, peffect->mPlayer);
			mBarrierEffectHandles[idx]->setTeamColor(peffect->mPlayer->mTeam);
			isEmitting[idx] = 1;
		}
	}
	else {
		if (isEmitting[idx] and set != NULL) {
			set->mPos1 = { -10000.0f, -10000.0f, -10000.0f };
			set->mPos2 = { -10000.0f, -10000.0f, -10000.0f };
			set->mReqUpdatePos = 1;
		}
	}
	return 0;
}

void playerKingSquidCalcHook(Game::Player *player){
	Starlion::PlayerKingSquid *kingSquid = ((Starlion::PlayerKingSquid*)player->mPlayerKingSquid);
	if(kingSquid != NULL){
		kingSquid->calc();
	}
	Game::MainMgr *mainMgr;
	Game::Player *tmp;
	tmp = player;
	asm("MOV X19, X0");
	mainMgr = Game::MainMgr::sInstance;
	asm("MOV X8, X0");
}

// Reimplementation of Game::SighterTarget::startAllMarking(int, int) from 3.1.0
// NOTE: SighterTarget field offsets are from 3.1.0 and may need adjustment for 5.5.2
void Game::SighterTarget_startAllMarking(Game::SighterTarget *sighterTarget, int a2, int a3) {
	u8 *st = (u8 *)sighterTarget;

	// Check state at 0x590 - skip if in states 2, 3, or 4
	u32 state = *(u32 *)(st + 0x590);
	if ((state - 2) < 3)
		return;

	// Store marking parameters
	*(u32 *)(st + 0x66C) = a2;
	*(u32 *)(st + 0x670) = (a3 >= 1) ? a3 : 1;

	// Get controlled player
	Game::Player *player = Game::PlayerMgr::sInstance->getControlledPerformer();

	// Store a3 at 0x6A8
	*(u32 *)(st + 0x6A8) = a3;

	// Get scale factor via nested object chain:
	// this[0x4D0] -> [0x10] + 0x5A0 -> vtable[0xD8/8]() returns float*
	u64 v7 = *(u64 *)(st + 0x4D0);
	u64 subObj = *(u64 *)(v7 + 0x10) + 0x5A0;
	float *scalePtr = ((float *(*)(u64))(*(u64 *)(*(u64 *)subObj + 0xD8)))(subObj);
	float scale = *scalePtr;

	// Compute target position: direction * scale + offset
	float targetX = scale * *(float *)(st + 0x384) + *(float *)(st + 0x39C);
	float targetY = scale * *(float *)(st + 0x388) + *(float *)(st + 0x3A0);
	float targetZ = scale * *(float *)(st + 0x38C) + *(float *)(st + 0x3A4);
	*(float *)(st + 0x6B8) = targetX;
	*(float *)(st + 0x6BC) = targetY;
	*(float *)(st + 0x6C0) = targetZ;

	// Get player barrier center position
	sead::Vector3<float> barrierPos;
	player->calcBarrier_CenterPos(&barrierPos);
	*(sead::Vector3<float> *)(st + 0x6AC) = barrierPos;

	// Calculate distance between barrier center and target position
	float dx = barrierPos.mX - targetX;
	float dy = barrierPos.mY - targetY;
	float dz = barrierPos.mZ - targetZ;
	float dist = sqrtf(dx * dx + dy * dy + dz * dz);

	// Map distance to search line duration: [0..600] -> [2.0..5.0]
	float searchLineValue;
	if (dist <= 0.0f) {
		searchLineValue = 2.0f;
	} else if (dist >= 600.0f) {
		searchLineValue = 5.0f;
	} else {
		searchLineValue = (dist * 3.0f / 600.0f) + 2.0f;
	}
	*(float *)(st + 0x6C4) = searchLineValue;

	// Emit "SearchLine" XLink effect
	xlink2::Handle handle;
	sighterTarget->mXLink->searchAndEmitWrap("SearchLine", false, &handle);

	// Store handle
	*(u64 *)(st + 0x6C8) = (u64)handle.mEvent;
	*(u32 *)(st + 0x6D0) = handle.mEventId;
}

// Hook for Game::Player::startAllMarking_Impl to add SighterTarget marking
void startAllMarking_ImplHook(Game::Player *player, int a1) {
	// Call original startAllMarking_Impl
	player->startAllMarking_Impl(a1);

	// Iterate all active SighterTargets and call startAllMarking on each
	auto iterNode = Game::SighterTarget::getClassIterNodeStatic();
	for (Game::SighterTarget *st = (Game::SighterTarget *)iterNode->derivedFrontActiveActor();
		 st != NULL;
		 st = (Game::SighterTarget *)iterNode->derivedNextActiveActor(st))
	{
		Game::SighterTarget_startAllMarking(st, a1 + 0x21C, a1);
	}
}

void markedHook(Game::Player *player, int a1,int a2,Game::Player::MarkingType a3,int a4,unsigned int a5){
	Game::MainMgr::sInstance->mPaintGameFrame+=0x14;
	player->startMarked_Bomb_Direct(0x21C, a4, 0);
	// sightertarget->startMarkedBomb(a2);
	PlaySuperArmorUse();
	PlaySuperArmorSt();
	Game::MainMgr::sInstance->mPaintGameFrame-=0x14;
}

xlink2::UserInstanceSLink *startSkill_DeathMarkingHook(Game::Player *player, unsigned int a2, char a3) {
    xlink2::Handle v8;

    Game::Player *PerformerAt = Game::PlayerMgr::sInstance->getPerformerAt(a2);

    // a3 bit 0 selects marking type (m3=2 vs m2=1)
    Game::Player::MarkingType markType = (a3 & 1) ? Game::Player::MarkingType::m3
                                                   : Game::Player::MarkingType::m2;

    player->startMarkingOne_Impl(PerformerAt, markType, 0, 0);

    if (!player->mIsRemote) {
        int *netCtrlField = (int *)((u8 *)player->mPlayerNetControl + 132);
        *netCtrlField |= 0x20;
    }

    player->mXLink->searchAndPlayWrap("MarkingDeathAttackStart", false, &v8);
    return (xlink2::UserInstanceSLink *)v8.mEvent;
}


void inkstrikeNetHook(u64 *x0, u32 w1, u64 *x2, u64 *x3, u32 w4, u32 w5){
	if(!tornadoMgr->isShot) ((void (*)(u64*, u32, u64*, u64*, u32, u32))ProcessMemory::MainAddr(0x4CEE6C))(x0, w1, x2, x3, w4, w5);
}
sead::Vector3<float> inkstrikeBombVelHook(Game::PlayerInkAction *inkAction){
	sead::Vector3<float> vec = inkAction->getShotVel_BombStd();
	if(tornadoMgr->isShot){
		vec.mX=0.0f;
		vec.mY=-5.0f;
		vec.mZ=0.0f;
	}
	return vec;
}

void inkstrikeShotHook(Game::BulletSpSuperBall *ball, Game::Player *sender, int senderId, int senderId2, sead::Vector3<float> *startpos, Game::SuperBallShotArg2 *arg2, int poop){
	Flexlion::BulletTornado *bullet = tornadoMgr->bullets[sender->mIndex];
	if(bullet and tornadoMgr->isShot) *startpos = bullet->pos;
	ball->shot(sender, senderId, senderId2, startpos, arg2, poop);
}

bool isInInkstrikeCarryHook(Game::Player *player){
	return (player->isInSpecial() and player->mSpecialWeaponId == TORNADO_SPECIAL_ID) and tornadoMgr->playerState[player->mIndex] != Flexlion::TornadoState::cShoot;
}

sead::Heap *npcHeapHook(sead::HeapMgr *a1, sead::Heap *a2){
	sead::Heap *heap = a1->setCurrentHeap_(a2);
	FsLogger::LogFormatDefaultDirect("[Dbg] Npc Creation, Free Heap Space: %i\n", a2->getFreeSize());
	return heap;
}

sead::HeapMgr *npcHeapFix(sead::HeapMgr *a1, sead::Heap *a2){
	if(a2 != NULL){
		if(a2->getFreeSize() < 20000000){
			a2 = NULL;
		}
	}
	sead::Heap *poop;
	poop = a2;
	asm("MOV X1, X0");
	return a1;
}
 
void hooks_init(){
    autoMatchTest();
    rivalOctohook(Cmn::Def::PlayerModelType::Rival);
	weaponFixHook(NULL, sead::SafeStringBase<char>::create("bruh"));
	playerModelDrawHook(NULL, NULL);
	krakenDiveHook(0);
	isInKingSquidHook(NULL);
	barrierEffectNameHook();
	renderEntrypoint(NULL, NULL);
	extraBigLaserBulletHook(NULL);
	jetPackJetHook(0);
	curl_easy_perform_hook(NULL);
	inkstrikeBombVelHook(NULL);
	inkstrikeShotHook(NULL, NULL, 0, 0, NULL, NULL, 0);
	bcatHook(0);
	npcHeapFix(NULL, NULL);
	//getBombThrowSpanFrmHook(NULL, 0);
	isInLauncherHook(NULL);
	Game::SighterTarget_startAllMarking(NULL, 0, 0);
	startAllMarking_ImplHook(NULL, 0);
	markedHook(NULL, 0, 0, Game::Player::MarkingType::m1, 0, 0);
	startSkill_DeathMarkingHook(NULL, 0, 0);
	inkstrikeNetHook(NULL, 0, NULL, NULL, 0, 0);
	playerModelSetupHook(NULL);
	playerKingSquidCalcHook(NULL);
	handleDisplayVersion(NULL);
	registKingSquidAnimHook(NULL, 0, NULL, 0, 0);
	setupKingSquidAnimHook(NULL, NULL);
	kingSquidAnimSetControllerHook(NULL, NULL);
	actorDbHook(NULL, NULL, NULL);
	supershotJumpHook();
	custommgrjptHook();
	specialSetupWithoutModelHook();
	getSuperShotBurstWaitFrameHook(NULL);
	getSuperShotBurstWarnFrameHook(NULL);
	barrierEffectHook(NULL, 0);
	stepPaintTypeHook(NULL);
	fixEffHook(NULL);
	playerModelResourceLoadHook(NULL, NULL);
	createPlayerModelHook(Cmn::Def::Team::Alpha, sead::SafeStringBase<char>::create("test"), *(Lp::Sys::ModelArc*)NULL, *(Lp::Utl::ModelCreateArg*)NULL, NULL);
	damageReasonHook(NULL, *(Game::DamageReason*)NULL, NULL, 0, 0, 0);
	GetCharKindHook(NULL, 0);
	isInInkstrikeCarryHook(NULL);
	handleBulletCloneEventHook(NULL, NULL, NULL, 0);
	PlaySuperArmorUse();
	healPlayerSuperLandingHook(NULL);
	createHumanModelHook(sead::SafeStringBase<char>::create("test"), Cmn::Def::Team::Alpha, *(Game::PlayerModelResource*)NULL,  *(Lp::Utl::ModelCreateArg*)NULL,  *(Lp::Utl::AnimCreateArg*)NULL, Cmn::Def::PlayerModelType::InkGirl, *(sead::RingBuffer<int>*)NULL);
	msnArmorHook(0, Cmn::Def::GearKind::cHead, 0);
}

ushort GetCharKindHook(uintptr_t _this, ushort charKind){
	switch(charKind){
	case 0x32:
		return 0xE087;
	case 0x33:
		return 0xE089;
	case 0x34:
		return 0xE0B5;    
	case 0x35:
		return 0xE0C0;
	case 0x36:
		return 0xE0C1;
	case 0x37:
		return 0xE0B6;
	};
	return ((ushort (*)(uintptr_t, ushort))ProcessMemory::MainAddr(0x423FF4))(_this, charKind);
}

void damageReasonHook(Game::VersusBeatenPage *page, Game::DamageReason &reason, Cmn::PlayerInfo const*attackerInfo, int dieFrm, int uiAppearFrm, bool iwannaslep){
	Cmn::Def::WeaponKind kind = Cmn::Def::WeaponKind(reason.mClassType << 26 >> 26); // clear unnecessary bytes
	if(attackerInfo != NULL and (kind == Cmn::Def::WeaponKind::cMain or reason.mClassType == 9 or reason.mClassType == 12 or reason.mClassType == 4 or reason.mClassType == 6)){
		Game::Player *player = Utils::getPerformerById(attackerInfo->mPlayerIndex);
		if(player != NULL){
			Starlion::Sp1WeaponLookup::getLookupRefId(&reason.mWeaponId, player->mMainWeaponId, reason.mWeaponId);
		}
	}
	page->start(reason, attackerInfo, dieFrm, uiAppearFrm, iwannaslep);
}

static Lp::Sys::ModelArc *inkGirlHalfNewArc = NULL;
static Lp::Sys::ModelArc *rivalHalfArc = NULL;
static Lp::Sys::ModelArc *rivalFullArc = NULL;

void playerModelResourceLoadHook(Game::PlayerModelResource *res, sead::Heap *heap){
	inkGirlHalfNewArc = NULL;
	rivalHalfArc = NULL;
	rivalFullArc = NULL;
	res->load(heap);
	inkGirlHalfNewArc = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Player00_Hlf"), heap, 0, NULL, NULL);
	rivalHalfArc = (Lp::Sys::ModelArc *)res->mHalfArcs.mPtr[6];
	if(rivalHalfArc == NULL) rivalHalfArc = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Rival00_Hlf"), heap, 0, NULL, NULL);
	rivalFullArc = new Lp::Sys::ModelArc(sead::SafeStringBase<char>::create("Player04"), heap, 0, NULL, NULL);
}

gsys::Model *createHumanModelHook(sead::SafeStringBase<char> const& name, Cmn::Def::Team team, Game::PlayerModelResource &res, const Lp::Utl::ModelCreateArg &arg, Lp::Utl::AnimCreateArg const&animarg, Cmn::Def::PlayerModelType modeltype, sead::RingBuffer<int> const&hairinfo){
	register Game::PlayerModel *x19 asm("x19");
	u64 tmparc = res.mHumanArcs.mPtr[2];
	if(modeltype == Cmn::Def::PlayerModelType::OctGirl and x19->mPlayer->mPlayerInfo->mModelId == 54){
		res.mHumanArcs.mPtr[2] = (u64)rivalFullArc;
	}
	gsys::Model *model = Cmn::PlayerCustomUtl::createPlayerModelHuman(name, team, res, arg, animarg, modeltype, hairinfo);
	res.mHumanArcs.mPtr[2] = tmparc;
	return model;
}

gsys::Model *createPlayerModelHook(Cmn::Def::Team team,sead::SafeStringBase<char> const& name,Lp::Sys::ModelArc &arc,Lp::Utl::ModelCreateArg const&createArg,sead::Heap *heap){
	Game::PlayerModelResource *res = &Game::PlayerMgr::sInstance->mModelResource;
	Lp::Sys::ModelArc *useArc = &arc;
	register Game::PlayerModel *x19 asm("x19");
	if(((Lp::Sys::ModelArc *)res->mHalfArcs.mPtr[0]) == useArc and x19->mPlayer->mPlayerInfo->mModelId == 0){
		useArc = inkGirlHalfNewArc;
	} else if(((Lp::Sys::ModelArc *)res->mHalfArcs.mPtr[2]) == useArc and x19->mPlayer->mPlayerInfo->mModelId == 54){ // nacho asked to add this for his hair
		useArc = rivalHalfArc;
	}
	gsys::Model *model = Cmn::GfxUtl::createModel(team, name, *useArc, createArg, heap);
	return model;
}

int stepPaintTypeHook(Game::PlayerStepPaint *step){
	Game::Player *player = step->mPlayer;
	if(player->isInSpecial_KingSquid_Impl(0) and player->mPlayerKingSquid != NULL){
		Starlion::PlayerKingSquid *kingSquid = (Starlion::PlayerKingSquid *)player->mPlayerKingSquid;
		return !kingSquid->mIsRush;
	}
	return step->mStepPaintType;
}

bool fixEffHook(Game::Player *player){
	return (player->isInSquid_Stealth_Move() and !player->isInSpecial_KingSquid_Impl(0));
}

void actorDbHook(Lp::Utl::ByamlIter *iter, const char **a, const char *b){
	iter->tryGetStringByKey(a, b);
	if(strcmp(*a, "TestObj_Lift_Ikkaku112") == 0){
		memcpy((void*)(*a), "Obj_Box00S", strlen("Obj_Box00S") + 1);
	}
}

void bcatHook(int *res){
	int poop;
	register int w8 asm("w8");
	int p2 = *res;
	if(-1 >= 0){
		p2 = 2;
	}
	poop = p2;
	asm("MOV W8, W0");
}

void allocHeap() {
    Lp::Sys::HeapGroup* heapGroup = Lp::Sys::HeapGroup::sInstance;
    if(heapGroup != NULL){
        sead::ExpHeap* othersHeap = heapGroup->mHeaps[Lp::Sys::HeapGroup::Group::cOthers];
        Collector::mHeapMgr->setCurrentHeap_(othersHeap);
        sead::Heap* currentHeap = Collector::mHeapMgr->getCurrentHeap();

        if(mStarlightHeap == NULL){
            sead::SafeStringBase<char> str;
            str = (char*)"StarlightHeap";

            mStarlightHeap = sead::ExpHeap::create(0, str, currentHeap, 4, sead::Heap::HeapDirection::TAIL, true);
        }
    }
}

Game::BulletMgr *extraBigLaserBulletHook(Game::BulletMgr *mgr){
	register u64 x1 asm("x1");
	Cmn::Def::Mode mode = Cmn::Def::getCurMode();
	if(mode == Cmn::Def::Mode::cPlaza or mode == Cmn::Def::Mode::cPlayerMake or mode == Cmn::Def::Mode::cStaffRoll or mode == Cmn::Def::Mode::cStaffRollOcta or mode == Cmn::Def::Mode::cTwoShot or mode == Cmn::Def::Mode::cCustomize){
		x1 = 0;
		return mgr;
	}
	Cmn::PlayerInfoAry *ary = Cmn::StaticMem::sInstance->mPlayerInfoAry;
	if(mode != Cmn::Def::Mode::cVersus){
		for(int i = 0; i < ary->getValidInfoNum() * 2; i++){
			Lp::Sys::Actor::create<Game::BulletSuperLaser>(mgr->getBulletParent(), NULL);
		}	
		if((mode != Cmn::Def::Mode::cMission or strcmp(Cmn::StaticMem::sInstance->mMapFileName1.mCharPtr, "Fld_BossLastKing_Msn") != 0) and mode != Cmn::Def::Mode::cMissionOcta){
			for(int i = 0; i < 16; i++){
				Lp::Sys::Actor::create<Game::BulletGachihoko>(mgr->getBulletParent(), NULL);
			}
			for(int i = 0; i < 16 * 20; i++){
				Lp::Sys::Actor::create<Game::BulletGachihokoSplash>(mgr->getBulletParent(), NULL);
			}
		}
		if(mode == Cmn::Def::Mode::cMission){
			for(int i = 0; i < ary->getValidInfoNum(); i++){
				Lp::Sys::Actor::create<Game::BulletSpJetpackJet>(mgr->getBulletParent(), NULL);
				Lp::Sys::Actor::create<Game::BulletSpSuperBall>(mgr->getBulletParent(), NULL);
			}
		}
	} else{
		for(int i = 0; i < ary->getValidInfoNum(); i++){
			Lp::Sys::Actor::create<Game::BulletSpAquaBall>(mgr->getBulletParent(), NULL);
			for(int j = 0; j < 2; j++){
				Lp::Sys::Actor::create<Game::BulletSlosherBase>(mgr->getBulletParent(), NULL);
			}
			for(int j = 0; j < 8; j++){
				Lp::Sys::Actor::create<Game::BulletSlosherSplash>(mgr->getBulletParent(), NULL);
			}
			Lp::Sys::Actor::create<Game::BulletSpJetpackJet>(mgr->getBulletParent(), NULL);
			Lp::Sys::Actor::create<Game::BulletSpSuperBall>(mgr->getBulletParent(), NULL);
		}
		for(int i = 0; i < 8; i++){
			Lp::Sys::Actor::create<Game::BulletGachihoko>(mgr->getBulletParent(), NULL);
		}
		for(int i = 0; i < 8 * 20; i++){
			Lp::Sys::Actor::create<Game::BulletGachihokoSplash>(mgr->getBulletParent(), NULL);
		}
	}
	x1 = 0;
	return mgr;
}


bool jetPackJetHook(Game::Player *player){
	if(player != NULL){
		return ((player->isInSpecial_KingSquid_Impl(0) and Prot::ObfLoad(&player->mAerialState) == 0) or player->mPlayerJetpack->isInAirWalk());
	}
	return 0;
}

int weaponFixHook(gsys::Model *model, sead::SafeStringBase<char> lol){
	if(model != NULL){
		return model->searchBone(lol);
	} else{
		return 0xFFFFFFFF;
	}
}

int ret69Hook(){
	return 69;
}

void playerModelDrawHook(Cmn::PlayerWeapon *playerWeapon, sead::Matrix34<float> *mtx){
	playerWeapon->getRootBoneMtx(mtx);
	if(playerWeapon->iCustomPlayerInfo == NULL){ 
		return;
	}
	Game::Player *player = playerWeapon->iCustomPlayerInfo->vtable->getGamePlayer(playerWeapon->iCustomPlayerInfo);
	if(player == NULL){
		return;
	}
	/*if(mS1Inkstrike->isBulletActive[player->mIndex] and ((*((u64*)((*(u64*)playerWeapon) + (0x8)))) == ProcessMemory::MainAddr(0x1AF438))){ // Second check is vtable for PlayerWeaponSuperMissile
		memcpy(mtx, &mS1Inkstrike->mtxs[player->mIndex], sizeof(sead::Matrix34<float>));
		Utils::rotateMtxY(mtx, 90.0f / 180.0f * MATH_PI);
	}*/
	
	//memcpy(&bufMtx, mtx, sizeof(sead::Matrix34<float>));
}

bool clamCalcSleepHook(Game::VictoryClamHolding *clam){
	if(clam->clamDirector != NULL){
		return (*(int (**)(Game::VictoryClamHolding *clam))(*(u64*)(clam) + (0x5B8)))(clam);
	}
	return 0;
}

void playReportHook(nn::prepo::PlayReport *playReport, nn::account::Uid const& uid){
	if(playReport != NULL){
		playReport->Save(uid);
	}
}

static int mRushEndFrame = 0;

bool isInKingSquidHook(Game::Player *player){
	return player->isInSpecial_KingSquid_Impl(0) or krakenDiveHook(player);
}

bool krakenDiveHook(Game::Player *player){
	Starlion::PlayerKingSquid *kingSquid = ((Starlion::PlayerKingSquid*)player->mPlayerKingSquid);
	if(kingSquid == NULL){
		return 0;
	}
	return kingSquid->mIsRush and Prot::ObfLoad(&player->mAerialState) != 0 and player->isInSpecial_KingSquid_Impl(0);
}

// void sceneChanger(){
// 	// handle scene changer toggle
//     static bool sceneChanger = false;
//     if (Collector::mController.isPressed(Controller::Buttons::LStick)) {
//         sceneChanger = !sceneChanger;
// 	sead::SafeStringBase<char> sceneName;
//     sceneName.mCharPtr = (char*)"Lobby/Lan";
//     Lp::Utl::reqChangeScene(sceneName, NULL);
// 	PlayEnable();
//     }
// }

void PlayEnable(){
	Game::PlayerMgr *playerMgr = Collector::mPlayerMgrInstance;
	if(playerMgr != NULL){
		Game::Player* player = playerMgr->getControlledPerformer();
		if(player != NULL){
			if(player->mPlayerEffect != NULL){
				player->mPlayerEffect->emitAndPlay_GetArmor();
			}
		}
	}
}

void PlaySuperArmorUse(){
	Game::PlayerMgr *playerMgr = Collector::mPlayerMgrInstance;
	if(playerMgr != NULL){
		Game::Player* player = playerMgr->getControlledPerformer();
		if(player != NULL){
			if(player->mPlayerEffect != NULL){
				player->mPlayerEffect->emitAndPlay_SuperArmorUse();
			}
		}
	}
}

void PlaySuperArmorSt(){
	Game::PlayerMgr *playerMgr = Collector::mPlayerMgrInstance;
	if(playerMgr != NULL){
		Game::Player* player = playerMgr->getControlledPerformer();
		if(player != NULL){
			if(player->mPlayerEffect != NULL){
				player->mPlayerEffect->emitAndPlay_SuperArmorSt();
				xlink2::Handle tmp;
                player->mXLink->searchAndEmitWrap("AuraBody", false, &tmp);
			}
		}
	}
}

void PlayBarrierOn(){
	Game::PlayerMgr *playerMgr = Collector::mPlayerMgrInstance;
	if(playerMgr != NULL){
		Game::Player* player = playerMgr->getControlledPerformer();
		if(player != NULL){
			if(player->mPlayerEffect != NULL){
				player->mPlayerEffect->emitAndPlay_BarrierOn();
			}
		}
	}
}

void PlayFreeBombsEffect(){
	Game::PlayerMgr *playerMgr = Collector::mPlayerMgrInstance;
	if(playerMgr != NULL){
		Game::Player* player = playerMgr->getControlledPerformer();
		if(player != NULL){
			if(player->mPlayerEffect != NULL){
				xlink2::Handle tmp;
                player->mXLink->searchAndEmitWrap("RollerDash", false, &tmp);
			}
		}
	}
}

void PlaySuperArmorVanish(){
	Game::PlayerMgr *playerMgr = Collector::mPlayerMgrInstance;
	if(playerMgr != NULL){
		Game::Player* player = playerMgr->getControlledPerformer();
		if(player != NULL){
			if(player->mPlayerEffect != NULL){
				player->mPlayerEffect->emitAndPlay_SuperArmorVanish();
			}
		}
	}
}

void handlePlayerControl(){
    Cmn::PlayerCtrl *playerCtrl = Collector::mPlayerCtrlInstance;
    Game::PlayerMgr *playerMgr = Collector::mPlayerMgrInstance;
    if(playerMgr == NULL) return; // Null check
    
    Game::Player *player = playerMgr->getControlledPerformer();
    if (player == NULL) return;

    Game::PlayerBehindCamera *playerCamera = player->mPlayerBehindCamera;
    if (playerCamera == NULL) return;

    Game::PlayerGamePadData::FrameInput input;
    // input.record(); // grab input data

    // handle freefly toggle
//     static bool freefly = false;
//     if (Collector::mController.isPressed(Controller::Buttons::LStick)) {
//         freefly = !freefly;
//     }
    
//     static bool entered = false;
//     if(freefly){
//         mTextWriter->printf("Enabled Freefly!\n");
// 		PlayEnable();
//         static float x, y, z;
//         sead::Vector3<float> *playerPos = &player->mPosition;
//         if(!entered){
//             x = playerPos->mX;
//             y = playerPos->mY;
//             z = playerPos->mZ;
//         }

//         int speed = 10;

//         sead::Vector3<float> camPos = *playerCamera->mPosition;
//         sead::Vector3<float> camLookAtPos = playerCamera->mLookAt;
//         if(Collector::mController.isHeld(Controller::Buttons::B))
//             y+=speed / 2;
//         if(Collector::mController.isHeld(Controller::Buttons::ZL))
//             y-=speed / 2;
//         x-=playerCtrl->lpController->leftStick.mY * speed * sinf(atan2f(camPos.mX - camLookAtPos.mX, camPos.mZ - camLookAtPos.mZ));
//         z-=playerCtrl->lpController->leftStick.mY * speed * cosf(atan2f(camPos.mX - camLookAtPos.mX, camPos.mZ - camLookAtPos.mZ));
//         x+=playerCtrl->lpController->leftStick.mX * speed * cosf(atan2f(camPos.mX - camLookAtPos.mX, camPos.mZ - camLookAtPos.mZ));
//         z-=playerCtrl->lpController->leftStick.mX * speed * sinf(atan2f(camPos.mX - camLookAtPos.mX, camPos.mZ - camLookAtPos.mZ));
//         if(speed < 0)
//             speed = 0;

//         playerPos->mX = x;
//         playerPos->mY = y;
//         playerPos->mZ = z;
//         entered = true;
//     }
//     else{
//         entered = false;
// 		PlayDisable();
//     }
}

void barrierEffectNameHook(){
	const char *poop = "SWpInkArmorPreBreak";
	const char *poop2;
	poop2 = poop;
	asm("MOV X2, X0");
}

void sead::Random::init(u32 seed){
    const u32 mt_constant = 0xCEB9D8D9;
    mSeed1 = mt_constant * (seed ^ (seed >> 30u)) + 1;
    mSeed2 = mt_constant * (mSeed2 ^ (mSeed2 >> 30u)) + 2;
    mSeed3 = mt_constant * (mSeed3 ^ (mSeed3 >> 30u)) + 3;
    mSeed4 = mt_constant * (mSeed4 ^ (mSeed4 >> 30u)) + 4;
}

void enl::PiaMatchmakeCondition::makeRandomCryptoKey(int matchamekconditin,const u64 deword[], int hola){


}

void autoMatchTest(){


}

bool rivalOctohook(Cmn::Def::PlayerModelType modeltyape){
    if(modeltyape == Cmn::Def::PlayerModelType::Rival or modeltyape == Cmn::Def::PlayerModelType::RivalOcta){
        FsLogger::LogFormatDefaultDirect("[Gamblitz] rival what the actual fuck\n");
        return true;
    }
    FsLogger::LogFormatDefaultDirect("[Gamblitz] nonono\n");
        return false;
};

int main(int arg, char **argv){

}