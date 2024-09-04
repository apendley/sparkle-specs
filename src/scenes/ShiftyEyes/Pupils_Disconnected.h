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
    void chooseNextPupilPosition();

private:
    int8_t xPupil;
    int8_t yPupil;
    int8_t xPupilNext;
    int8_t yPupilNext;
    int8_t dX;
    int8_t dY;
    int32_t gazeTime;
    int32_t gazeCountdown;
};