#pragma once

namespace Lp {

  namespace Sys {

      class DbgTextWriter
      {
        public:
        static Lp::Sys::DbgTextWriter *sInstance;
        struct Info;
        class ArgEx{

        };
        void pilotEntry(int,sead::Vector2<float> const&,Lp::Sys::DbgTextWriter::ArgEx const*,char const*);
        void pilotEntryF(int,sead::Vector2<float> const&,Lp::Sys::DbgTextWriter::ArgEx const*,char const*,...);
      };
  };
};