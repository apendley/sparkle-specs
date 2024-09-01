#pragma once

#include <Arduino.h>

namespace Rings {
    struct Row {
        const uint8_t* indices;
        uint8_t numIndices;
    };

    extern const int numRows;
    extern const Row* getRow(uint8_t index);
}
