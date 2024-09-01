#include "GooglyRings_Disconnected.h"
#include "GooglyRingsScene.h"
#include <Adafruit_Sensor.h>

void GooglyRings_Disconnected::enter() {
    getDevice().glasses.fill(0);
    frameElapsed = frameInterval;
}

void GooglyRings_Disconnected::update(uint32_t dt) {
    frameElapsed += dt;

    while (frameElapsed >= frameInterval) {
        frameElapsed -= frameInterval;

        sensors_event_t event;
        getDevice().accel.getEvent(&event);

        scene.leftPendulum.step(event.acceleration.x, event.acceleration.z);
        scene.rightPendulum.step(event.acceleration.x,event.acceleration.z);
    }
}
