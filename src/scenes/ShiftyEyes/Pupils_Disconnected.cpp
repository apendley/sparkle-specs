#include "Pupils_Disconnected.h"
#include "ShiftyEyesScene.h"
#include "Color.h"

namespace {
    const int32_t gazeTimeMin = 100;
    const int32_t gazeTimeMax = 250;
    const int32_t countdownMax = 2000;
}

void Pupils_Disconnected::enter() {
        xPupil = xPupilNext = scene.xPupil;
        yPupil = yPupilNext = scene.yPupil;
        dX = 0;
        dY = 0;
        gazeTime = gazeTimeMin;
        gazeCountdown = gazeTime;
}

void Pupils_Disconnected::update(uint32_t dt) {
    gazeCountdown -= dt;

    if (gazeCountdown <= gazeTime) {
        // Pupils are moving, set interpolated position.
        scene.xPupil = xPupilNext - (dX * gazeCountdown / gazeTime);
        scene.yPupil = yPupilNext - (dY * gazeCountdown / gazeTime);

        if (gazeCountdown <= 0) {
            chooseNextPupilPosition();
        }
    }
    else {
        // Pupils are stationary.
        scene.xPupil = xPupil;
        scene.yPupil = yPupil;
    }    
}

void Pupils_Disconnected::chooseNextPupilPosition() {
    // We've arrived at the next pupil position.
    xPupil = xPupilNext;
    yPupil = yPupilNext;        

    // Pick a new position, staying within bounds.
    if (scene.hasMonsterPupils) {
        xPupilNext = random(7) * 3;
        xPupilNext = max(3, min(xPupilNext, 20));

        yPupilNext = random(4) * 3 - 3;
        yPupilNext = max(-2, min(yPupilNext, 7));
    } 
    else {
        xPupilNext = random(5) * 3 + 3;
        yPupilNext = random(4) * 3;
    }

    // Distance to next pupil position.
    dX = xPupilNext - xPupil;
    dY = yPupilNext - yPupil;

    // Duration of eye movement.
    gazeTime = random(gazeTimeMin, gazeTimeMax);

    // Count to end of next pupil movement.
    gazeCountdown = random(gazeTime, countdownMax);
}
