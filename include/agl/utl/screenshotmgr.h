#pragma once

#include "types.h"
#include "agl/utl/parameter.h"
#include "agl/TextureData.h"
#include "agl/DrawContext.h"
#include "Lp/Sys/heapgroup.h"
#include "Lp/Utl.h"

namespace agl {
  class TextureDataSerializerTGA{
    public:
    enum TGAFormat{
      DEFAULT
    };
  };
  namespace utl {
      class ScreenShotMgr {
          public:
            static agl::utl::ScreenShotMgr *sInstance;
            static void createInstance(sead::Heap *);
            void initialize(sead::Heap *);
            void reserveCapture_(char *filename){
              mIsReserveCapture = 1;
              _40 = 0;
              mTgaFormat = 2;
              mHeap = Lp::Utl::getSceneHeap();
              _58 = 0;
              strncpy(mFileName.mCharPtr, filename, mFileName.mBufferSize);
            };
            void captureToTexture_(agl::DrawContext *, const agl::TextureData *) const;
            void captureToFile_(agl::DrawContext *, agl::TextureData const*, agl::TextureDataSerializerTGA::TGAFormat, sead::SafeStringBase<char> const&, sead::Heap *)const;
            void addTimeStamp_(sead::FixedSafeString<256> *, sead::SafeStringBase<char>, bool)const;
          _BYTE _0[0x30];
          bool mIsReserveCapture;
          int mTgaFormat;
          u64 _38;
          bool _40;
          sead::Heap *mHeap;
          u64 _50;
          bool _58;
          _BYTE _59[0x178 - 0x59];
          sead::BufferedSafeStringBase<char> mFileName;
      };
  }; 
};