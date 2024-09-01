#pragma once

#include <Arduino.h>
#include "SceneState.h"

class ShiftyEyesScene;

class EyeLids_Glare: public SceneState<ShiftyEyesScene> {
public:

    EyeLids_Glare(ShiftyEyesScene& s)
        : SceneState(s)
    {
    }

    virtual void enter() override;
    virtual void update(uint32_t dt) override;
    virtual FSMState* transition() override;

private:
    int32_t animationTimer = 0;
};