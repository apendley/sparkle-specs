#pragma once

#include <Arduino.h>

// Adapted from MIT-licensed https://learn.adafruit.com/adafruit-eyelights-led-glasses-and-driver/googly-eye-rings
class Pendulum {
public:
    Pendulum() {
        angle = random(1000);
        momentum = 0.0;

        // Randomize slightly so the rings aren't perfectly synced together.
        friction = 0.94 + random(300) * 0.0001;
    }

    void setAngle(float a) {
        angle = a;
        momentum = 0;
    }

    const float getAngle() const {
        return angle;
    }    

    void step(float xAccel, float yAccel) {
        // Minus here is because LED pixel indices run clockwise vs. trigwise.
        // 0.006 is just an empirically-derived scaling fudge factor that looks
        // good; smaller values for more sluggish rings, higher = more twitch.
        momentum =  momentum * friction - 0.006 *(cos(angle) * yAccel + sin(angle) * xAccel);
        angle += momentum;
    }
    
private:
    float    angle;
    float    momentum;
    float    friction;
};