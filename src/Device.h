#pragma once

#include <Adafruit_IS31FL3741.h>
#include <Adafruit_LIS3DH.h>
#include "Gamepad.h"
#include "SoftGamepad.h"
#include "PdmRecorder.h"
#include "Settings.h"

typedef Adafruit_LIS3DH Accel;
typedef Adafruit_EyeLights_buffered Glasses;

struct Device {
    Device(Accel& _accel, 
           Glasses& _glasses, 
           PdmRecorder& _pdmRecorder, 
           Gamepad& _gamepad, 
           SoftGamepad& _softGamepad,
           Settings& _settings) : 
        accel(_accel),
        glasses(_glasses), 
        pdmRecorder(_pdmRecorder),
        gamepad(_gamepad),
        softGamepad(_softGamepad),
        settings(_settings)
    {

    }

    Accel& accel;
    Glasses& glasses;
    PdmRecorder& pdmRecorder;
    Gamepad& gamepad;
    SoftGamepad& softGamepad;
    Settings& settings;

    bool isGamepadConnected() {
        return gamepadConnected;
    }

    void setGamepadConnected(bool c) {
        gamepadConnected = c;
    }

private:
    bool gamepadConnected = false;
};