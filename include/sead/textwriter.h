/**
 * @file viewport.h
 * @brief Basic Viewport class.
 */

#pragma once

#include "types.h"
#include "sead/projection.h"
#include "sead/bound.h"
#include "sead/vector.h"
#include "sead/font.h"

namespace sead
{
    class TextWriter
    {
        public:
            virtual ~TextWriter();

            void printImpl_(char const*, int, bool, sead::BoundBox2<float>*);
            void printf(char const*, ...);
            //void printf(char16_t const*, ...);
            void calcFormatStringRect(sead::BoundBox2<float> *, char const*, ...);
            void calcFormatStringRect(sead::BoundBox2<float> *, char16_t const*, ...);
            void setScaleFromFontHeight(float arg){
                this->mScale.mX = arg / 16.0f;
                this->mScale.mY = arg / 16.0f;
            };
            void beginDraw();
            void setCursorFromTopLeft(sead::Vector2<float> const&);
            void getCursorFromTopLeft(sead::Vector2<float> *) const;
            static void setupGraphics(sead::DrawContext*);
            static void setDefaultFont(sead::FontBase *); // 'GambitPic.bffnt'

            sead::Viewport *mViewport;
            sead::Projection *mProjection;
            sead::Camera *mCamera;
            sead::FontBase *mFont;
            int TextWriter_x28;
            int TextWriter_x2C;
            sead::Vector2<float> mScale;
            sead::Color4f mColor;
            int TextWriter_x48;
            float mLineSpace;
            sead::BoundBox2<float> *mBoundBox2;
            int TextWriter_x58;
            int TextWriter_x5C;
            char16_t *mFormatBuffer;
            int mFormatBufferSize;
            int TextWriter_x6C;
            sead::DrawContext *mDrawContext;
    };
};