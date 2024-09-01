// Adapted from MIT-licensed code at
// https://learn.adafruit.com/adafruit-eyelights-led-glasses-and-driver/googly-eye-rings    

#pragma once

#include <Arduino.h>
#include "Scene.h"
#include "FSM.h"
#include "Pendulum.h"

class GooglyRingsScene: public Scene {
public:
    GooglyRingsScene(Device& d);
    virtual ~GooglyRingsScene() = default;

    virtual void enter() override;
    virtual void update(uint32_t dt);
    virtual void gamepadConnected() override;
    virtual void gamepadDisconnected() override;
    virtual void receivedColor(const Color::RGB& c) override;

    FSM fsm;
    Pendulum leftPendulum;
    Pendulum rightPendulum;
    float hue = (65536 / 6) * 5;
    uint8_t saturation = 255;
    uint8_t brightness = 150;

private:
    void draw();
};
