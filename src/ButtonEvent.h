#pragma once

#include <Arduino.h>

struct ButtonEvent {
    ButtonEvent(uint8_t i, bool s) : index(i), state(s) { }

    const uint8_t index;
    const bool state;
};