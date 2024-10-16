// Adapted from MIT-licensed code at
// https://learn.adafruit.com/adafruit-eyelights-led-glasses-and-driver/bluetooth-message-scroller
// by Phil "Paint Your Dragon" Burgess for Adafruit Industries

#pragma once

#include <Arduino.h>
#include "Scene.h"
#include "Color.h"

class MarqueeScene: public Scene {
public:
    MarqueeScene(Device& d);
    virtual ~MarqueeScene() = default;

    virtual void enter() override;
    virtual void update(uint32_t dt);
    virtual void receivedColor(const Color::RGB& c) override;
    virtual void receivedText(const char* text) override;

private:
    void resetScroll();
    uint8_t charWidth(uint8_t c) const;
    uint16_t textWidth(const char* str);

private:
    static constexpr size_t messageBufferSize = Settings::textBufferSize;
    char messageBuffer[messageBufferSize] = {0};

    int32_t messageWidth = 0;
    int32_t scrollPosition = 0;
    uint32_t scrollElapsed = 0;
    uint16_t startHue = 0;

    bool useCustomColor = false;
    float hue = 0;
    uint8_t saturation = 255;

    static constexpr float minScrollDelay = 15.0;
    static constexpr float defaultScrollDelay = 25.0;    
    static constexpr float maxScrollDelay = 35.0;
    float scrollDelay = defaultScrollDelay;
    bool isAdjustingScrollDelay = false;
};
