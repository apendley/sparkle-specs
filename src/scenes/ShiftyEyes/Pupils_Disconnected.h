// Adapted from the MIT-licensed "roboface" example in the Adafruit LED backpack library at
// https://github.com/adafruit/Adafruit_LED_Backpack/blob/master/examples/roboface/roboface.ino
// by Phil "Paint Your Dragon" Burgess for Adafruit Industries

#pragma once

#include <Arduino.h>
#include "SceneState.h"

class ShiftyEyesScene;

class Pupils_Disconnected: public SceneState<ShiftyEyesScene> {
public:

    Pupils_Disconnected(ShiftyEyesScene& s)
        : SceneState(s)
    {
    }

    virtual void enter();
    virtual void update(uint32_t dt);

private:
    bool isEyePositionValid(int8_t dx, int8_t dy) {
        return (dx * dx + dy * dy) >= 8;
    }

    void chooseNextPupilPosition();

private:
    int8_t pupilX;
    int8_t pupilY;
    int8_t nextPupilX;
    int8_t nextPupilY;
    int8_t dX;
    int8_t dY;
    int32_t gazeTime;
    int32_t gazeCountdown;
};