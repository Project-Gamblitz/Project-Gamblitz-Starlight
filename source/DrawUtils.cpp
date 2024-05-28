#include "flexlion/DrawUtils.hpp"

static agl::DrawContext *mDrawCtx;
static sead::TextWriter *mWriter;
static sead::Color4f mColor;
agl::TextureData *mTudouTex;
agl::TextureData *mEllipse;
agl::TextureData *mCredits;
agl::TextureData *mRules;
agl::TextureData *mMainFrame;
//agl::TextureData *mNoIdent;
//agl::TextureData *mFoundIdent;

void DrawUtils::setDrawContext(agl::DrawContext *mDrawContext){
    mDrawCtx = mDrawContext;
}

void DrawUtils::setColor(sead::Color4f color){
    mColor = color;
}

const float mainTextScale = 24.0f;
const float subTextScale = 16.0f;
static float animfade = 0.0f;
static float animSpeed = 0.12f;
int DrawUtils::drawLogo(bool isInformToEnd){
    animfade += (float(isInformToEnd ^ 1) - animfade) * 0.13f;
    if(animfade > 1.0f){
        animfade = 1.0f;
    }
    if(animfade < 0.0f){
        animfade = 0.0f;
    }
    sead::Color4f color = sead::Color4f::cWhite;
    color.a = animfade;
    DrawUtils::setColor(color);
    DrawUtils::fill2dBoxTexture(mMainFrame, sead::Vector2<float>::zero, SCREEN_H, SCREEN_W);
    DrawUtils::fill2dBoxTexture(mCredits, {SCREEN_W - 443.0f * animfade, 70.0f}, 524.0f, 443.0f);
    DrawUtils::fill2dBoxTexture(mRules, {-388.0f * (1.0f - animfade), 70.0f}, 524.0f, 449.0f);
    // if(ServerClient::sInstance->hasident) DrawUtils::fill2dBoxTexture(mFoundIdent, sead::Vector2<float>::zero, SCREEN_H, SCREEN_W);
    // else DrawUtils::fill2dBoxTexture(mNoIdent, sead::Vector2<float>::zero, SCREEN_H, SCREEN_W);
    if(isInformToEnd and animfade <= 0.06f){
        return 1;
    }
    return 0;
}

void DrawUtils::fill2dBoxTexture(agl::TextureData *data, sead::Vector2<float> startPos, float height, float width){
    if(data == NULL){
        return;
    }
    startPos.mX+=width*0.5f;
    startPos.mY+=height*0.5f;
    DrawUtils::drawTexture(data, startPos, {width, height}, width / SCREEN_W * SCREEN_H * 2.0f, mColor);
}


void DrawUtils::drawTexture(agl::TextureData *data, sead::Vector2<float> startPos, sead::Vector2<float> resolution, float scale, sead::Color4f color){
    scale/=SCREEN_H;
    startPos = {-1.0f + 2.0f * startPos.mX / SCREEN_W, -1.0f + 2.0f * startPos.mY / SCREEN_H};
    agl::TextureSampler sampler = agl::TextureSampler(*data);
    sead::Matrix34<float> mtx = sead::Matrix34<float>::ident;
    sead::Matrix44<float> customProj = sead::Matrix44<float>::ident;
    customProj.matrix[0][0] = scale;
    customProj.matrix[1][1] = SCREEN_W / SCREEN_H * resolution.mY / resolution.mX * scale;
    customProj.matrix[2][2] = -1.0f;
    customProj.matrix[2][3] = -1.0f;
    customProj.matrix[3][2] = -4.0f;
    mtx.matrix[0][3] = startPos.mX / scale;
    mtx.matrix[1][3] = startPos.mY / scale * SCREEN_H / SCREEN_W * resolution.mX / resolution.mY;
    agl::utl::DevTools::drawTexture(mDrawCtx, sampler, mtx, customProj, color);
}

void DrawUtils::makeTudou(){
    mEllipse = Utils::getTextureDataFromModelArc("Gamblitz", "ellipse");
    mMainFrame = Utils::getTextureDataFromModelArc("Gamblitz", "MainFrame");
    //mNoIdent = Utils::getTextureDataFromModelArc("Gamblitz", "NoIdent");
    //mFoundIdent = Utils::getTextureDataFromModelArc("Gamblitz", "FoundIdent");
    mCredits = Utils::getTextureDataFromModelArc("Gamblitz", "Credits");
    mRules = Utils::getTextureDataFromModelArc("Gamblitz", "Rules");
}

void DrawUtils::dbgPrintActorInfo(){
    Lp::Sys::ActorClassIterNodeBase *iterNode = Cmn::Actor::getClassIterNodeStatic();
    Cmn::Actor *actor = (Cmn::Actor *)iterNode->derivedFrontActiveActor();
    while(actor != NULL){
        ((Cmn::ActorPicker*)(NULL))->entryInfo(actor, Cmn::GfxUtl::getLyrIdx_3D_Main());
        actor = (Cmn::Actor *)iterNode->derivedNextActiveActor(actor);
    }
}