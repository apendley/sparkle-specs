#pragma once

#include <Arduino.h>

class FSMState {
public:
    FSMState() = default;
    virtual ~FSMState() = default;

    virtual void enter() {}
    virtual void update(uint32_t dt) {}
    virtual FSMState* transition() { return nullptr; }
    virtual void exit() {}
};