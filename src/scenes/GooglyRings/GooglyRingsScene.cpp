#include "GooglyRingsScene.h"
#include "GooglyRings_Connected.h"
#include "GooglyRings_Disconnected.h"
#include "Color.h"

namespace {
    void drawRingPixel(Adafruit_EyeLights_Ring_buffered& ring, uint8_t index, const Color::RGB& color, float scale) {
        uint8_t s = scale * 255;
        ring.setPixelColor(index, color.scaled(s).gammaApplied().packed());
    }    

    void drawRing(Pendulum& pendlum, Adafruit_EyeLights_Ring_buffered& ring, const Color::RGB& color) {
        // Scale pendulum angle into pixel space
        float midpoint = fmodf(pendlum.getAngle() * 12.0 / M_PI, 24.0);

        // Go around the whole ring, setting each pixel based on proximity
        // (this is also to erase the prior position)...
        for (uint8_t i=0; i<24; i++) {
            // Pixel to pendulum distance...
            float dist = fabs(midpoint - (float)i);

            // If it crosses the "seam" at top,
            if (dist > 12.0) {
                // take the shorter path.
                dist = 24.0 - dist;
            }

            // Not close to pendulum,
            if (dist > 5.0) {
                // erase pixel.
                ring.setPixelColor(i, 0);
            }
            // Close to pendulum,
            else if (dist < 1.0) {
                // solid color
                drawRingPixel(ring, i, color, 1.0);
            }
            // Anything in-between, 
            else {
                // interpolate
                drawRingPixel(ring, i, color, (5.0 - dist) / 4.0);
            }
        }        
    }
}

GooglyRingsScene::GooglyRingsScene(Device& device) :
    Scene(device) {
}

void GooglyRingsScene::enter() {
    Settings& settings = getDevice().settings;
    hue = settings.googlyRingsHue();
    saturation = settings.googlyRingsSaturation();

    triggerGamepadConnectionEvent();
}

void GooglyRingsScene::update(uint32_t dt) {
    fsm.update(dt);
    draw();
}

void GooglyRingsScene::draw() {
    Glasses& glasses = getDevice().glasses;
    Color::RGB color = Color::HSV(hue, saturation, brightness).toRGB();
    drawRing(leftPendulum, glasses.left_ring, color);
    drawRing(rightPendulum, glasses.right_ring, color);
    glasses.show();
}

void GooglyRingsScene::gamepadConnected() {
    fsm.gotoState(new GooglyRings_Connected(*this));
}

void GooglyRingsScene::gamepadDisconnected() {
    fsm.gotoState(new GooglyRings_Disconnected(*this));
}

void GooglyRingsScene::receivedColor(const Color::RGB& c) {
    if (c.isBlack()) {
        return;
    }

    Color::HSV hsv = Color::HSV::fromRGB(c);
    hue = hsv.h;
    saturation = hsv.s;

    Settings& settings = getDevice().settings;    
    settings.googlyRingsSetHue(hsv.h);
    settings.googlyRingsSetSaturation(hsv.s);
}