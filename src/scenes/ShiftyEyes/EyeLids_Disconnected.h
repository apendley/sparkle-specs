#pragma once

#include <Arduino.h>
#include "SceneState.h"

class ShiftyEyesScene;

class EyeLids_Disconnected: public SceneState<ShiftyEyesScene> {
public:

    EyeLids_Disconnected(ShiftyEyesScene& s)
        : SceneState(s)
    {
    }

    virtual void enter() override;
    virtual void update(uint32_t) override;

private:
    int32_t blinkCountdown;
};