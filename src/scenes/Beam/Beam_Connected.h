#pragma once

#include <Arduino.h>
#include "SceneState.h"

class BeamScene;

class Beam_Connected: public SceneState<BeamScene> {
public:
    Beam_Connected(BeamScene& s)
        : SceneState(s)
    {
    }

    virtual ~Beam_Connected() = default;

    virtual void enter() override;
    virtual void update(uint32_t) override;
};
