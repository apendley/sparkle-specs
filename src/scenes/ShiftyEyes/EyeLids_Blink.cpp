#include "EyeLids_Blink.h"
#include "ShiftyEyesScene.h"
#include "EyeLids_Idle.h"

namespace {
    const int32_t animationInterval = 15;
}

void EyeLids_Blink::enter() {
    animationTimer = 0;
}

void EyeLids_Blink::update(uint32_t dt) {
    animationTimer -= dt;

    if (animationTimer <= 0) {
        animationTimer += animationInterval;

        uint8_t eyelidPosition = scene.getEyelidPosition();

        if (eyelidPosition < scene.maxEyelidPosition) {
            eyelidPosition += 1;
            scene.setEyelidPosition(eyelidPosition);    
        } 
    }
}

FSMState* EyeLids_Blink::transition() {
    if (getDevice().isGamepadConnected()) {
        Gamepad& gamepad = getDevice().gamepad;

        if (gamepad.isUp(gamepad.buttonZ)) {
            return new EyeLids_Idle(scene);
        }


    }
    return nullptr;	
}