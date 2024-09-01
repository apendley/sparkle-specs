#pragma once

#include <Arduino.h>
#include "Scene.h"
#include "Color.h"

class SparklesScene: public Scene {
public:
    SparklesScene(Device& d);
    virtual ~SparklesScene() = default;

    virtual void enter() override;
    virtual void exit() override;
    virtual void update(uint32_t dt);
    virtual void receivedColor(const Color::RGB& c) override;

private:
    void draw();
    void newSparkle(const Color::HSV& hsv, int16_t ttl);

private:
    static constexpr float intensityDeadZone = 10.0;

    static constexpr float intensityMinInvScale = 75.0;

    static constexpr float intensityMin = 0.0;
    static constexpr float intensityMax = 1.0;

    static constexpr uint16_t minSpawnInterval = 100;
    static constexpr uint16_t maxSpawnInterval = 300;

    static constexpr int16_t minTimeToLive = 400;
    static constexpr int16_t maxTimeToLive = 800;
    static constexpr uint8_t minBrightness = 80;
    static constexpr uint8_t maxBrightness = 150;

    struct Sparkle {
        Sparkle() { }

        bool isActive = false;
        uint8_t x = 0;
        uint8_t y = 0;
        int16_t timeToLive = 0;
        int16_t duration = 0;
        Color::HSV hsv = Color::HSV(0, 255, 255);
    };

    static constexpr uint16_t maxSparkles = 255;
    Sparkle sparkles[maxSparkles];
    int32_t newSparkleTimer = 0;
    float intensityInvScale = intensityMinInvScale;
    float lastMagnitude = 0;
    float currentHue = 0;

    bool useCustomColor = false;
    float hue = 0;    
};
