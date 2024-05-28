#pragma once

#include "types.h"
#include "agl/TextureData.h"
#include "sead/vector.h"
#include "sead/camera.h"
#include "agl/utl/devtools.h"
#include "agl/DrawContext.h"
#include "Cmn/CameraUtl.h"
#include "sead/textwriter.h"
#include "string.h"
#include <stdarg.h>
#include <cstring>
#include <stdio.h>
#include "Cmn/Cmn.h"
#include "Cmn/ActorPicker.h"
#include "sead/string.h"
#include "Lp/Ui.h"
#include <queue>
#include "flexlion/Utils.hpp"
#include "flexlion/FileReader.hpp"

class DrawUtils
{
    public:
    enum BoundBoxDrawType{
        Default, Flat, Clean2D, TudouCool
    };
    static int drawLogo(bool isInformToEnd);
    static void setDrawContext(agl::DrawContext *mDrawContext);
    static void setColor(sead::Color4f color);
    static float getSymbolWidthDef(float scale);
    static float getSymbolHeightDef(float scale); // Can be used as getWriterScale
    static float getSymbolWidth();
    static float getSymbolHeight(); // Can be used as getWriterScale
    static float getTextWidth(const char* text);
    static float getTextHeight(int linenum);
    static void drawTexture(agl::TextureData *data, sead::Vector2<float> startPos, sead::Vector2<float> resolution, float scale, sead::Color4f color);
    static void fill2dBoxTexture(agl::TextureData *data, sead::Vector2<float> startPos, float height, float width);
    static void makeTudou();
    static void dbgPrintActorInfo();
};