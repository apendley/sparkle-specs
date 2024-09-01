#pragma once

#include <Arduino.h>
#include <bluefruit.h>

// Adapted from BLEClientHidAdafruit
class BLEClientHidGamepad : public BLEClientService {
public:
    typedef void (*gamepad_callback_t) (hid_gamepad_report_t* report);    

public:
    BLEClientHidGamepad(void);

    virtual bool begin(void) override;
    virtual bool discover(uint16_t connHandle) override;

    bool getHidInfo(uint8_t info[4]);    

    bool gamepadPresent(void);
    bool enableGamepad();
    bool disableGamepad();
    void getGamepadReport(hid_gamepad_report_t* report);

    void setGamepadReportCallback(gamepad_callback_t fp);

private:
    static void gamepadClientNotifyCallback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len);
    void handleGamepadInput(uint8_t* data, uint16_t len);    

private:
    gamepad_callback_t gamepadCallback;
    hid_gamepad_report_t lastGamepadReport;
    BLEClientCharacteristic hidInfo;
    BLEClientCharacteristic hidReport;
};
