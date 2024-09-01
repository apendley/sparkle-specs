#include "BLEClientHidGamepad.h"

BLEClientHidGamepad::BLEClientHidGamepad(void) : 
    BLEClientService(UUID16_SVC_HUMAN_INTERFACE_DEVICE),
    hidInfo(UUID16_CHR_HID_INFORMATION),
    hidReport(UUID16_CHR_REPORT)
{
    gamepadCallback = NULL;
    varclr(&lastGamepadReport);
}

bool BLEClientHidGamepad::begin(void) {
    // Invoke base class begin()
    BLEClientService::begin();

    hidInfo.begin(this);
    hidReport.begin(this);

    // set notify callback
    hidReport.setNotifyCallback(gamepadClientNotifyCallback);  

    return true;
}

void BLEClientHidGamepad::setGamepadReportCallback(gamepad_callback_t fp) {
    gamepadCallback = fp;
}

bool BLEClientHidGamepad::discover(uint16_t connHandle) {
    // Call Base class discover
    VERIFY( BLEClientService::discover(connHandle) );

    // make as invalid until we found all chars
    _conn_hdl = BLE_CONN_HANDLE_INVALID;

    // Discover all characteristics
    Bluefruit.Discovery.discoverCharacteristic(connHandle, hidInfo, hidReport);

    VERIFY( hidInfo.discovered(), hidReport.discovered() );

    _conn_hdl = connHandle;
    return true;
}

bool BLEClientHidGamepad::getHidInfo(uint8_t info[4]) {
    return 4 == hidInfo.read(info, 4);
}

bool BLEClientHidGamepad::gamepadPresent()  {
    return hidReport.discovered();
}

bool BLEClientHidGamepad::enableGamepad()  {
    return hidReport.enableNotify();
}

bool BLEClientHidGamepad::disableGamepad()  {
    return hidReport.disableNotify();
}

void BLEClientHidGamepad::handleGamepadInput(uint8_t* data, uint16_t len) {
    varclr(&lastGamepadReport);
    memcpy(&lastGamepadReport, data, len);

    if ( gamepadCallback ) {
        gamepadCallback(&lastGamepadReport);
    }
}

void BLEClientHidGamepad::getGamepadReport(hid_gamepad_report_t* report) {
    memcpy(report, &lastGamepadReport, sizeof(hid_gamepad_report_t));
}

void BLEClientHidGamepad::gamepadClientNotifyCallback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len) {
    BLEClientHidGamepad& svc = (BLEClientHidGamepad&) chr->parentService();
    svc.handleGamepadInput(data, len);
}