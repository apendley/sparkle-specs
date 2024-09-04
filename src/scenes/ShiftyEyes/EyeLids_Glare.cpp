#include "EyeLids_Glare.h"
#include "ShiftyEyesScene.h"
#include "EyeLids_Idle.h"

namespace {
    const int32_t animationInterval = 15;
}

void EyeLids_Glare::enter() {
    animationTimer = 0;
}

void EyeLids_Glare::update(uint32_t dt) {
    animationTimer -= dt;

    if (animationTimer <= 0) {
        animationTimer += animationInterval;
        uint8_t eyelidPosition = scene.getEyelidPosition();

        if (eyelidPosition < 2) {
            eyelidPosition += 1;
            scene.setEyelidPosition(eyelidPosition);    
        }     
    }
}

FSMState* EyeLids_Glare::transition() {
    Gamepad& gamepad = getDevice().gamepad;

    if (gamepad.isUp(gamepad.buttonC)) {
        return new EyeLids_Idle(scene);
    }

    return nullptr;
}