#include "FSM.h"

FSM::~FSM() {
    if (currentState != nullptr) {
        delete currentState;
        currentState = nullptr;
    }
}

void FSM::gotoState(FSMState* s) {
    if (currentState != nullptr) {
        currentState->exit();
        delete currentState;
    }

    currentState = s;

    if (currentState != nullptr) {
        currentState->enter();
    }    
}

void FSM::update(uint32_t dt) {
    if (currentState != nullptr) {
        currentState->update(dt);
    }
}

void FSM::transition() {
    if (currentState != nullptr) {
        FSMState* nextState = currentState->transition();

        if (nextState != nullptr) {
            gotoState(nextState);            
        }
    }    
}
