#pragma once

#include <Arduino.h>
#include "Color.h"

class Adafruit_EEPROM_I2C;

class Settings {
public:
    static constexpr uint8_t sceneCount = 7;

    static constexpr uint8_t textBufferSize = 128;
    typedef char Text[textBufferSize];

    // 32 = 8 brightness settings
    static constexpr uint8_t defaultBrightnessIncrement = 32;

private:
    struct Header {
        // Help verify that we've written to the eeprom before.
        // Provides a method of invalidatin g previously written data.
        uint32_t signature;

        // Increment this every time you make a change to the 
        // memory map to invalidate previously saved data when developing.
        uint16_t version;
    } __attribute__((packed));

    struct MemoryMap {
        Header header = {0xBEEF, 1};

        uint8_t sceneIndex = 0;

        // Careful; we're manually calculating the address for each mode's brightness when writing to the EEPROM.
        // If any variables are added above this, their size will need to be added to the base scene brightness address.
        uint8_t sceneBrightness[sceneCount] = {192, 159, 128, 96, 32, 64, 96};

        bool shiftyEyesHasMonsterPupils = false;
        uint16_t shiftyEyesRingHue = 10922;
        uint16_t shiftyEyesPupilHue = 0;

        uint8_t beamMode = 0;
        uint16_t beamHue = 0;
        uint8_t beamSaturation = 255;
        uint8_t beamDisconnectedSpeed = 0;

        uint16_t googlyRingsHue = 54613;
        uint8_t googlyRingsSaturation = 255;

        bool audioBarsUseCustomColor = 0;
        uint8_t audioBarsSnowCapped = 1;
        uint16_t audioBarsHue = 0;
        uint8_t audioBarsSaturation = 255;

        bool volumeMeterUseCustomColor = 0;
        uint16_t volumeMeterHue = 0;
        uint8_t volumeMeterSaturation = 255;

        bool sparklesUseCustomColor = 0;
        uint16_t sparklesHue = 0;

        bool marqueeUseCustomColor = 0;
        uint16_t marqueeHue = 0;
        uint8_t marqueeSaturation = 255;
        uint8_t marqueeScrollDelay = 25;

        uint8_t marqueeMessageLength = 6;
        Text marqueeMessage = "Hello!";
    } __attribute__((packed));

public:
    Settings() {}

    void begin(Adafruit_EEPROM_I2C* _eeprom = nullptr, bool eraseEeprom = false);
    bool hasEeprom();

    uint8_t sceneIndex() const;
    void setSceneIndex(uint8_t i);

    uint8_t sceneBrightness() const;
    void increaseSceneBrightness(uint8_t amount = defaultBrightnessIncrement);    
    void decreaseSceneBrightness(uint8_t amount = defaultBrightnessIncrement);

    uint16_t shiftyEyesGetRingHue();
    void shiftyEyesSetRingHue(uint16_t h);

    uint16_t shiftyEyesGetPupilHue();
    void shiftyEyesSetPupilHue(uint16_t h);

    bool shiftyEyesHasMonsterPupils();
    void shiftyEyesSetHasMonsterPupils(bool has);

    uint8_t beamMode() const;
    void beamSetMode(uint8_t m);

    uint16_t beamHue() const;
    void beamSetHue(uint16_t h);

    uint8_t beamSaturation() const;
    void beamSetSaturation(uint8_t s);

    uint8_t beamDisconnectedSpeed() const;
    void beamDisconnectedSetSpeed(uint8_t s);

    uint16_t googlyRingsHue() const;
    void googlyRingsSetHue(uint16_t h);

    uint8_t googlyRingsSaturation() const;
    void googlyRingsSetSaturation(uint8_t s);

    bool volumeMeterUseCustomColor() const;
    void volumeMeterSetUseCustomColor(bool u);

    uint16_t volumeMeterHue() const;
    void volumeMeterSetHue(uint16_t h);

    uint8_t volumeMeterSaturation() const;
    void volumeMeterSetSaturation(uint8_t s);

    bool sparklesUseCustomColor() const;
    void sparklesSetUseCustomColor(bool u);

    uint16_t sparklesHue() const;
    void sparklesSetHue(uint16_t h);    

    bool audioBarsSnowCapped() const;
    void audioBarsSetSnowCapped(bool s);

    bool audioBarsUseCustomColor() const;
    void audioBarsSetUseCustomColor(bool u);

    uint16_t audioBarsHue() const;
    void audioBarsSetHue(uint16_t h);

    uint8_t audioBarsSaturation() const;
    void audioBarsSetSaturation(uint8_t s);

    bool marqueeUseCustomColor() const;
    void marqueeSetUseCustomColor(bool u);

    uint16_t marqueeHue() const;
    void marqueeSetHue(uint16_t h);

    uint8_t marqueeSaturation() const;
    void marqueeSetSaturation(uint8_t s);

    uint8_t marqueeScrollDelay() const;
    void marqueeSetScrollDelay(uint8_t s);    

    void marqueeGetMessage(char* buffer, uint8_t bufferSize) const;
    void marqueeSetMessage(const char* message);

private:
    bool write(uint16_t addr, uint8_t value, const char* successMessage);
    bool writeBuffer(uint16_t addr, uint8_t* buffer, uint16_t count, const char* successMessage);
    
    template <class T> 
    bool writeObject(uint16_t addr, const T &value, const char* successMessage);

private:
    MemoryMap memoryMap;
    Adafruit_EEPROM_I2C* eeprom = nullptr;
};
