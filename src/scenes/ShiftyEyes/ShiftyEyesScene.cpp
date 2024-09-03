#include "ShiftyEyesScene.h"
#include "Pupils_Connected.h"
#include "Pupils_Disconnected.h"
#include "EyeLids_Disconnected.h"
#include "EyeLids_Idle.h"
#include "RingRows.h"

ShiftyEyesScene::ShiftyEyesScene(Device& d)
    : Scene(d) 
{
}

void ShiftyEyesScene::enter() {
    Settings& settings = getDevice().settings;

    eyes.setPupilPosition(9, 9);
    eyes.setEyelidPosition(0);

    pupilHue = settings.shiftyEyesGetPupilHue();
    ringHue = settings.shiftyEyesGetRingHue();

    hasMonsterPupils = settings.shiftyEyesHasMonsterPupils();

    triggerGamepadConnectionEvent();
}

void ShiftyEyesScene::update(uint32_t dt) {
    SoftGamepad& softGamepad = getDevice().softGamepad;
    Settings& settings = getDevice().settings;

    if (softGamepad.wasPressed(softGamepad.button1)) {
        hasMonsterPupils = !hasMonsterPupils;
        getDevice().settings.shiftyEyesSetHasMonsterPupils(hasMonsterPupils);
    }

    if (softGamepad.isDown(softGamepad.button2)) {
        pupilHue += dt * 6;

        if (pupilHue >= 65535.0) {
            pupilHue = 0.0;
        }        
    }
    else if (softGamepad.wasReleased(softGamepad.button2)) {
        settings.shiftyEyesSetPupilHue(pupilHue);
    }

    if (softGamepad.isDown(softGamepad.button3)) {
        ringHue += dt * 6;

        if (ringHue >= 65535.0) {
            ringHue = 0.0;
        }        
    }
    else if (softGamepad.wasReleased(softGamepad.button3)) {
        settings.shiftyEyesSetRingHue(ringHue);
    }


    pupilsFSM.update(dt);
    eyelidsFSM.update(dt);

    draw();

    pupilsFSM.transition();
    eyelidsFSM.transition();
}

void ShiftyEyesScene::draw() {
    Glasses& glasses = getDevice().glasses;
    auto canvas = glasses.getCanvas();

    canvas->fillScreen(0);

    drawPupils();
    glasses.scale();

    drawEyeOutlines();
    glasses.show();
}

void ShiftyEyesScene::drawPupils() {
    Glasses& glasses = getDevice().glasses;
    auto canvas = glasses.getCanvas();

    int8_t x = eyes.getPupilX();
    int8_t y = eyes.getPupilY();
    uint16_t c = Color::HSV(pupilHue, 255, pupilBrightness).toRGB().gammaApplied().packed565();
    
    if (hasMonsterPupils) {
        canvas->fillRect(x, y + 1, 3, 9, c);
        canvas->fillRect(x + 30, y + 1, 3, 9, c);
    }
    else {
        canvas->fillRect(x, y, 6, 6, c);
        canvas->fillRect(x + 30, y, 6, 6, c);
    }
}

void ShiftyEyesScene::drawRingRow(uint8_t index, const Color::RGB& color) {
    Glasses& glasses = getDevice().glasses;
    const Rings::Row* row = Rings::getRow(index);

    uint32_t c = color.packed();
    
    for (uint8_t i = 0; i < row->numIndices; i++) {
        glasses.left_ring.setPixelColor(row->indices[i], c);
        glasses.right_ring.setPixelColor(row->indices[i], c);
    }
}

void ShiftyEyesScene::drawEyeOutlines() {
    Glasses& glasses = getDevice().glasses;
    uint8_t eyelidPosition = eyes.getEyelidPosition();

    Color::RGB ringColor = Color::HSV(ringHue, 255, ringBrightness).toRGB().gammaApplied();

    // Fill in the rect between the rings, in case the pupils bleed over a little.
    glasses.fillRect(7, 0, 4, 4, 0); 

    if (eyelidPosition == 0) {
        uint32_t c = ringColor.packed();
        glasses.left_ring.fill(c);
        glasses.right_ring.fill(c);
    } else {
        // Draw the matrix pixels. We're drawing first so the ring drawing
        // can have priority over the overlapping matrix pixels.
        // The matrix's first row is actually the ring's second "row",
        // so we can just draw zeros over the "eye" pixels,
        // except on the line where we want to draw the eyelid
        for (int i = 0; i < glasses.height(); i++) {
            int matrixRowIndex = eyelidPosition - 1;
            
            if (matrixRowIndex >= 0) {
                if (i == matrixRowIndex) {
                    uint16_t c = ringColor.packed565();

                    // Left eye
                    glasses.fillRect(1, matrixRowIndex, 6, 1, c);
                    // Right eye
                    glasses.fillRect(11, matrixRowIndex, 6, 1, c);
                }
                else if (i < matrixRowIndex) {
                    // Left eye
                    glasses.fillRect(1, i, 6, 1, 0);
                    // Right eye
                    glasses.fillRect(11, i, 6, 1, 0);
                }
            }
        }

        // Draw the rings now, clearing the rows above the "blink row".
        for (int i = 0; i < Rings::numRows; i++) {
            if (i < eyelidPosition) {
                drawRingRow(i, Color::RGB());
            } else {
                drawRingRow(i, ringColor);
            }
        }        
    }
}

void ShiftyEyesScene::gamepadConnected() {
    pupilsFSM.gotoState(new Pupils_Connected(*this));
    eyelidsFSM.gotoState(new EyeLids_Idle(*this));
}

void ShiftyEyesScene::gamepadDisconnected() {
    pupilsFSM.gotoState(new Pupils_Disconnected(*this));
    eyelidsFSM.gotoState(new EyeLids_Disconnected(*this));
}

void ShiftyEyesScene::receivedColor(const Color::RGB& c) {
    if (c.isBlack()) {
        return;
    }

    Settings& settings = getDevice().settings;

    Color::HSV hsv = Color::HSV::fromRGB(c);
    pupilHue = hsv.h;
    settings.shiftyEyesSetPupilHue(pupilHue);

    // Set ring color to major color in the color wheel.
    hsv.h = hsv.h + 65536 / 6;
    ringHue = hsv.h;
    settings.shiftyEyesSetRingHue(ringHue);    
    

}