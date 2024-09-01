#pragma once

#include <Arduino.h>

struct ButtonEvent {
    static constexpr uint8_t invalidIndex = 0xFF;

    ButtonEvent() { }
    ButtonEvent(uint8_t i, bool s) : index(i), state(s) { }

    const uint8_t index = invalidIndex;
    const bool state = false;
};