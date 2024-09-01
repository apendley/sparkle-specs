#pragma once

#include <Arduino.h>
#include "SceneState.h"

class BeamScene;

class Beam_Disconnected: public SceneState<BeamScene> {
public:

    Beam_Disconnected(BeamScene& s)
        : SceneState(s)
    {
    }

    virtual ~Beam_Disconnected() = default;

    virtual void enter() override;
    virtual void update(uint32_t) override;

private:
	float direction = 1;
    uint8_t speedIndex = 0;
};
