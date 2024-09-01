#pragma once

#include <Arduino.h>
#include "Device.h"
#include "Color.h"

class Scene {
public:
    Scene(Device& d) : device(d) {}
    virtual ~Scene() = default;

    // Really just for consistency with SceneState.getDevice(), so I don't
    // have to remember which one to use depending on which class I'm in.
    inline Device& getDevice() { return device; }

    // Lifetime events
    virtual void enter() {}
    virtual void exit() {}
    virtual void update(uint32_t dt) {}

    // External events
    virtual void gamepadConnected() {}
    virtual void gamepadDisconnected() {}
    virtual void receivedColor(const Color::RGB& c) {}
    virtual void receivedText(const char* text) {}

protected:
    // Useful for subclasses that have disconnected and connected states.
    // Call in enter() to automatically call the correct event based
    // on the current connectivity of the gamepad.
    void triggerGamepadConnectionEvent() {
        if (getDevice().isGamepadConnected()) {
            gamepadConnected();
        } else {
            gamepadDisconnected();
        }        
    }

private:
    Device& device;
};