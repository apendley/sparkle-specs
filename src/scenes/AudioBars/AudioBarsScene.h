// Adapted from MIT-licensed code at
// https://learn.adafruit.com/adafruit-eyelights-led-glasses-and-driver/music-reactive-lights-2
// by Phil "Paint Your Dragon" Burgess for Adafruit Industries

#pragma once

#include <Arduino.h>
#include "Scene.h"
#include "ColumnSpectrumizer.h"
#include "PdmRecorder.h"

class AudioBarsScene: public Scene {
public:
    AudioBarsScene(Device& d);
    virtual ~AudioBarsScene() = default;

    virtual void enter() override;
    virtual void exit() override;
    virtual void update(uint32_t dt);
    virtual void receivedColor(const Color::RGB& c) override;

private:
    void draw();

private:
    static constexpr int columnCount = 16;
    ColumnSpectrumizer<columnCount, PdmRecorder::sampleCount> spectrumizer;

    uint16_t columnColors[columnCount];

    bool useCustomColor = false;
    float hue = 0;
    uint8_t saturation = 255;
    uint8_t brightness = 40;

    bool snowCapped = true;
};
