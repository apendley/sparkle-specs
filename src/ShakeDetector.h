#pragma once

#include <Arduino.h>
#include <CircularBuffer.hpp>

class ShakeDetector {
public:
    struct Reading {

        Reading() = default;

        Reading(uint16_t _x, uint16_t _y, uint16_t _z) :
            x(_x),
            y(_y),
            z(_z) 
        {
        }

        uint16_t x;
        uint16_t y;
        uint16_t z;
    };

    ShakeDetector(uint8_t _shakeCount) : shakeCount(_shakeCount) {}

    void reset(bool resetDebounce = true) {
        shakeEvents.clear();

        if (resetDebounce) {
            debounceTimer = 0;
        }
    }
    void update(const Reading& r1, const Reading& r2){
        uint32_t now = millis();

        if (now < debounceTimer) {
            return;
        }
        
        // We're only using the x axis for this for now.
        int32_t reading1 = r1.x;
        int32_t reading2 = r2.x;
        uint32_t diff = abs(reading2 - reading1);

        if (diff >= eventThreshold) {
            shakeEvents.push(now + eventTimeout);
            debounceTimer = now + debounceDuration;
            // Serial.printf("Shake event detected at %d, force: %d. Current event total: %d\n", now, diff, shakeEvents.size());        
        }

        if (!shakeEvents.isEmpty()) {
            uint32_t shiftCount = 0;

            for (uint32_t i = 0; i < shakeEvents.size(); i++) {
                if (shakeEvents[i] < now) {
                    shiftCount++;
                }
            }

            for (uint32_t i = 0; i < shiftCount; i++) {
                // Serial.println("Removing stale shake event");
                shakeEvents.shift();
            }
        }
    }

    bool shakeDetected() const {
        return shakeEvents.size() >= shakeCount;
    }
    
private:
    // This needs to be at least as big as the provided shakeCount
    // or else you won't be able to detect a shake event.
    static constexpr int maxEventCount = 8;

    static constexpr int16_t eventThreshold = 300;
    static constexpr uint32_t debounceDuration = 200;
    static constexpr uint32_t eventTimeout = 1200;    

    CircularBuffer<uint32_t, maxEventCount> shakeEvents;
    uint32_t debounceTimer = 0;
    const uint8_t shakeCount;
};