#pragma once

#include "types.h"
#include "Game/Player/PlayerAnimCtrlSet.h"
namespace Game {
    class Player;
    class PlayerMotion {
        public:
        enum ResetMode{
            cDefault
        };
        enum AnimID {
            
        };
        void startEventAnim(Game::PlayerMotion::AnimID, float, float);
        void startUserDemoAnim(char const*,int,int);
        void startDemoAnim(char const*, float, float, bool);
        void reset(bool,Game::PlayerMotion::ResetMode);
        Game::Player *mPlayer;
        _BYTE _8[0x30];
        int animSeq_38;
        int animSeq_3C;
        Game::PlayerAnimCtrlSet *mPlayerAnimCtrlSet;
        Game::PlayerAnimCtrlSet *mSquidAnimCtrlSet;
        u64 _50;
        u32 mAnimByteSeq;
        _BYTE _5C[0x33B0 - 0x5C];
        Game::AnimSetController *mAnimSetCtrlSquid;
    };
};