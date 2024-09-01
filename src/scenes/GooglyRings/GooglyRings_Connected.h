#pragma once

#include <Arduino.h>
#include "SceneState.h"

class GooglyRingsScene;

class GooglyRings_Connected: public SceneState<GooglyRingsScene> {
public:

    GooglyRings_Connected(GooglyRingsScene& s) : SceneState(s) { }

    virtual ~GooglyRings_Connected() = default;

    virtual void enter() override;
    virtual void update(uint32_t) override;

private:
    // Fixed timestep since pendulum does not factor time into simulation.    
    static constexpr uint32_t frameInterval = 33;
    uint32_t frameElapsed = 0;
};
