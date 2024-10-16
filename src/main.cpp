// Pairing:
// 1. Turn off the adapter that you want to pair with the glasses.
// 2. With the glasses powered on, press and hold the mode button for 3 seconds.
// 3. When the green LED lights up, the glasses are in pairing mode.
// 4. Press and hold both nunchuck buttons while powering on the adapter.
// 5. When the green LED lights up, the adapter is now in pairing mode.
// 6. When successfully paired, the blue LED will be solidly lit (not flashing) on the glasses and the adapter.

#include <Arduino.h>
#include <Adafruit_IS31FL3741.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_NeoPixel.h>
#include <PDM.h>
#include <Adafruit_EEPROM_I2C.h>
#include "Config.h"
#include "BLEClientHidGamepad.h"
#include "Gamepad.h"
#include "SoftGamepad.h"
#include "PdmRecorder.h"
#include "Device.h"
#include "DigitalInput.h"
#include "ShakeDetector.h"
#include "UartCommandParser.h"
#include "Settings.h"

#include "Scene.h"
#include "scenes/ShiftyEyes/ShiftyEyesScene.h"
#include "scenes/Beam/BeamScene.h"
#include "scenes/GooglyRings/GooglyRingsScene.h"
#include "scenes/AudioBars/AudioBarsScene.h"
#include "scenes/VolumeMeter/VolumeMeterScene.h"
#include "scenes/Marquee/MarqueeScene.h"
#include "scenes/Sparkles/SparklesScene.h"

// Uncomment define below to enable debug logging in this file.
// #define LOGGER Serial
#include "Logger.h"

static const char* revision = "1.0.2";

////////////////////////////
// Scene management
////////////////////////////
class SceneCreator {
public:
    virtual Scene* createScene(Device& device) = 0;
};

template<typename SceneClass>
class SceneFactory : public SceneCreator {
public:
    virtual Scene* createScene(Device& device) override {
        return new SceneClass(device);
    }
};

SceneFactory<ShiftyEyesScene> shiftyEyesSceneFactory;
SceneFactory<BeamScene> beamSceneFactory;
SceneFactory<GooglyRingsScene> googlyRingsSceneFactory;
SceneFactory<VolumeMeterScene> volumeMeterSceneFactory;
SceneFactory<AudioBarsScene> audioBarsSceneFactory;
SceneFactory<SparklesScene> sparklesSceneFactory;
SceneFactory<MarqueeScene> marqueeSceneFactory;

SceneCreator* sceneFactories[] = {
    &shiftyEyesSceneFactory,
    &beamSceneFactory,
    &googlyRingsSceneFactory,
    &volumeMeterSceneFactory,    
    &audioBarsSceneFactory,    
    &sparklesSceneFactory,
    &marqueeSceneFactory,    
};

uint8_t sceneFactoryCount = sizeof(sceneFactories) / sizeof(sceneFactories[0]);
uint8_t sceneIndex = 0;
Scene* currentScene = nullptr;

////////////////////////////
// Device
////////////////////////////
Adafruit_LIS3DH accel;
Adafruit_EyeLights_buffered glasses(true);
Adafruit_EEPROM_I2C eeprom;
Gamepad gamepad;
SoftGamepad softGamepad;
PdmRecorder pdmRecorder;
Settings settings;

Device device(
    accel, 
    glasses, 
    pdmRecorder,
    gamepad, 
    softGamepad,
    settings
);

// Used for detecting shakes from the nunchuck to change scenes.
ShakeDetector gamepadShake(3);

// Used for pairing and changing scenes.
DigitalPinInput modeButton(4);
const uint32_t pairingHoldDuration = 3000;
uint32_t pairingHeldTime = 0;
bool ignoreModeButtonFell = false;

////////////////////////////
// BLE
////////////////////////////
BLEUart bleUart;
BLEDis  bleUartDis;
BLEClientHidGamepad hidGamepad;
bool isPairing = false;
const uint16_t invalidConnectionHandle = BLE_MAX_CONNECTION;
uint16_t gamepadConnectionHandle = invalidConnectionHandle;

UartCommand::Parser uartCommandParser;
uint32_t bleUartLastRxTime = 0;

const int bleUartPairedLedPin = 2;
Adafruit_NeoPixel pixel(1, 3, NEO_GRB + NEO_KHZ800);

const uint16_t ledPairingColor = Color::RGB(0, 2, 0).packed();        
const uint16_t ledConnectedColor = Color::RGB(0, 0, 2).packed();

////////////////////////////
// Timing
////////////////////////////
uint32_t millisLast = 0;

////////////////////////////
// Forward declarations
////////////////////////////
bool initGlasses();
void initSettings(bool eepromInitialized);
void initBle();

void initScene();
void setScene(Scene* scene);
void nextScene();
void previousScene();

void updateModeSelection(uint32_t dt);
void updateNunchuck();
void readPdmData();
void updateConnectionLeds();
void updateBleUart();
void updateBleUartTimeout();

void uartFlush();

// callbacks
void uartCommandColor(const Color::RGB& c);
void uartCommandButtonEvent(const ButtonEvent& e);
void uartCommandText(const char* text);
void uartCommandError(const char* msg);

void scanCallback(ble_gap_evt_adv_report_t* report);
void centralConnectCallback(uint16_t connHandle);
bool centralPairPasskeyCallback(uint16_t conn_hdl, uint8_t const passkey[6], bool match_request);
void centralPairCompleteCallback(uint16_t connHandle, uint8_t authStatus);
void centralConnectionSecuredCallback(uint16_t connHandle);
void centralDisconnectCallback(uint16_t connHandle, uint8_t reason);
void peripheralConnectCallback(uint16_t connHandle);
void peripheralDisconnectCallback(uint16_t conn_hdl, uint8_t reason);

////////////////////////////
// Setup
////////////////////////////
void setup() {
    pixel.begin();
    pixel.fill(0);
    pixel.show();

    pinMode(bleUartPairedLedPin, OUTPUT);
    analogWrite(bleUartPairedLedPin, 0);    

    Serial.begin(115200);
    // while(!Serial) { delay(10); }
  
    // Both I2C devices can use 400kHz, but the accel driver sets
    // the bus speed to 100kHz, while the glasses sets it to
    // 400kHz. So, initialize the glasses driver last for max speed.
    // Initialize EEPROM and settings.
    bool eepromInitialized = eeprom.begin();

    if (!accel.begin()) {
        // TODO: What?
    }        

    if (!initGlasses()) {
        // TODO: What?
    }

    // Forward signal to pdmRecorder when data is ready.
    PDM.onReceive(readPdmData);

    // Initialize mode button.
    modeButton.begin();    

    initSettings(eepromInitialized);

    // Everything else
    initBle();
    initScene();

    // Reset timer
    millisLast = millis();
}

////////////////////////////
// Loop
////////////////////////////
void loop() {
    uint32_t now = millis();
    uint32_t dt = now - millisLast;
    millisLast = now;

    // Let at least 1 ms elapse.
    if (dt == 0) {
        delay(1);
        return;
    }

    // Serial.printf("dt: %d\n", dt);

    updateBleUart();
    updateBleUartTimeout();
    updateConnectionLeds();
    softGamepad.update();
    updateNunchuck();
    pdmRecorder.sync();

    if (currentScene != nullptr) {
        currentScene->update(dt);
    }

    updateModeSelection(dt);
}

////////////////////////////
// Function definitions
////////////////////////////
bool initGlasses() {
    if (!glasses.begin(IS3741_ADDR_DEFAULT)) {
        return false;
    }

    // Set brightness to max and bring controller out of shutdown state
    glasses.setLEDscaling(0xFF);
    glasses.setGlobalCurrent(0xFF);
    glasses.enable(true);

    // Set ring brightness and rotation.
    glasses.setRotation(0);

    // Clear glasses
    glasses.fill(0);
    glasses.show();

    return true;
}

void initSettings(bool eepromInitialized) {
    // We'll need to know if the user wants to do this when we initialize the settings.
    bool eraseEeprom = false;

    if (eepromInitialized) {
        // Wait just a little bit to give the use a chance to reset the eeprom if they want.
        // If the press the button stop waiting to improve responsiveness.
        uint32_t startTime = millis();
        while ((millis() - startTime) < 250) {
            delay(10);
            modeButton.update();

            if (modeButton.isDown()) {
                break;
            }
        }

        // If the mode button is pressed, the user wants to resetore default settings.
        eraseEeprom = modeButton.isDown();

        // Make the LEDs blue to acknowledge that we are erasing the EEPROM.
        if (eraseEeprom) {
            const Color::RGB c(0, 0, 32);
            glasses.fillScreen(c.packed565());
            glasses.left_ring.fill(c.packed());
            glasses.right_ring.fill(c.packed());
            glasses.show();
        }
    } 
    else {
        LOGLN("EEPROM not found, or failed to initialize.");
    }    

    // Initialize the settings based on the information we've gathered
    settings.begin(eepromInitialized ? &eeprom : nullptr, eraseEeprom);

    if (eraseEeprom) {
        // Wait for the mode button to be released before we continue
        while(modeButton.isDown()) {
            modeButton.update();
            delay(10);
        }

        // Turn off all the LEDs again
        glasses.fill(0);
        glasses.left_ring.fill(0);
        glasses.right_ring.fill(0);
        glasses.show();        
    }
}

void initBle() {
    // Set up the command parser
    uartCommandParser.setColorCallback(uartCommandColor);
    uartCommandParser.setButtonEventCallback(uartCommandButtonEvent);
    uartCommandParser.setTextCallback(uartCommandText);
    uartCommandParser.setErrorCallback(uartCommandError);

    // General setup
    Bluefruit.autoConnLed(false);
    Bluefruit.begin(1, 1);
    Bluefruit.setName(BLE_ADVERTISING_NAME);
    Bluefruit.setConnLedInterval(250);

    // Set up peripheral as UART to connect to app
    {
        bleUartDis.setManufacturer(BLE_MANUFACTURER_NAME);
        bleUartDis.setModel(BLE_MODEL);
        bleUartDis.setFirmwareRev(revision);
        bleUartDis.setSoftwareRev(revision);
        bleUartDis.setSerialNum(BLE_SERIAL_NUMBER);
        bleUartDis.begin();
        
        bleUart.begin();
        
        Bluefruit.Periph.setConnectCallback(peripheralConnectCallback);
        Bluefruit.Periph.setDisconnectCallback(peripheralDisconnectCallback);

        Bluefruit.ScanResponse.addName();

        Bluefruit.Advertising.addTxPower();
        Bluefruit.Advertising.addService(bleUart);
        Bluefruit.Advertising.restartOnDisconnect(true);

        // in unit of 0.625 ms        
        Bluefruit.Advertising.setInterval(32, 244);

        // number of seconds in fast mode        
        Bluefruit.Advertising.setFastTimeout(30);

        // 0 = Don't stop advertising after n seconds        
        Bluefruit.Advertising.start(0);
    }

    // Set up gamepad support
    {
        // display = true, yes/no = true, keyboard = false.
        // We're not telling the whole truth here. We do have a display,
        // but we don't have yes/not buttons or a keyboard,
        // and even though we have a display, we aren't using it for pairing purposes.
        // In the pairing callback, we allow pairing if the glasses were previously
        // put in pairing mode, otherwise we don't.
        Bluefruit.Security.setIOCaps(true, true, false);
        Bluefruit.Security.setMITM(true);
        Bluefruit.Security.setPairPasskeyCallback(centralPairPasskeyCallback);
        Bluefruit.Security.setPairCompleteCallback(centralPairCompleteCallback);
        Bluefruit.Security.setSecuredCallback(centralConnectionSecuredCallback);        

        hidGamepad.begin();

        Bluefruit.Central.setConnectCallback(centralConnectCallback);
        Bluefruit.Central.setDisconnectCallback(centralDisconnectCallback);

        Bluefruit.Scanner.setRxCallback(scanCallback);
        Bluefruit.Scanner.restartOnDisconnect(true);

        // in unit of 0.625 ms    
        Bluefruit.Scanner.setInterval(160, 80);

        // only report gamepad HID service.
        Bluefruit.Scanner.filterService(hidGamepad);
        Bluefruit.Scanner.useActiveScan(false);

        // 0 = Don't stop scanning after n seconds    
        Bluefruit.Scanner.start(0);
    }
}

void readPdmData() {
    pdmRecorder.readPdmData();
}

void updateConnectionLeds() {
    // Show the red LED if we're connected to a UART peripheral.
    if (Bluefruit.Periph.connected()) {
        analogWrite(bleUartPairedLedPin, 8);
    } 
    else {
        analogWrite(bleUartPairedLedPin, 0);
    }

    if (isPairing) {
        pixel.fill(ledPairingColor);        
    } 
    else if (Bluefruit.Central.connected()) {
        pixel.fill(ledConnectedColor);
    } 
    else {
        pixel.fill(0);
    }

    pixel.show();
}

void initScene() {
    sceneIndex = settings.sceneIndex();;
    Scene* s = sceneFactories[sceneIndex]->createScene(device);
    setScene(s);
}

void setScene(Scene* scene) {
    if (currentScene != nullptr) {
        currentScene->exit();
        delete currentScene;
    }

    currentScene = scene;

    if (currentScene != nullptr) {
        currentScene->enter();
    }
}

void nextScene() {
    gamepadShake.reset(false);
    softGamepad.reset();

    sceneIndex++;

    if (sceneIndex >= sceneFactoryCount) {
        sceneIndex = 0;
    }

    Scene* s = sceneFactories[sceneIndex]->createScene(device);
    setScene(s);
    settings.setSceneIndex(sceneIndex);
}

void previousScene() {
    gamepadShake.reset(false);
    softGamepad.reset();


    if (sceneIndex == 0) {
        sceneIndex = sceneFactoryCount - 1;
    } else {
        sceneIndex--;
    }

    Scene* s = sceneFactories[sceneIndex]->createScene(device);
    setScene(s);
    settings.setSceneIndex(sceneIndex);    
}

void updateModeSelection(uint32_t dt) {
   modeButton.update();

    if (modeButton.wasPressed()) {
        if (isPairing) {
            LOGLN("Leaving pairing mode");
            isPairing = false;
            ignoreModeButtonFell = true;
        } 
        else {
            pairingHeldTime = 0;
            ignoreModeButtonFell = false;            
        }
    }
    else if (modeButton.wasReleased()) {
        if (ignoreModeButtonFell) {
            ignoreModeButtonFell = false;
        }
        else {
            nextScene();
        }
    }
    else if (modeButton.isDown() && !isPairing) {
        bool wasHeld = pairingHeldTime < pairingHoldDuration;
        pairingHeldTime += dt;
        bool isFinishedHolding = pairingHeldTime >= pairingHoldDuration;

        // Trigger pairing
        if (wasHeld && isFinishedHolding) {
            // Disconnect from currently connected gamepad, if connected.
            BLEConnection* conn = Bluefruit.Connection(gamepadConnectionHandle);

            if (conn != nullptr) {
                LOGLN("Disconnecting from gamepad...");
                conn->disconnect();
            }

            // Ignore the next button release, so we don't trigger a scene change.
            ignoreModeButtonFell = true;                        

            // Clearing the bonds can take a second or two, so:
            // 1) Turn the pairing LED on now
            pixel.fill(ledPairingColor);
            pixel.show();            

            // 2) Clear the bonds
            Bluefruit.Central.clearBonds();

            // 3) Reset the scene timer to prevent a huge dt on the next loop.
            millisLast = millis();

            // We are now pairing.
            isPairing = true;
            LOGLN("Entering pairing mode...");
        }
    }
    else if (softGamepad.wasPressed(softGamepad.buttonRight) || gamepadShake.shakeDetected()) {
        nextScene();
    }
    else if (softGamepad.wasPressed(softGamepad.buttonLeft)) {
        previousScene();
    } 
    else if (softGamepad.wasPressed(softGamepad.buttonUp)) {
        settings.increaseSceneBrightness();
    }
    else if (softGamepad.wasPressed(softGamepad.buttonDown)) {
        settings.decreaseSceneBrightness();
    }
}

void updateNunchuck() {
    if (!hidGamepad.discovered()) {
        return;
    }
  
    hid_gamepad_report_t report;
    hidGamepad.getGamepadReport(&report);
    gamepad.update(report);

    const Gamepad::Report& report1 = gamepad.getPreviousReport();
    const Gamepad::Report& report2 = gamepad.getReport();

    ShakeDetector::Reading r1(report1.ax, report1.ay, report1.az);
    ShakeDetector::Reading r2(report2.ax, report2.ay, report2.az);
    gamepadShake.update(r1, r2);
}

void scanCallback(ble_gap_evt_adv_report_t* report) {
    if (isPairing) {
        LOGLN("Connecting...");
        Bluefruit.Central.connect(report);        
    }
    else {
        // Check if already bonded. We don't need the actual key, just need the struct for calling the function below.    
        bond_keys_t peer_bkeys;

        if (bond_load_keys(BLE_GAP_ROLE_CENTRAL, &report->peer_addr, &peer_bkeys)) {
            LOGLN("Connecting to previously paired device...");
            Bluefruit.Central.connect(report);
            Bluefruit.Scanner.stop();
            return;
        } 
        else {
            // Warning: this will spam the serial console if there is a non-paired peripheral around.
            // LOGLN("Scan callback: ignoring non-paired peripheral.");
        }

        // Otherwise, ignore the device and resume scanning.
        // LOGLN("Scanning...");
        Bluefruit.Scanner.resume();
    }
}

void centralConnectCallback(uint16_t connHandle) {
    BLEConnection* conn = Bluefruit.Connection(connHandle);

    LOG("Connected. Discovering HID service...");

    if (hidGamepad.discover(connHandle)) {
        LOGLN("Found gamepad");

        if (isPairing && !conn->bonded()) {
            LOGLN("Pairing gamepad...");
            conn->requestPairing();
        }
    } 
    else {
        // No point in connecting if we can't find the gamepad HID service.
        LOGLN("No gamepad found.");
        conn->disconnect();
    }
}

void centralConnectionSecuredCallback(uint16_t connHandle) {
    BLEConnection* conn = Bluefruit.Connection(connHandle);

    if (!conn->secured()) {
        LOGLN("Connection not secured, requesting pairing");
        // It is possible that connection is still not secured by this time.
        // This happens (central only) when we try to encrypt connection using stored bond keys
        // but peer reject it (probably it remove its stored key).
        // Therefore we will request an pairing again --> callback again when encrypted
        conn->requestPairing();
    } else {
        LOGLN("Connection secured");

        isPairing = false;
        gamepadConnectionHandle = connHandle;

        if (hidGamepad.gamepadPresent()) {
            hidGamepad.enableGamepad();
            device.setGamepadConnected(true);
            gamepadShake.reset();

            if (currentScene != nullptr) {
                currentScene->gamepadConnected();
            }
        }
    }
}

bool centralPairPasskeyCallback(uint16_t connHandle, uint8_t const passkey[6], bool matchRequest) {
    LOGFMT("pairPasskeyCallback, isPairing: %d\n", isPairing);
    return isPairing;
}

void centralPairCompleteCallback(uint16_t connHandle, uint8_t authStatus) {
    // Handy reference for authStatus values.
    // #define BLE_GAP_SEC_STATUS_SUCCESS                0x00  /**< Procedure completed with success. */
    // #define BLE_GAP_SEC_STATUS_TIMEOUT                0x01  /**< Procedure timed out. */
    // #define BLE_GAP_SEC_STATUS_PDU_INVALID            0x02  /**< Invalid PDU received. */
    // #define BLE_GAP_SEC_STATUS_RFU_RANGE1_BEGIN       0x03  /**< Reserved for Future Use range #1 begin. */
    // #define BLE_GAP_SEC_STATUS_RFU_RANGE1_END         0x80  /**< Reserved for Future Use range #1 end. */
    // #define BLE_GAP_SEC_STATUS_PASSKEY_ENTRY_FAILED   0x81  /**< Passkey entry failed (user canceled or other). */
    // #define BLE_GAP_SEC_STATUS_OOB_NOT_AVAILABLE      0x82  /**< Out of Band Key not available. */
    // #define BLE_GAP_SEC_STATUS_AUTH_REQ               0x83  /**< Authentication requirements not met. */
    // #define BLE_GAP_SEC_STATUS_CONFIRM_VALUE          0x84  /**< Confirm value failed. */
    // #define BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP       0x85  /**< Pairing not supported.  */
    // #define BLE_GAP_SEC_STATUS_ENC_KEY_SIZE           0x86  /**< Encryption key size. */
    // #define BLE_GAP_SEC_STATUS_SMP_CMD_UNSUPPORTED    0x87  /**< Unsupported SMP command. */
    // #define BLE_GAP_SEC_STATUS_UNSPECIFIED            0x88  /**< Unspecified reason. */
    // #define BLE_GAP_SEC_STATUS_REPEATED_ATTEMPTS      0x89  /**< Too little time elapsed since last attempt. */
    // #define BLE_GAP_SEC_STATUS_INVALID_PARAMS         0x8A  /**< Invalid parameters. */
    // #define BLE_GAP_SEC_STATUS_DHKEY_FAILURE          0x8B  /**< DHKey check failure. */
    // #define BLE_GAP_SEC_STATUS_NUM_COMP_FAILURE       0x8C  /**< Numeric Comparison failure. */
    // #define BLE_GAP_SEC_STATUS_BR_EDR_IN_PROG         0x8D  /**< BR/EDR pairing in progress. */
    // #define BLE_GAP_SEC_STATUS_X_TRANS_KEY_DISALLOWED 0x8E  /**< BR/EDR Link Key cannot be used for LE keys. */
    // #define BLE_GAP_SEC_STATUS_RFU_RANGE2_BEGIN       0x8F  /**< Reserved for Future Use range #2 begin. */
    // #define BLE_GAP_SEC_STATUS_RFU_RANGE2_END         0xFF  /**< Reserved for Future Use range #2 end. */
    LOGFMT("Pairing status: 0x%X\n", authStatus);    

    if (authStatus != BLE_GAP_SEC_STATUS_SUCCESS) {
        LOGLN("Pairing failed, disconnecting.");
        BLEConnection* conn = Bluefruit.Connection(connHandle);
        conn->disconnect();
    }
}

void centralDisconnectCallback(uint16_t connHandle, uint8_t reason) {
    // Handy reference for reason values.
    // #define BLE_HCI_STATUS_CODE_SUCCESS                                0x00
    // #define BLE_HCI_STATUS_CODE_UNKNOWN_BTLE_COMMAND                   0x01
    // #define BLE_HCI_STATUS_CODE_UNKNOWN_CONNECTION_IDENTIFIER          0x02
    // #define BLE_HCI_AUTHENTICATION_FAILURE                             0x05
    // #define BLE_HCI_STATUS_CODE_PIN_OR_KEY_MISSING                     0x06
    // #define BLE_HCI_MEMORY_CAPACITY_EXCEEDED                           0x07
    // #define BLE_HCI_CONNECTION_TIMEOUT                                 0x08
    // #define BLE_HCI_STATUS_CODE_COMMAND_DISALLOWED                     0x0C
    // #define BLE_HCI_STATUS_CODE_INVALID_BTLE_COMMAND_PARAMETERS        0x12
    // #define BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION                  0x13
    // #define BLE_HCI_REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES        0x14
    // #define BLE_HCI_REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF            0x15
    // #define BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION                   0x16
    // #define BLE_HCI_UNSUPPORTED_REMOTE_FEATURE                         0x1A
    // #define BLE_HCI_STATUS_CODE_INVALID_LMP_PARAMETERS                 0x1E
    // #define BLE_HCI_STATUS_CODE_UNSPECIFIED_ERROR                      0x1F
    // #define BLE_HCI_STATUS_CODE_LMP_RESPONSE_TIMEOUT                   0x22
    // #define BLE_HCI_STATUS_CODE_LMP_ERROR_TRANSACTION_COLLISION        0x23
    // #define BLE_HCI_STATUS_CODE_LMP_PDU_NOT_ALLOWED                    0x24
    // #define BLE_HCI_INSTANT_PASSED                                     0x28
    // #define BLE_HCI_PAIRING_WITH_UNIT_KEY_UNSUPPORTED                  0x29
    // #define BLE_HCI_DIFFERENT_TRANSACTION_COLLISION                    0x2A
    // #define BLE_HCI_PARAMETER_OUT_OF_MANDATORY_RANGE                   0x30
    // #define BLE_HCI_CONTROLLER_BUSY                                    0x3A
    // #define BLE_HCI_CONN_INTERVAL_UNACCEPTABLE                         0x3B
    // #define BLE_HCI_DIRECTED_ADVERTISER_TIMEOUT                        0x3C
    // #define BLE_HCI_CONN_TERMINATED_DUE_TO_MIC_FAILURE                 0x3D
    // #define BLE_HCI_CONN_FAILED_TO_BE_ESTABLISHED                      0x3E
    LOGFMT("Disconnected from gamepad, reason: 0x%X\n", reason);

    if (gamepadConnectionHandle == connHandle) {
        gamepadConnectionHandle = invalidConnectionHandle;

        device.setGamepadConnected(false);

        if (currentScene != nullptr) {
            currentScene->gamepadDisconnected();
        }        
    }

    hidGamepad.disableGamepad();
}

void peripheralConnectCallback(uint16_t conn_hdl) {
    LOGLN("Connected to UART client");
}

void peripheralDisconnectCallback(uint16_t conn_hdl, uint8_t reason) {
    LOGFMT("Disconnected from UART client, reason: 0x%X\n", reason);
}

void updateBleUart() {
    if (!Bluefruit.Periph.connected()) {
        return;
    }

    if (!bleUart.notifyEnabled()) {
        return;
    }

    #if defined(LOGGER)
    if (bleUart.available()) {
        LOGLN("BLE UART packed received");
    }
    #endif

    while (bleUart.available()) {
        uartCommandParser.rx(bleUart.read());
        bleUartLastRxTime = millis();
    }
}

void updateBleUartTimeout() {
    if (uartCommandParser.isIdle()) {
        return;
    }

    uint32_t now = millis();

    if (now - bleUartLastRxTime > 1000) {
        LOGLN("Command parsing timed out waiting for data");
        bleUartLastRxTime = now;
        uartCommandParser.reset();
    }    
}

void uartFlush() {
    if (Bluefruit.connected() && bleUart.notifyEnabled()) {
        LOGLN("Flushing BLE UART packet");
        while (bleUart.available()) {
            bleUart.read();
        }            
    }
}

void uartCommandColor(const Color::RGB& c) {
    LOGFMT("Received color: r: %d, g: %d, b: %d\n", c.r, c.g, c.b);

    if (currentScene != nullptr) {
        currentScene->receivedColor(c);
    }
}

void uartCommandButtonEvent(const ButtonEvent& e) {
    LOGFMT("Received button event: index: %d, state: %d\n", e.index, e.state);
    softGamepad.event(e);
}

void uartCommandText(const char* text) {
    LOGFMT("Received text: %s\n", text);

    if (currentScene) {
        currentScene->receivedText(text);
    }
}

void uartCommandError(const char* msg) {
    LOGFMT("Error: %s\n", msg);
    uartFlush();
}