#include "MarqueeScene.h"
#include <EyeLightsCanvasFont.h>

MarqueeScene::MarqueeScene(Device& d)
    : Scene(d) 
{

}

void MarqueeScene::enter() {
    Glasses& glasses = getDevice().glasses;
    glasses.left_ring.fill(0);
    glasses.right_ring.fill(0);
    glasses.fill(0);

    Settings& settings = getDevice().settings;
    useCustomColor = settings.marqueeUseCustomColor();
    hue = settings.marqueeHue();
    saturation = settings.marqueeSaturation();
    scrollDelay = settings.marqueeScrollDelay();
    settings.marqueeGetMessage(messageBuffer, messageBufferSize);

    GFXcanvas16* canvas = glasses.getCanvas();
    canvas->setFont(&EyeLightsCanvasFont);
    canvas->setTextWrap(false);

    resetScroll();
}

void MarqueeScene::update(uint32_t dt) {
    Settings& settings = getDevice().settings;    
    Gamepad& gamepad = getDevice().gamepad;
    SoftGamepad& softGamepad = getDevice().softGamepad;
    Glasses& glasses = getDevice().glasses;
    GFXcanvas16* canvas = glasses.getCanvas();

    // Update color
    if (gamepad.isDown(gamepad.buttonC)) {
        useCustomColor = true;
        hue += dt * 10;
        saturation = 255;

        if (hue >= 65535.0) {
            hue = 0.0;
        }
    }
    else if (gamepad.fell(gamepad.buttonC)) {
        settings.marqueeSetUseCustomColor(useCustomColor);
        settings.marqueeSetHue(hue);
        settings.marqueeSetSaturation(saturation);
    }
    else if (gamepad.rose(gamepad.buttonZ)) {
        useCustomColor = false;        
        settings.marqueeSetUseCustomColor(useCustomColor);
    }

    // Update scroll speed
    const Gamepad::Report& report = gamepad.getReport();

    if (abs(report.x) > 0) {
        scrollDelay -= report.x * (dt * .0001);
        scrollDelay = min(max(scrollDelay, minScrollDelay), maxScrollDelay);
        isAdjustingScrollDelay = true;
    } else if (isAdjustingScrollDelay && abs(report.x) < 0.0001) {
        isAdjustingScrollDelay = false;
        settings.marqueeSetScrollDelay(scrollDelay);
    }

    if (softGamepad.rose(softGamepad.button1)) {
        scrollDelay = maxScrollDelay;
        settings.marqueeSetScrollDelay(scrollDelay);
    }
    else if (softGamepad.rose(softGamepad.button2)) {
        scrollDelay = defaultScrollDelay;
        settings.marqueeSetScrollDelay(scrollDelay);
    }
    else if (softGamepad.rose(softGamepad.button3)) {
        scrollDelay = minScrollDelay;
        settings.marqueeSetScrollDelay(scrollDelay);
    }

    // Update scroll
    scrollElapsed += dt;

    if (scrollElapsed >= scrollDelay) {
        scrollElapsed = fmod(scrollElapsed, scrollDelay);

        canvas->fillScreen(0);
        canvas->setCursor(scrollPosition, canvas->height());

        uint32_t len = strlen(messageBuffer);

        if (useCustomColor) {
            canvas->setTextColor(Color::HSV(hue, saturation, brightness).toRGB().packed565());
            canvas->print(messageBuffer);
        } 
        else {
            for (unsigned int i = 0; i < len; i++) {
                uint16_t h = (startHue + (i * 4096)) & 0xFFFF;
                uint16_t c = Color::HSV(h, 255, brightness).toRGB().packed565();
                canvas->setTextColor(c);
                canvas->write(messageBuffer[i]);
            }            
        }

        glasses.scale();
        glasses.show();

        scrollPosition--;
        if( scrollPosition < -messageWidth) {
            scrollPosition = canvas->width();
            startHue = (startHue + (len * 4096)) & 0xFFFF;
        }
    }
}

void MarqueeScene::receivedColor(const Color::RGB& c) {
    Settings& settings = getDevice().settings;        

    if (c.isBlack()) {
        useCustomColor = false;
    }
    else {
        useCustomColor = true;
        Color::HSV hsv = Color::HSV::fromRGB(c);
        hue = hsv.h;
        saturation = hsv.s;
        settings.marqueeSetHue(hsv.h);
        settings.marqueeSetSaturation(hsv.s);    
    }

    settings.marqueeSetUseCustomColor(useCustomColor);
}

void MarqueeScene::receivedText(const char* text) {
    strcpy(messageBuffer, text);
    getDevice().settings.marqueeSetMessage(text);
    resetScroll();
}

void MarqueeScene::resetScroll() {
    Glasses& glasses = getDevice().glasses;
    messageWidth = textWidth(messageBuffer);
    scrollPosition = glasses.getCanvas()->width();
    scrollElapsed = 0;
    startHue = 0;    
}

uint8_t MarqueeScene::charWidth(uint8_t c) const {
    return EyeLightsCanvasFont.glyph[c - EyeLightsCanvasFont.first].xAdvance;
}

uint16_t MarqueeScene::textWidth(const char* str) {
    const uint32_t len = strlen(str);
    uint32_t w = 0;

    for (uint32_t i = 0; i < len; i++) {
        w += charWidth(str[i]);
    }
    
    return w;
}