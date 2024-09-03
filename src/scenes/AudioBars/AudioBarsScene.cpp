#include "AudioBarsScene.h"
#include "Color.h"

AudioBarsScene::AudioBarsScene(Device& d) :
    Scene(d),
    spectrumizer(10, 70)
{
    for (int i = 0; i < columnCount; i++) {
        columnColors[i] = Color::HSV(57600UL * i / columnCount, 255, brightness)
            .toRGB()
            .packed565();
    }
}

void AudioBarsScene::enter() {
    spectrumizer.reset();
    getDevice().pdmRecorder.startRecording();

    Settings& settings = getDevice().settings;
    useCustomColor = settings.audioBarsUseCustomColor();
    snowCapped = settings.audioBarsSnowCapped();
    hue = settings.audioBarsHue();
    saturation = settings.audioBarsSaturation();
}

void AudioBarsScene::update(uint32_t dt) {
    Gamepad& gamepad = getDevice().gamepad;
    SoftGamepad& softGamepad = getDevice().softGamepad;
    PdmRecorder& pdmRecorder = getDevice().pdmRecorder;
    Settings& settings = getDevice().settings;

    if (gamepad.isDown(gamepad.buttonC)) {
        useCustomColor = true;
        hue += dt * 10;
        saturation = 255;

        if (hue >= 65535.0) {
            hue = 0.0;
        }
    }
    else if (gamepad.wasReleased(gamepad.buttonC)) {
        settings.audioBarsSetUseCustomColor(useCustomColor);
        settings.audioBarsSetHue(hue);
        settings.audioBarsSetSaturation(saturation);
    } 
    else if (gamepad.wasPressed(gamepad.buttonZ)) {
        useCustomColor = false;
        settings.audioBarsSetUseCustomColor(useCustomColor);
    }

    if (softGamepad.wasPressed(softGamepad.button1)) {
        snowCapped = !snowCapped;
        settings.audioBarsSetSnowCapped(snowCapped);
    }

    spectrumizer.update(pdmRecorder.frontBuffer(), dt);
    draw();
}

void AudioBarsScene::draw() {
    Glasses& glasses = getDevice().glasses;

    glasses.fill(0);

    uint16_t rgbOverride = Color::HSV(uint16_t(hue), saturation, brightness).toRGB().packed565();

    // Reduce brightness of pure white, because it's a lot brighter than other colors.
    uint16_t defaultDotColor = Color::RGB::gray(brightness).scaled(90).packed565();

    for (int columnIndex = 0; columnIndex < columnCount; columnIndex++) {
        int xDisplay;

        // Skip the middle two columns
        if (columnIndex <= 7) {
            xDisplay = columnIndex;
        }
        else {
            xDisplay = columnIndex + 2;
        }

        float columnTop = spectrumizer.getColumnTop(columnIndex);
        float columnDot = spectrumizer.getColumnDot(columnIndex);

        uint16_t barColor = useCustomColor ? rgbOverride : columnColors[columnIndex];
        glasses.drawLine(xDisplay, columnTop, xDisplay, columnTop + 20, barColor);

        uint16_t yDot = min(columnDot, 4);
        uint16_t dotColor = snowCapped ? defaultDotColor : barColor;
        glasses.drawPixel(xDisplay, yDot, dotColor);
    }

    glasses.show();    
}

void AudioBarsScene::exit() {
    getDevice().pdmRecorder.stopRecording();
}

void AudioBarsScene::receivedColor(const Color::RGB& c) {
    Settings& settings = getDevice().settings;

    if (c.isBlack()) {
        useCustomColor = false;
    } 
    else {
        useCustomColor = true;
        Color::HSV hsv = Color::HSV::fromRGB(c);
        hue = hsv.h;
        saturation = hsv.s;

        settings.audioBarsSetHue(hsv.h);
        settings.audioBarsSetSaturation(hsv.s);
    }

    settings.audioBarsSetUseCustomColor(useCustomColor);    
}