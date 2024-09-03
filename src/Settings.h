#pragma once

#include <Arduino.h>
#include "Color.h"

class Adafruit_EEPROM_I2C;

class Settings {
public:
    static constexpr uint8_t textBufferSize = 128;
    typedef char Text[textBufferSize];

private:
    struct Header {
        // Help verify that we've written to the eeprom before
        uint32_t signature;

        // Increment this every time you make a change to the 
        // memory map to invalidate previously saved data.
        uint16_t version;
    } __attribute__((packed));

    struct MemoryMap {
        Header header = {0x3141, 4};

        uint8_t sceneIndex = 0;

        uint8_t shiftyEyesHasMonsterPupils = false;
        uint16_t shiftyEyesRingHue = 10922;
        uint16_t shiftyEyesPupilHue = 0;

        uint8_t beamMode = 0;
        uint16_t beamHue = 0;
        uint8_t beamSaturation = 255;
        uint8_t beamDisconnectedSpeed = 0;

        uint16_t googlyRingsHue = (65536 / 6) * 5;
        uint8_t googlyRingsSaturation = 255;

        uint8_t audioBarsUseCustomColor = 0;
        uint8_t audioBarsSnowCapped = 1;
        uint16_t audioBarsHue = 0;
        uint8_t audioBarsSaturation = 255;

        uint8_t volumeMeterUseCustomColor = 0;
        uint16_t volumeMeterHue = 0;
        uint8_t volumeMeterSaturation = 255;

        uint8_t sparklesUseCustomColor = 0;
        uint16_t sparklesHue = 0;

        uint8_t marqueeUseCustomColor = 0;
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
