#include "Pupils_Connected.h"
#include "ShiftyEyesScene.h"
#include "Color.h"

void Pupils_Connected::update(uint32_t dt) {
    Gamepad& gamepad = getDevice().gamepad;
    const Gamepad::Report& report = gamepad.getReport();

    // Invert x so that left matches the viewer's left.
    int8_t x = -report.x;    
    uint8_t xPupil = map(x, -127, 127, 1, 5);

    // Bias downward to position pupils in visual center of eyes, 
    // since the LEDs do not extend all the way to the bottom.
    // Also, invert y so that down matches the viewer's down.
    int8_t y = min(-report.y + 48, 127);
    uint8_t yPupil = map(y, -127, 127, 0, 3);    

    scene.eyes.setPupilPosition(xPupil, yPupil);
}