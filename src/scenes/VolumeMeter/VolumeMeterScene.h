// Inspired by the NeoPixel VU Meter project at
// https://learn.adafruit.com/neopixel-mini-vu-meter

#pragma once

#include <Arduino.h>
#include "Scene.h"

class VolumeMeterScene: public Scene {
public:
    VolumeMeterScene(Device& d);
    virtual ~VolumeMeterScene() = default;

    virtual void enter() override;
    virtual void exit() override;
    virtual void update(uint32_t dt);
    virtual void receivedColor(const Color::RGB& c) override;

private:
    void draw();

private:
    static constexpr float deadZone = 10.0;
    static constexpr float invScale = 175.0;
    static constexpr float minValue = 1.0;
    static constexpr float maxValue = 24.0;    

    float lastMagnitude = 0.0;
    float magnitude = 0.0;

    bool useCustomColor = false;
    float hue = (65536 / 6) * 3.0;
    float saturation = 255;
    uint8_t brightness = 128;    
};
