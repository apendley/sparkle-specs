// Adapted from MIT-licensed code at
// https://learn.adafruit.com/adafruit-eyelights-led-glasses-and-driver/music-reactive-lights-2
// by Phil "Paint Your Dragon" Burgess for Adafruit Industries

#pragma once

#include <Arduino.h>

class PdmRecorder {
public:
    static constexpr int32_t sampleCount = 512;

public:
    PdmRecorder() = default;
    ~PdmRecorder() = default;

    void startRecording();
    void stopRecording();

    inline bool isRecording() { 
        return recording;
    }

    // This needs to be called PDM data ready callback in the main file.
    void readPdmData();

    // Call this once each loop.
    void sync();

    // This is the "draw" buffer, i.e. the buffer
    // that's not currently being written to by the mic.
    inline const int16_t* frontBuffer() const {
        return audioBuffer[1 - backBuffer];
    }

    // Calculate magnitude of the data in frontBuffer().
    float magnitude() const;

private:
    void reset();

private:
    // Front (draw) buffer, back (write) buffer, and garbage buffer for everything else.
    int16_t audioBuffer[3][sampleCount];
    uint8_t backBuffer = 0;

    volatile bool busy = false;
    volatile int samplesRead = 0;

    bool recording = false;
};