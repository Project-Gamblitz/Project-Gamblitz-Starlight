#include "starlight/input.hpp"

namespace starlight {

    Controller::Controller() {}

    Controller::Controller(Lp::Sys::Ctrl *controller){
        mController = controller;
    }

    void Controller::update(){
        for(int i = 19; i > 0; i--){
            mLastPoll[i] = mLastPoll[i - 1];
        }
        mLastPoll[0] = mController->data;
        mCntr-=1;
        if(mCntr < 0){
            mCntr = 5;
        }
    }

    u32 Controller::isHeld(Buttons inputs) {
        return (mController->data & inputs) != 0;
    }

    u32 Controller::wasPressed(Buttons inputs) {
        bool buttonHeld = mController->data & inputs;
        return (!buttonHeld & (mController->data & mLastPoll[0] & inputs)) != 0;
    }

    u32 Controller::isPressed(Buttons inputs) {
        bool buttonHeld = mController->data & inputs;
        return (buttonHeld & !(mController->data & mLastPoll[0] & inputs)) != 0;
    }

    u32 Controller::isPressedWithRepeat(Buttons inputs){
        bool val = (mController->data & inputs);
        for(int i = 0; i < 20; i++){
            val = val and (mLastPoll[i] & inputs);
        }
        return ((this->isPressed(inputs)) or (val and (mCntr == 0)));
    }
};