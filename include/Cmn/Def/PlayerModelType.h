#pragma once

#include "types.h"
#include "Sex.h"

namespace Cmn {
    namespace Def {
        enum class PlayerModelType {
            InkGirl = 0,
            InkBoy = 1,
            OctGirl = 2,
            OctBoy = 3,
			HeroGirl = 4,
			HeroBoy = 5,
			Rival = 6,
			RivalOcta = 7
        };
		static Cmn::Def::Sex modelTypeToSex(Cmn::Def::PlayerModelType type){
            int a = int(type);
            if(a == 1 or a == 3 or a == 5){
                return Cmn::Def::Sex::Male;
            }
            return Cmn::Def::Sex::Female;
        }
    };
}; 