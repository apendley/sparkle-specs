#pragma once

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <CircularBuffer.hpp>

class Gamepad {
public:
    enum {
        buttonC = GAMEPAD_BUTTON_C,
        buttonZ = GAMEPAD_BUTTON_Z
    };

    struct Report {
        int8_t x = 0;
        int8_t y = 0;
        uint16_t ax = 0;
        uint16_t ay = 0;
        uint16_t az = 0;
        uint32_t buttons = 0;
    };

    Gamepad() { }

    void reset() { 
        report = previousReport = Report();
    }
    
    void update(const hid_gamepad_report_t& r) {
        previousReport = report;

        report.x = r.x;
        report.y = r.y;
        // Serial.printf("report x: %d, y: %d\n", report.x, report.y);

        report.ax = map(r.rx, -127, 127, 0, 1024);
        report.ay = map(r.ry, -127, 127, 0, 1024);
        report.az = map(r.rz, -127, 127, 0, 1024);

        report.buttons = r.buttons;
    }

    inline bool isDown(uint32_t b) {
        return report.buttons & b;
    }
    
    inline bool isUp(uint32_t b) {
        return ~report.buttons & b;
    }
    
    inline bool buttonHeld(uint32_t b) {
        return (report.buttons & previousReport.buttons) & b;
    }
    
    inline bool fell(uint32_t b) {
        return (~report.buttons & previousReport.buttons) & b;
    }
    
    inline bool rose(uint32_t b) {
        return (report.buttons& ~previousReport.buttons) & b;
    }
    
    inline bool changed(uint32_t b) {
        return (report.buttons ^ previousReport.buttons) & b; 
    }

    inline const Report& getReport() const {
        return report;
    }

    inline const Report& getPreviousReport() const {
        return previousReport;
    }    

    inline bool stickChanged() const {
        return (report.x != previousReport.x) || (report.y != previousReport.y);
    }

private:
    Report report;
    Report previousReport;
};