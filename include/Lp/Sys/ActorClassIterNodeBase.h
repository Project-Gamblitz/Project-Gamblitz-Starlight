#include "types.h"

namespace Lp {
  namespace Sys {
    class Actor;
    class ActorClassIterNodeBase{
		  public:
			Lp::Sys::Actor* derivedFrontActor();
			Lp::Sys::Actor* derivedNextActor(Lp::Sys::Actor *);
      Lp::Sys::Actor *derivedFrontActiveActor();
      Lp::Sys::Actor *derivedNextActiveActor(Lp::Sys::Actor *);
      int derivedSize(){
        int size = 0;
        for(Lp::Sys::Actor *a = this->derivedFrontActor(); a != NULL; a = this->derivedNextActor(a)){
          size+=1;
        }
        return size;
      }
		
      _BYTE _0[0x40];
      u32 mActorNum;
      };
  };
};