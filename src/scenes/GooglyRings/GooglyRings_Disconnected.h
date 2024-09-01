#pragma once

#include <Arduino.h>
#include "SceneState.h"

class GooglyRingsScene;

class GooglyRings_Disconnected: public SceneState<GooglyRingsScene> {
public:

    GooglyRings_Disconnected(GooglyRingsScene& s)
        : SceneState(s)
    {
    }

    virtual ~GooglyRings_Disconnected() = default;

    virtual void enter() override;
    virtual void update(uint32_t) override;

private:
    // Fixed timestep since pendulum does not factor time into simulation.    
    static constexpr uint32_t frameInterval = 33;
    uint32_t frameElapsed = 0;
};
