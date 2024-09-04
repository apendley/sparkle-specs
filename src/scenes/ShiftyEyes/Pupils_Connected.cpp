#include "Pupils_Connected.h"
#include "ShiftyEyesScene.h"
#include "Color.h"

void Pupils_Connected::update(uint32_t dt) {
    Gamepad& gamepad = getDevice().gamepad;
    const Gamepad::Report& report = gamepad.getReport();

    if (scene.hasMonsterPupils) {
        int8_t xPupil = map(-report.x, -127, 127, -1, 16) + 2;
        xPupil = max(xPupil, 3);
        
        int8_t yPupil = map(-report.y, -127, 127, -2, 10); 
        yPupil = min(yPupil, 7);

        scene.xPupil = xPupil;
        scene.yPupil = yPupil;
    } 
    else {
        int8_t xPupil = map(-report.x, -127, 127, 1, 17);

        int8_t yPupil = map(-report.y, -127, 127, -3, 11) + 2;
        yPupil = min(yPupil, 11);

        scene.xPupil = xPupil;
        scene.yPupil = yPupil;
    }
}