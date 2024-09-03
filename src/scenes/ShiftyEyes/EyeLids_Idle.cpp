#include "EyeLids_Idle.h"
#include "ShiftyEyesScene.h"
#include "Eyelids_Blink.h"
#include "Eyelids_Glare.h"

void EyeLids_Idle::update(uint32_t) {
    uint8_t eyelidPosition = scene.eyes.getEyelidPosition();

    if (eyelidPosition > 0) {
        eyelidPosition -= 1;
        scene.eyes.setEyelidPosition(eyelidPosition);    
    } 
}

FSMState* EyeLids_Idle::transition() {
    Gamepad& gamepad = getDevice().gamepad;

    // Wait until almost back up before allowing transitions
    if (scene.eyes.getEyelidPosition() > 1) {
        return nullptr;
    }

    if (gamepad.wasPressed(gamepad.buttonZ)) {
        return new EyeLids_Blink(scene);
    }
    else if (gamepad.wasPressed(gamepad.buttonC)) {
        return new EyeLids_Glare(scene);
    }

    return nullptr;
}
