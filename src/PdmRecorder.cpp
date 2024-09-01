// Adapted from MIT-licensed code at
// https://learn.adafruit.com/adafruit-eyelights-led-glasses-and-driver/music-reactive-lights-2
// by Phil "Paint Your Dragon" Burgess for Adafruit Industries

#include "PdmRecorder.h"
#include <PDM.h>

void PdmRecorder::reset() {
    samplesRead = 0;
    backBuffer = 0;

    for (int b = 0; b < 2; b++) {
        for (int i = 0; i < sampleCount; i++) {
            audioBuffer[b][i] = 0;
        }
    }
}

void PdmRecorder::startRecording() {
    reset();
    recording = true;
    busy = true;
    PDM.begin(1, 16000);
}

void PdmRecorder::stopRecording() {
    PDM.end();
    recording = false;
}

void PdmRecorder::sync() {
    if (!recording) {
        return;
    }

    if (busy) {
        return;
    }
    
    // Swap back/front buffers.
    backBuffer = 1 - backBuffer;

    // Start recording next batch.
    busy = true;
}

void PdmRecorder::readPdmData() {
    int bytesToRead = PDM.available();

    if (bytesToRead == 0) {
        return;
    }

    if (busy) {
         // Space remaining,
        int byteLimit = (sampleCount - samplesRead) * 2;

        // don't overflow!
        bytesToRead = min(bytesToRead, byteLimit);
        
        PDM.read(&audioBuffer[backBuffer][samplesRead], bytesToRead);

        // Increment counter
        samplesRead += bytesToRead / 2;

        // Buffer full?
        if (samplesRead >= sampleCount) {
            // Stop and reset counter for next time
            busy = false;
            samplesRead = 0;
        }
    }
    else {
        // Mic is off (code is busy) - must read but discard data. audioBuf[2] is a 'bit bucket' for this.
        PDM.read(audioBuffer[2], bytesToRead);        
    }
}

float PdmRecorder::magnitude() const {
    const int16_t* buffer = frontBuffer();
    float mean = 0;

    for (int i = 0; i < sampleCount; i++) {
        mean += buffer[i];
    }
    mean /= sampleCount;
    
    int32_t minBuf = int(mean);
    float samplesSum = 0;

    for (int i = 0; i < sampleCount; i++) {
        int16_t sample = buffer[i];
        samplesSum += float(sample - minBuf) * (sample - minBuf);
    }

    // Serial.printf("mean: %f, mag: %f\n", mean, magnitude);    
    return sqrt(samplesSum / sampleCount);
}