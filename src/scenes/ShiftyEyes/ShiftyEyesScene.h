// Inspired by the various Adafruit eyes projects

#pragma once

#include <Arduino.h>
#include "Scene.h"
#include "FSM.h"

class ShiftyEyesScene: public Scene {
public:
    ShiftyEyesScene(Device& d);
    virtual ~ShiftyEyesScene() = default;

    virtual void enter() override;
    virtual void update(uint32_t dt);
    virtual void gamepadConnected() override;
    virtual void gamepadDisconnected() override;
    virtual void receivedColor(const Color::RGB& c) override;

    static constexpr uint8_t maxEyelidPosition = 7;

    uint8_t getEyelidPosition() const {
        return eyelidPosition;
    }

    void setEyelidPosition(uint8_t pos) {
        eyelidPosition = min(pos, maxEyelidPosition);
    }

    bool hasMonsterPupils = false;

    int8_t xPupil = 9;
    int8_t yPupil = 9;

    FSM pupilsFSM;
    FSM eyelidsFSM;

private:
    void draw();
    void drawPupils();
    void drawRingRow(uint8_t index, const Color::RGB& color);
    void drawEyeOutlines();

private:
    static constexpr uint8_t pupilBrightness = 220;
    float pupilHue = 0;

    static constexpr uint8_t ringBrightness = 100;
    float ringHue = 10922;

    // 0 = fully open, maxEyelidPosition = fully closed
    uint8_t eyelidPosition = 0;
};