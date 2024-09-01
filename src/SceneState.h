#pragma once

#include "FSMState.h"
#include "Device.h"

template <typename SceneType> 
class SceneState : public FSMState {
public:
    SceneState(SceneType& s) : 
        scene(s)
    { }
    virtual ~SceneState() = default;

protected:
    // Easy access to the device, frequently needed by each state.
    inline Device& getDevice() {
        return scene.getDevice();
    }

    SceneType& scene;
};