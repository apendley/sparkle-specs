#include "Settings.h"
#include <Adafruit_EEPROM_I2C.h>
#include <cstddef> 

// Uncomment define below to enable debug logging in this file.
// #define LOGGER Serial
#include "Logger.h"

void Settings::begin(Adafruit_EEPROM_I2C* eep, bool eraseEeprom) {
    LOGFMT("Settings memory map size: %d bytes\n", sizeof(MemoryMap));

    if (eep == nullptr) {
        // There's no eeprom, and our initialized memory map already contains all of the default values, so we're done.
        LOGLN("EEPROM not found, settings will not be persisted.");
        return;
    }

    if (eraseEeprom) {
        LOGLN("Erasing eeprom header...");

        for (size_t i = 0; i < sizeof(Header); i++) {
            if (!eep->write(i, 0)) {
                LOGFMT("Error erasing eeprom at address 0x%X\n", i);
                break;
            }
        }
    }

    Header currentHeader = memoryMap.header;    
    Header savedHeader;
    eep->readObject(0, savedHeader);

    if (savedHeader.signature == currentHeader.signature) {
        LOGLN("Saved signature matches current signature");
    }
    else {
        LOGLN("Saved signature does not match current signature");
    }

    if (savedHeader.version == currentHeader.version) {
        LOGLN("Saved version matches current version");
    }
    else {
        LOGLN("Saved version does not match current version");
    }

    // If the signature and version match
    if ((currentHeader.signature == savedHeader.signature) && (currentHeader.version == savedHeader.version)) {
        // Load the reset of the contents of the eprom into our memory map.
        const size_t headerSize = sizeof(Header);
        bool readSuccess = eep->read(headerSize, ((uint8_t*)&memoryMap) + headerSize, sizeof(MemoryMap) - headerSize);

        if (readSuccess) {
            LOGLN("Settings loaded from eeprom!");
            eeprom = eep;
        }
        else {
            LOGLN("Failed to read settings from eeprom.");
        }
    } else {
        bool writeSuccess = eep->writeObject(0, memoryMap);

        if (writeSuccess) {
            LOGLN("Default settings written to eeprom!");
            eeprom = eep;
        }
        else {
            LOGLN("Failed to write default settings to eeprom");
        }
    }
}

bool Settings::hasEeprom() {
    return (eeprom != nullptr);
}

uint8_t Settings::sceneIndex() const {
    return memoryMap.sceneIndex;
}

void Settings::setSceneIndex(uint8_t i) {
    memoryMap.sceneIndex = i;
    write(offsetof(MemoryMap, sceneIndex), i, "Scene index saved!");
}

void Settings::increaseSceneBrightness(uint8_t amount) {
    int16_t b = memoryMap.sceneBrightness[memoryMap.sceneIndex] + amount;
    b = max((int16_t)0, min((int16_t)255, b));
    memoryMap.sceneBrightness[memoryMap.sceneIndex] = b;
    uint8_t address = sizeof(memoryMap.header) + sizeof(memoryMap.sceneIndex) + memoryMap.sceneIndex;
    write(address, memoryMap.sceneBrightness[memoryMap.sceneIndex], "Increased current scene brightness!");
}

void Settings::decreaseSceneBrightness(uint8_t amount) {
    int16_t b = memoryMap.sceneBrightness[memoryMap.sceneIndex] - amount;
    b = max((int16_t)0, min((int16_t)255, b));
    memoryMap.sceneBrightness[memoryMap.sceneIndex] = b;
    uint8_t address = sizeof(memoryMap.header) + sizeof(memoryMap.sceneIndex) + memoryMap.sceneIndex;
    write(address, memoryMap.sceneBrightness[memoryMap.sceneIndex], "Decreased current scene brightness!");
}

uint8_t Settings::sceneBrightness() const {
    return memoryMap.sceneBrightness[memoryMap.sceneIndex];
}

uint16_t Settings::shiftyEyesGetRingHue() {
    return memoryMap.shiftyEyesRingHue;
}

void Settings::shiftyEyesSetRingHue(uint16_t h) {
    memoryMap.shiftyEyesRingHue = h;
    writeObject(offsetof(MemoryMap, shiftyEyesRingHue), h, "Shifty eyes ring color saved!");
}

uint16_t Settings::shiftyEyesGetPupilHue() {
    return memoryMap.shiftyEyesPupilHue;
}

void Settings::shiftyEyesSetPupilHue(uint16_t h) {
    memoryMap.shiftyEyesPupilHue = h;
    writeObject(offsetof(MemoryMap, shiftyEyesPupilHue), h, "Shifty pupil hue saved!");
}

bool Settings::shiftyEyesHasMonsterPupils() {
    return memoryMap.shiftyEyesHasMonsterPupils;
}

void Settings::shiftyEyesSetHasMonsterPupils(bool has) {
    memoryMap.shiftyEyesHasMonsterPupils = has;
    write(offsetof(MemoryMap, shiftyEyesHasMonsterPupils), has, "Shifty eyes has monster pupils saved!");
}

uint8_t Settings::beamMode() const {
    return memoryMap.beamMode;
}

void Settings::beamSetMode(uint8_t m) {
    memoryMap.beamMode = m;
    write(offsetof(MemoryMap, beamMode), m, "Beam mode saved!");
}

uint16_t Settings::beamHue() const {
    return memoryMap.beamHue;
}

void Settings::beamSetHue(uint16_t h) {
    memoryMap.beamHue = h;
    writeObject(offsetof(MemoryMap, beamHue), h, "Beam hue saved!");
}

uint8_t Settings::beamSaturation() const {
    return memoryMap.beamSaturation;
}

void Settings::beamSetSaturation(uint8_t s) {
    memoryMap.beamSaturation = s;
    write(offsetof(MemoryMap, beamSaturation), s, "Beam saturation saved!");
}

uint8_t Settings::beamDisconnectedSpeed() const {
    return memoryMap.beamDisconnectedSpeed;
}

void Settings::beamDisconnectedSetSpeed(uint8_t s) {
    memoryMap.beamDisconnectedSpeed = s;
    write(offsetof(MemoryMap, beamDisconnectedSpeed), s, "Beam speed saved!");
}

uint16_t Settings::googlyRingsHue() const {
    return memoryMap.googlyRingsHue;
}

void Settings::googlyRingsSetHue(uint16_t h) {
    memoryMap.googlyRingsHue = h;
    writeObject(offsetof(MemoryMap, googlyRingsHue), h, "Googly rings hue saved!");
}

uint8_t Settings::googlyRingsSaturation() const {
    return memoryMap.googlyRingsSaturation;
}

void Settings::googlyRingsSetSaturation(uint8_t s) {
    memoryMap.googlyRingsSaturation = s;
    write(offsetof(MemoryMap, googlyRingsSaturation), s, "Googly rings saturation saved!");
}

bool Settings::volumeMeterUseCustomColor() const {
    return memoryMap.volumeMeterUseCustomColor;
}

void Settings::volumeMeterSetUseCustomColor(bool u) {
    memoryMap.volumeMeterUseCustomColor = u;
    write(offsetof(MemoryMap, volumeMeterUseCustomColor), u, "Audio rings use custom color saved!");
}

uint16_t Settings::volumeMeterHue() const {
    return memoryMap.volumeMeterHue;
}

void Settings::volumeMeterSetHue(uint16_t h) {
    memoryMap.volumeMeterHue = h;
    writeObject(offsetof(MemoryMap, volumeMeterHue), h, "Audio rings hue saved!");
}

uint8_t Settings::volumeMeterSaturation() const {
    return memoryMap.volumeMeterSaturation;
}

void Settings::volumeMeterSetSaturation(uint8_t s) {
    memoryMap.volumeMeterSaturation = s;
    write(offsetof(MemoryMap, volumeMeterSaturation), s, "Audio rings saturation saved!");
}

bool Settings::sparklesUseCustomColor() const {
    return memoryMap.sparklesUseCustomColor;
}

void Settings::sparklesSetUseCustomColor(bool u) {
    memoryMap.sparklesUseCustomColor = u;
    write(offsetof(MemoryMap, sparklesUseCustomColor), u, "Sparkles use custom color saved!");    
}

uint16_t Settings::sparklesHue() const {
    return memoryMap.sparklesHue;
}

void Settings::sparklesSetHue(uint16_t h) {
    memoryMap.sparklesHue = h;
    writeObject(offsetof(MemoryMap, sparklesHue), h, "Sparkles hue saved!");    
}

bool Settings::audioBarsUseCustomColor() const {
    return memoryMap.audioBarsUseCustomColor;
}

void Settings::audioBarsSetUseCustomColor(bool u) {
    memoryMap.audioBarsUseCustomColor = u;
    write(offsetof(MemoryMap, audioBarsUseCustomColor), u, "Audio bars use custom color saved!");
}

bool Settings::audioBarsSnowCapped() const {
    return memoryMap.audioBarsSnowCapped;
}

void Settings::audioBarsSetSnowCapped(bool s) {
    memoryMap.audioBarsSnowCapped = s;
    write(offsetof(MemoryMap, audioBarsSnowCapped), s, "Audio bars snow capped setting saved!");
}


uint16_t Settings::audioBarsHue() const {
    return memoryMap.audioBarsHue;
}

void Settings::audioBarsSetHue(uint16_t h) {
    memoryMap.audioBarsHue = h;
    writeObject(offsetof(MemoryMap, audioBarsHue), h, "Audio bars hue saved!");
}

uint8_t Settings::audioBarsSaturation() const {
    return memoryMap.audioBarsSaturation;
}

void Settings::audioBarsSetSaturation(uint8_t s) {
    memoryMap.audioBarsSaturation = s;
    write(offsetof(MemoryMap, audioBarsSaturation), s, "Audio bars saturation saved!");
}

bool Settings::marqueeUseCustomColor() const {
    return memoryMap.marqueeUseCustomColor;
}

void Settings::marqueeSetUseCustomColor(bool u) {
    memoryMap.marqueeUseCustomColor = u;
    write(offsetof(MemoryMap, marqueeUseCustomColor), u, "Marquee use custom color saved!");
}

uint16_t Settings::marqueeHue() const {
    return memoryMap.marqueeHue;
}

void Settings::marqueeSetHue(uint16_t h) {
    memoryMap.marqueeHue = h;
    writeObject(offsetof(MemoryMap, marqueeHue), h, "Marquee hue saved!");
}

uint8_t Settings::marqueeSaturation() const {
    return memoryMap.marqueeSaturation;
}

void Settings::marqueeSetSaturation(uint8_t s) {
    memoryMap.marqueeSaturation = s;
    write(offsetof(MemoryMap, marqueeSaturation), s, "Marquee saturation saved!");
}

uint8_t Settings::marqueeScrollDelay() const {
    return memoryMap.marqueeScrollDelay;
}

void Settings::marqueeSetScrollDelay(uint8_t s) {
    memoryMap.marqueeScrollDelay = s;
    write(offsetof(MemoryMap, marqueeScrollDelay), s, "Marquee scroll delay saved!");
}

void Settings::marqueeGetMessage(char* buffer, uint8_t messageBufferSize) const {
    if (memoryMap.marqueeMessageLength == 0) {
        buffer[0] = 0;
        return;
    }

    size_t bufferSize = min(memoryMap.marqueeMessageLength + 1, messageBufferSize);

    strncpy(buffer, memoryMap.marqueeMessage, bufferSize);
    buffer[bufferSize - 1] = 0;
}

void Settings::marqueeSetMessage(const char* message) {
    size_t bufferSize = min(strlen(message) + 1, textBufferSize);
    strncpy(memoryMap.marqueeMessage, message, bufferSize);
    memoryMap.marqueeMessage[bufferSize - 1] = 0;

    memoryMap.marqueeMessageLength = strlen(memoryMap.marqueeMessage);

    // Write a zero first, in case something bad happens while writing the string.
    write(offsetof(MemoryMap, marqueeMessageLength), 0, "Marquee message length set to zero");

    // Write the string
    writeBuffer(offsetof(MemoryMap, marqueeMessage), (uint8_t*)memoryMap.marqueeMessage, bufferSize, "Marquee message saved!");

    // Now that we've written the string, set the length.
    write(offsetof(MemoryMap, marqueeMessageLength), memoryMap.marqueeMessageLength, "Marquee message length saved!");
}

bool Settings::write(uint16_t addr, uint8_t value, const char* successMessage) {
    if (!hasEeprom()) {
        return false;
    }    

    if (eeprom->write(addr, value)) {
        LOGLN(successMessage);
        return true;
    }

    return false;
}

bool Settings::writeBuffer(uint16_t addr, uint8_t* buffer, uint16_t count, const char* successMessage) {
    if (!hasEeprom()) {
        return false;
    }

    if (eeprom->write(addr, buffer, count)) {
        LOGLN(successMessage);
        return true;
    }

    return false;
}

template <class T> 
bool Settings::writeObject(uint16_t addr, const T& object, const char* successMessage) {
    if (!hasEeprom()) {
        return 0;
    }

    uint16_t wrote = eeprom->writeObject(addr, object);

    if (wrote == sizeof(object)) {
        LOGLN(successMessage);
    }

    return wrote;
}
