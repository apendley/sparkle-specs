// Inspired by Cylons from Battlestar Galactica

#pragma once

#include <Arduino.h>
#include "Scene.h"
#include "FSM.h"
#include "GlassesBuffer.h"
#include "Color.h"

class BeamScene: public Scene {
public:
    enum class BeamMode: uint8_t {
        vertical = 0,
        horizontal,
        point
    };

    static constexpr size_t beamModeCount = uint8_t(BeamMode::point);

public:
    BeamScene(Device& d);
    virtual ~BeamScene() = default;

    virtual void enter() override;
    virtual void update(uint32_t dt) override;
    virtual void gamepadConnected() override;
    virtual void gamepadDisconnected() override;
    virtual void receivedColor(const Color::RGB& c) override;

    void gotoNextBeamMode();

    BeamMode beamMode = BeamMode::vertical;

    float xBeam = 0;
    float yBeam = 0;
    uint8_t fade = 240;
    bool instant = false;

    float hue = 0;
    uint8_t saturation = 255;
    uint8_t brightness = 160;

    FSM beamFSM;

private:
    void draw();

private:
    GlassesBuffer glassesBuffer;

    const float xSpeed = 80.0;
    const float ySpeed = 60.0;

    float xCurrent = 0;
    float yCurrent = 0;
};