#include "Pupils_Disconnected.h"
#include "ShiftyEyesScene.h"
#include "Color.h"

namespace {
    const int32_t gazeTimeMin = 100;
    const int32_t gazeTimeMax = 250;
    const int32_t countdownMax = 2000;
}

void Pupils_Disconnected::enter() {
        pupilX = nextPupilX = scene.eyes.getPupilX();
        pupilY = nextPupilY = scene.eyes.getPupilY();
        dX = 0;
        dY = 0;
        gazeTime = gazeTimeMin;
        gazeCountdown = gazeTime;
}

void Pupils_Disconnected::update(uint32_t dt) {
    gazeCountdown -= dt;

    if (gazeCountdown <= gazeTime) {
        // Pupils are moving, set interpolated position.
        scene.eyes.setPupilX(nextPupilX - (dX * gazeCountdown / gazeTime));
        scene.eyes.setPupilY(nextPupilY - (dY * gazeCountdown / gazeTime));

        if (gazeCountdown <= 0) {
            chooseNextPupilPosition();
        }
    }
    else {
        // Pupils are stationary.
        scene.eyes.setPupilPosition(pupilX, pupilY);
    }    
}

void Pupils_Disconnected::chooseNextPupilPosition() {
    // We've arrived at the next pupil position.
    pupilX = nextPupilX;
    pupilY = nextPupilY;        

    // Pick a new position, staying within bounds.
    if (scene.hasMonsterPupils) {
        nextPupilX = random(7) * 3;
        nextPupilX = max(3, min(nextPupilX, 20));

        nextPupilY = random(4) * 3 - 3;
        nextPupilY = max(-2, min(nextPupilY, 7));
    } 
    else {
        nextPupilX = random(5) * 3 + 3;
        nextPupilY = random(4) * 3;
    }

    // Distance to next pupil position.
    dX = nextPupilX - pupilX;
    dY = nextPupilY - pupilY;

    // Duration of eye movement.
    gazeTime = random(gazeTimeMin, gazeTimeMax);

    // Count to end of next pupil movement.
    gazeCountdown = random(gazeTime, countdownMax);
}
