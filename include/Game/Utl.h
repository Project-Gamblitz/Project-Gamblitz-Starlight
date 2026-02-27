#pragma once

#include "types.h"
#include "Game/Player/Player.h"

namespace Game {
    namespace Utl {
        enum class ActorFactoryType {
            EnemyMgr = 4,
        };
        class ActorFactoryBase{
            public:
            virtual bool isNoActor() const;
        };
        class ActorFactoryMgr{
            public:
            static void registFactory(const ActorFactoryBase *factory);
        };
        template<typename T>
        class ActorFactory : public ActorFactoryBase{
            public:
            using CreateFuncT = Lp::Sys::Actor* (*)(Lp::Sys::Actor*);

            CreateFuncT mCreateFunc;
            ActorFactoryType mTypeId;
            sead::SafeStringBase<char> mName;

            static Lp::Sys::Actor* createFunc_(Lp::Sys::Actor *parent){
                return Lp::Sys::Actor::createCstm<T>(parent);
            };

            ActorFactory(ActorFactoryType typeId, sead::SafeStringBase<char> name){
                mCreateFunc = &createFunc_;
                mTypeId = typeId;
                mName = name;
                ActorFactoryMgr::registFactory(this);
            };
        };
        extern bool isRivalOrRivalOcta(Game::Player const&);
    };
};