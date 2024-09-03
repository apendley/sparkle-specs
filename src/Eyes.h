#pragma once

#include <Arduino.h>
#include "Color.h"

class Eyes {
public:

    static constexpr uint8_t numEyelidPositions = 8;
    static constexpr uint8_t maxEyelidPosition = numEyelidPositions - 1;

    Eyes() {  }

    int8_t getPupilX() const {
        return pupilX;
    }

    void setPupilX(int8_t x) {
        pupilX = x;
    }

    int8_t getPupilY() {
        return pupilY;
    }    

    void setPupilY(int8_t y) {
        pupilY = y;
    }

    void setPupilPosition(int8_t x, int8_t y) {
        setPupilX(x);
        setPupilY(y);
    }

    uint8_t getEyelidPosition() const {
        return eyelidPosition;
    }

    void setEyelidPosition(uint8_t pos) {
        eyelidPosition = min(pos, maxEyelidPosition);
    }

private:
    int8_t pupilX = 3;
    int8_t pupilY = 3;

    // 0 = fully open, maxEyelidPosition = fully closed
    uint8_t eyelidPosition = 0;
};