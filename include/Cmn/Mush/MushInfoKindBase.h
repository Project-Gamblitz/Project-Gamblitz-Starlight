#pragma once

#include "types.h"

namespace Cmn {
    class MushSubHolder{
        public:
        u64 data;
        int _8;
        int _0xC;
        int mEntryNum;
        u64 getByIndex(u64 size, int index){
            u32 a1 = _8;
            u32 a2 = _0xC + index;
            if(a2 < a1){
                a1 = 0;
            }
            return (u64)(data + (size * (a2 - a1)));
        }
    };
    template <class T, class K> class MushInfoWithKindBaseSimple {
        public:
            T* getAtRandom(K) const;
    };
    template <class T, class K> class MushInfoWithKindBase : public MushInfoWithKindBaseSimple<T, K>  {
        public:
            T* getById(K, int) const;    
    };
};