#pragma once

#include <Arduino.h>
#include "FSMState.h"

class FSM {
public:

    ~FSM();
    
    void gotoState(FSMState* s);
    void update(uint32_t dt);
    void transition();

private:
    FSMState* currentState = nullptr;
};