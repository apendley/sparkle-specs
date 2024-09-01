#include "Beam_Disconnected.h"
#include "BeamScene.h"

namespace {
    struct SpeedSetting {
        float speed;
        uint8_t fadeRate;
        float waveLength;
    };
    
    SpeedSetting speedSettings[] = {
        {0.01, 254, 0.25},
        {0.02, 251, 0.50},
        {0.03, 248, 0.75},
    };

    const uint8_t speedSettingsCount = sizeof(speedSettings) / sizeof(speedSettings[0]);
}

void Beam_Disconnected::enter() {
    direction = 1;
    scene.instant = true;
    speedIndex = getDevice().settings.beamDisconnectedSpeed();
    scene.fade = speedSettings[speedIndex].fadeRate;
}

void Beam_Disconnected::update(uint32_t dt) {
    Glasses& glasses = getDevice().glasses;
    SoftGamepad& softGamepad = getDevice().softGamepad;

    switch (scene.beamMode) {
        case BeamScene::BeamMode::vertical: {
            float x = scene.xBeam + direction * dt * speedSettings[speedIndex].speed;

            if (x >= glasses.width()) {
                direction = -1;
                x = glasses.width() - 0.001;
            } else if (x <= 0) {
                direction = 1;
                x = 0.001;
            }

            scene.xBeam = x;
        }
        break;

        case BeamScene::BeamMode::horizontal: {
            float y = scene.yBeam + direction * dt * (speedSettings[speedIndex].speed * 0.5);

            if (y >= glasses.height()) {
                direction = -1;
                y = glasses.height() - 0.001;
            } else if (y <= 0) {
                direction = 1;
                y = 0.001;
            }

            scene.yBeam = y;
        }
        break;

        case BeamScene::BeamMode::point: {
            float x = scene.xBeam + direction * dt * speedSettings[speedIndex].speed;

            if (x >= glasses.width()) {
                direction = -1;
                x = glasses.width() - 0.001;
            } else if (x <= 0) {
                direction = 1;
                x = 0.001;
            }

            scene.xBeam = x;
            uint8_t theta = millis() * speedSettings[speedIndex].waveLength;
            uint32_t s = Color::sine8(theta);
            scene.yBeam = (s * glasses.height()) >> 8;
        }
        break;                
    }

    // Update input
    bool speedChanged = false;

    if (softGamepad.rose(softGamepad.button1)) {
        speedChanged = speedIndex != 0;        
        speedIndex = 0;
    }
    else if (softGamepad.rose(softGamepad.button2)) {
        speedChanged = speedIndex != 1;
        speedIndex = 1;
    }
    else if (softGamepad.rose(softGamepad.button3)) {
        speedChanged = speedIndex != 2;
        speedIndex = 2;
    }    

    if (speedChanged) {
        scene.fade = speedSettings[speedIndex].fadeRate;
        getDevice().settings.beamDisconnectedSetSpeed(speedIndex);        
    }

    if (softGamepad.rose(softGamepad.button4)) {
        scene.gotoNextBeamMode();
    }
}