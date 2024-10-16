#include "BeamScene.h"
// #include <Adafruit_TinyUSB.h>
#include "Color.h"
#include "Beam_Connected.h"
#include "Beam_Disconnected.h"

BeamScene::BeamScene(Device& d)
    : Scene(d)
{
    
}

void BeamScene::gotoNextBeamMode() {
    uint8_t n = uint8_t(beamMode) + 1;

    if (n > beamModeCount) {
        n = 0;
    } 

    beamMode = BeamMode(n);

    getDevice().settings.beamSetMode(n);
}

void BeamScene::enter() {
    Glasses& glasses = getDevice().glasses;
    Settings& settings = getDevice().settings;

    glasses.left_ring.fill(0);
    glasses.right_ring.fill(0);
    glasses.fill(0);

    xCurrent = xBeam = glasses.width() / 2;
    yCurrent = yBeam = glasses.height() / 2;

    beamMode = BeamMode(settings.beamMode());
    hue = settings.beamHue();
    saturation = settings.beamSaturation();

    triggerGamepadConnectionEvent();
}

void BeamScene::update(uint32_t dt) {
    beamFSM.update(dt);

    if (instant) {
        xCurrent = xBeam;
        yCurrent = yBeam;
    } else {
        float xSign1 = (xBeam - xCurrent) < 0 ? -1 : 1;
        float ySign1 = (yBeam - yCurrent) < 0 ? -1 : 1;

        float x = xCurrent + xSign1 * (dt * xSpeed / 1000);
        float y = yCurrent + ySign1 * (dt * ySpeed / 1000);

        float xSign2 = (xBeam - x) < 0 ? -1 : 1;
        float ySign2 = (yBeam - y) < 0 ? -1 : 1;

        if (xSign1 == xSign2) {
            xCurrent = x;
        } else {
            xCurrent = xBeam;
        }

        if (ySign1 == ySign2) {
            yCurrent = y;
        } else {
            yCurrent = yBeam;
        }
    }

    draw();
    beamFSM.transition();
}

void BeamScene::draw() {
    Glasses& glasses = getDevice().glasses;
    Settings& settings = getDevice().settings;

    int32_t x = int32_t(xCurrent);
    int32_t y = int32_t(yCurrent);

    uint8_t brightness = map(settings.sceneBrightness(), 0, 255, 100, 200);
    // Serial.printf("%d, %d\n", settings.sceneBrightness(), brightness);
    Color::RGB beamColor = Color::HSV(int(hue), saturation, brightness).toRGB();

    switch (beamMode) {
        case BeamMode::vertical: {
            // Subtract one from height to exclude the row that's missing 2 pixels in the middle.
            for (int i = 0; i < glasses.height(); i++) {
                glassesBuffer.setMatrixColor(x, i, beamColor);
            }
        }
        break;

        case BeamMode::horizontal: {
            for (int i = 0; i < glasses.width(); i++) {
                glassesBuffer.setMatrixColor(i, y, beamColor);
            }
        }
        break;

        case BeamMode::point: {
            // the top left and top corner of the matrix are actually "phantom" LEDs,
            // so let's reposition slightly if the position matches those.            
            if (y <= 0) {
                if (x <= 0) {
                    x = 1;
                } 
                else if (x >= glasses.width() - 1) {
                    x = glasses.width() - 2;
                }                            
            }

            // The two middle pixels of the bottom row are missing.
            // Move up if our point is on one of those two pixels.
            if (y == 4 && (x == 8 || x == 9)) {
                y = 3;
            }

            glassesBuffer.setMatrixColor(x, y, beamColor);
        }
        break;        
    };

    glassesBuffer.fadeMatrix(fade);

    for (int px = 0; px < GlassesBuffer::matrixWidth; px++) {
        for (int py = 0; py < GlassesBuffer::matrixHeight; py++) {
            uint16_t c = glassesBuffer.getMatrixColor(px, py).gammaApplied().packed565();
            glasses.drawPixel(px, py, c);
        }
    }

    glasses.show();
}

void BeamScene::gamepadConnected() {
    beamFSM.gotoState(new Beam_Connected(*this));
}

void BeamScene::gamepadDisconnected() {
    beamFSM.gotoState(new Beam_Disconnected(*this));
}

void BeamScene::receivedColor(const Color::RGB& c) {
    if (c.isBlack()) {
        return;
    }
    
    Color::HSV hsv = Color::HSV::fromRGB(c);
    hue = hsv.h;
    saturation = hsv.s;
    
    Settings& settings = getDevice().settings;
    settings.beamSetHue(hsv.h);
    settings.beamSetSaturation(hsv.v);
}