#include "VolumeMeterScene.h"
#include "Color.h"

VolumeMeterScene::VolumeMeterScene(Device& d)
    : Scene(d) 
{

}

void VolumeMeterScene::enter() {
    getDevice().glasses.fill(0);
    getDevice().pdmRecorder.startRecording();

    Settings& settings = getDevice().settings;
    useCustomColor = settings.volumeMeterUseCustomColor();
    hue = settings.volumeMeterHue();
    saturation = settings.volumeMeterSaturation();    
}

void VolumeMeterScene::update(uint32_t dt) {
    Gamepad& gamepad = getDevice().gamepad;
    PdmRecorder& pdmRecorder = getDevice().pdmRecorder;
    Settings& settings = getDevice().settings;

    // Use the same filtering trick as the bars scene, and apply 30% 
    // of the last value to the current one to help prevent twitchiness.
    float reading = max(pdmRecorder.magnitude() - deadZone, 0);
    magnitude = (reading * 0.7) + (lastMagnitude * 0.3);
    lastMagnitude = magnitude;

    if (gamepad.isDown(gamepad.buttonC)) {
        useCustomColor = true;
        hue += dt * 10;
        saturation = 255;

        if (hue >= 65535.0) {
            hue = 0.0;
        }
    }
    else if (gamepad.fell(gamepad.buttonC)) {
        settings.volumeMeterSetUseCustomColor(useCustomColor);
        settings.volumeMeterSetHue(hue);
        settings.volumeMeterSetSaturation(saturation);
        
    }
    else if (gamepad.rose(gamepad.buttonZ)) {
        useCustomColor = false;
        settings.volumeMeterSetUseCustomColor(useCustomColor);
    }    

    draw();
}

void VolumeMeterScene::draw() {
    Glasses& glasses = getDevice().glasses;

    float value = minValue + (magnitude / invScale) * (maxValue - minValue);
    value = min(value, maxValue);
    uint8_t numLights = (uint8_t)value;
    // Serial.printf("mag: %f, value: %f, numLights: %d\n", magnitude, value, numLights);

    uint32_t rgbOverride = Color::HSV(hue, saturation, brightness).toRGB().gammaApplied().packed();

    glasses.left_ring.fill(0);
    glasses.right_ring.fill(0);

    for (int i = 0; i < numLights; i++) {
        uint32_t c = 0;

        if (useCustomColor) {
            c = rgbOverride;
        } else {
            uint8_t r = 0;
            uint8_t g = 0;

            if (i < 12) {
                g = 255;
                r = map(i, 0, 12, 0, 255);
            } else {
                g = map(i, 12, 24, 255, 0);
                r = 255;
            }

            c = Color::RGB(r, g, 0).scaled(brightness).gammaApplied().packed();
        }

        uint8_t leftIndex = ((23 - i) + 6) % 24;
        glasses.left_ring.setPixelColor(leftIndex, c);

        uint8_t rightIndex = (24 - leftIndex) % 24;
        glasses.right_ring.setPixelColor(rightIndex, c);
    }

    glasses.show();    
}

void VolumeMeterScene::exit() {
    getDevice().pdmRecorder.stopRecording();
}

void VolumeMeterScene::receivedColor(const Color::RGB& c) {
    Settings& settings = getDevice().settings;

    if (c.isBlack()) {
        useCustomColor = false;
    } 
    else {
        useCustomColor = true;
        Color::HSV hsv = Color::HSV::fromRGB(c);
        hue = hsv.h;
        saturation = hsv.s;
        settings.volumeMeterSetHue(hsv.h);
        settings.volumeMeterSetSaturation(hsv.s);
    }

    settings.volumeMeterSetUseCustomColor(useCustomColor);
}