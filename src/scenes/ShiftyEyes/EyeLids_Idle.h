#pragma once

#include <Arduino.h>
#include "SceneState.h"

class ShiftyEyesScene;

class EyeLids_Idle: public SceneState<ShiftyEyesScene> {
public:

    EyeLids_Idle(ShiftyEyesScene& s)
        : SceneState(s)
    {
    }

    virtual void update(uint32_t) override;
    virtual FSMState* transition() override;

private:
    uint32_t blinkCountdown;
};