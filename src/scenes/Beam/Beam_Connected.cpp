#include "Beam_Connected.h"
#include "BeamScene.h"

void Beam_Connected::enter() {
    Glasses& glasses = getDevice().glasses;

    scene.instant = false;
    scene.fade = 240;
    scene.xBeam = glasses.width() / 2;
    scene.yBeam = glasses.height() / 2;
}

void Beam_Connected::update(uint32_t dt) {
    Glasses& glasses = getDevice().glasses;
    Gamepad& gamepad = getDevice().gamepad;
    SoftGamepad& softGamepad = getDevice().softGamepad;
    Settings& settings = getDevice().settings;
    const Gamepad::Report& report = gamepad.getReport();

    // Invert since "left" is opposite wearer's left if looking at glasses.
    uint16_t x = 255 - (report.x + 128);

    // Invert so that "down" matches down on the glasses.
    uint16_t y = 255 - (report.y + 128);

    scene.xBeam = map(x, 0, 255, 0, glasses.width());
    scene.yBeam = map(y, 0, 255, 0, glasses.height());

    if (gamepad.isDown(gamepad.buttonC)) {
        scene.hue += dt * 10;
        scene.saturation = 255;

        if (scene.hue >= 65535) {
            scene.hue = 0.0;
        }
    }
    else if (gamepad.wasReleased(gamepad.buttonC)) {
        settings.beamSetHue(scene.hue);
        settings.beamSetSaturation(scene.saturation);
    }

    if (gamepad.wasPressed(gamepad.buttonZ) || softGamepad.wasPressed(softGamepad.button1)) {
        scene.gotoNextBeamMode();
    }
}
