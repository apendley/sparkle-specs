#pragma once

#include <Arduino.h>
#include "SceneState.h"

class ShiftyEyesScene;

class Pupils_Connected: public SceneState<ShiftyEyesScene> {
public:
    Pupils_Connected(ShiftyEyesScene& s)
        : SceneState(s)
    {
    }

    virtual void update(uint32_t dt);

};