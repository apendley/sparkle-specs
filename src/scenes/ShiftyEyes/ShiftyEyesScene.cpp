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

    eyes.setPupilPosition(3, 3);
    eyes.setEyelidPosition(0);
    eyes.ringColor = settings.shiftyEyesGetRingColor();

    triggerGamepadConnectionEvent();
}

void ShiftyEyesScene::update(uint32_t dt) {
    pupilsFSM.update(dt);
    eyelidsFSM.update(dt);

    draw();

    pupilsFSM.transition();
    eyelidsFSM.transition();
}

void ShiftyEyesScene::draw() {
    Glasses& glasses = getDevice().glasses;
    glasses.fill(0);
    drawPupils();
    drawEyeOutlines();
    glasses.show();
}

void ShiftyEyesScene::drawPupils() {
    Glasses& glasses = getDevice().glasses;

    int8_t x = eyes.getPupilX();
    int8_t y = eyes.getPupilY();
    uint16_t c = eyes.pupilColor.gammaApplied().packed565();
    
    glasses.fillRect(x, y, 2, 2, c);
    glasses.fillRect(x + 10, y, 2, 2, c);    
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

    if (eyelidPosition == 0) {
        uint32_t c = eyes.ringColor.gammaApplied().packed();
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
                    uint16_t c = eyes.ringColor.gammaApplied().packed565();

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
                drawRingRow(i, eyes.ringColor.gammaApplied());
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
    // Convert to HSV with full saturation and our brightness, back to RGB, and set.
    Color::HSV hsv = Color::HSV::fromRGB(c);
    hsv.v = eyes.ringBrightness;
    hsv.s = 255;
    eyes.ringColor = hsv.toRGB();

    getDevice().settings.shiftyEyesSetRingColor(eyes.ringColor);
}