#include "EyeLids_Disconnected.h"
#include "ShiftyEyesScene.h"

namespace {
    const uint8_t blinkRowIndices[] = {1, 2, 3, 4, 5, 6, 7, 7, 7, 6, 5, 4, 3, 2, 1};    
    const uint8_t blinkFrameCount = sizeof(blinkRowIndices) / sizeof(blinkRowIndices[0]);
    const int32_t blinkAnimationRate = 15;
    const int32_t countdownMin = blinkFrameCount * blinkAnimationRate;
    const int32_t countdownMax = 5000;
}

void EyeLids_Disconnected::enter() {
    scene.setEyelidPosition(0);
    blinkCountdown = random(countdownMin, countdownMax);
}

void EyeLids_Disconnected::update(uint32_t dt) {
    blinkCountdown -= dt;

    if (blinkCountdown < countdownMin) {
        uint8_t frameIndex = blinkFrameCount - 1 - (blinkCountdown / blinkAnimationRate);
        uint8_t blinkRowIndex = blinkRowIndices[frameIndex];
        scene.setEyelidPosition(blinkRowIndex);
    } else {
        scene.setEyelidPosition(0);
    }

    // Done blinking, start a new countdown!
    if (blinkCountdown <= 0) {
        blinkCountdown = random(countdownMin, countdownMax);
    }            
}