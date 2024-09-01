#include "GooglyRings_Connected.h"
#include "GooglyRingsScene.h"

namespace {
    // Earth's gravity in m/s²
    const float GRAVITY = 9.8;

    // Typical zero-g offset for Wii Nunchuk
    const uint16_t ZERO_G_OFFSET = 512;

    // Scale factor: LSB per g    
    const float SCALE_FACTOR = 256.0;

    float raw2accel(int rawValue) {
        float gForce = (rawValue - ZERO_G_OFFSET) / SCALE_FACTOR;
        return gForce * GRAVITY;
    }
}

void GooglyRings_Connected::enter() {
    Glasses& glasses = getDevice().glasses;
    glasses.fill(0);

    frameElapsed = frameInterval;
}

void GooglyRings_Connected::update(uint32_t dt) {
    Gamepad& gamepad = getDevice().gamepad;
    Gamepad::Report report = gamepad.getReport();    

    frameElapsed += dt;

    while (frameElapsed >= frameInterval) {
        frameElapsed -= frameInterval;

        // Downward by default.
        float ax = raw2accel(350);
        float az = raw2accel(512);

        // Press z to "grab" the pendulums and move them by rotating the nunchuck on the x axis.
        if (gamepad.isDown(gamepad.buttonZ)) {
            ax = raw2accel(1023 - report.az);
            az = raw2accel(report.ax);
        }
        // Or, you can use the analog stick to pull the pendulums.
        else if (abs(report.x) > 0 || abs(report.y) > 0) {
            // Mimic nunchuck accelerometer values.
            int32_t jx = map(report.x, -127, 127, 350, 700);
            int32_t jy = map(report.y, -127, 127, 350, 700);
            ax = raw2accel(jy);
            az = raw2accel(jx);
        }

        scene.leftPendulum.step(ax, az);
        scene.rightPendulum.step(ax, az);                    
    }

    if (gamepad.isDown(gamepad.buttonC)) {
        scene.hue += dt * 10;
        scene.saturation = 255;

        if (scene.hue >= 65535) {
            scene.hue = 0.0;
        }
    }    
    else if (gamepad.fell(gamepad.buttonC)) {
        Settings& settings = getDevice().settings;
        settings.googlyRingsSetHue(scene.hue);
        settings.googlyRingsSetSaturation(scene.saturation);
    }
}
