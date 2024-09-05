#pragma once

#include <Arduino.h>
#include "ButtonEvent.h"

struct SoftGamepad {
public:
    enum {
        button1     = (1 << 0),
        button2     = (1 << 1),
        button3     = (1 << 2),
        button4     = (1 << 3),
        buttonUp    = (1 << 4),
        buttonDown  = (1 << 5),
        buttonLeft  = (1 << 6),
        buttonRight = (1 << 7),
    };

public:
    SoftGamepad() { }

    void reset() {
        pressed = buttons = previousButtons = 0;
        released = 0;
    }

    void event(const ButtonEvent& event) {
        if (event.state) {
            pressed |= (1 << event.index);
        } else {
            released |= (1 << event.index);
        }
    }

    void update() {
        previousButtons = buttons;

        if (released) {
            buttons &= ~released;
            released = 0;
        }

        if (pressed) {
            buttons |= pressed;
            pressed = 0;
        }

        // if (buttons != previousButtons) {
        //     Serial.printf("buttons: 0x%02x, previousButtons: 0x%02x\n", softGamepad.buttons, softGamepad.previousButtons);
        // }        
    }

    inline bool isDown(uint8_t b) {
        return buttons & b;
    }
    
    inline bool isUp(uint8_t b) {
        return ~buttons & b;
    }
    
    inline bool buttonHeld(uint8_t b) {
        return (buttons & previousButtons) & b;
    }
    
    inline bool wasReleased(uint8_t b) {
        return (~buttons & previousButtons) & b;
    }
    
    inline bool wasPressed(uint8_t b) {
        return (buttons & ~previousButtons) & b;
    }
    
    inline bool changed(uint8_t b) {
        return (buttons ^ previousButtons) & b; 
    }

private:
    uint8_t pressed = 0;
    uint8_t released = 0;
    uint8_t buttons = 0;
    uint8_t previousButtons = 0;    
};