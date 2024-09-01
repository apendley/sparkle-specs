// Adapted from MIT-licensed code at
// https://learn.adafruit.com/adafruit-eyelights-led-glasses-and-driver/music-reactive-lights-2
// by Phil "Paint Your Dragon" Burgess for Adafruit Industries

#pragma once

#include <Arduino.h>
#include <Adafruit_ZeroFFT.h>

template<uint8_t COLUMN_COUNT, uint32_t SAMPLE_COUNT>
class ColumnSpectrumizer {
public:
    static constexpr uint8_t columnCount = COLUMN_COUNT;
    static constexpr uint32_t sampleCount = SAMPLE_COUNT;

public:
    // Assumes signed 16-bit samples.
    ColumnSpectrumizer(uint16_t _lowBin, uint16_t _highBin) :
        lowBin(_lowBin),
        highBin(_highBin)
    {
        // e.g. 8 = 256 bin spectrum
        uint8_t spectrumBits = (int)log2f((float)sampleCount / 2);

        // Scale lowBin and highBin to 0.0 to 1.0 equivalent range in spectrum
        float lowFrac = log2f((float)lowBin) / (float)spectrumBits;
        float fracRange = log2((float)highBin) / (float)spectrumBits - lowFrac;

        for (int column = 0; column < columnCount; column++) {
            // Start off bottom of graph
            columns[column].top = 6.0;
            columns[column].dot = 6.0;
            columns[column].velocity = 0.0;

            // Determine the lower and upper frequency range for this column, as
            // fractions within the scaled 0.0 to 1.0 spectrum range. 0.95 below
            // creates slight frequency overlap between columns, looks nicer.
            float lower = lowFrac + fracRange * ((float)column / float(columnCount) * 0.95);
            float upper = lowFrac + fracRange * ((float)(column + 1) / float(columnCount));

            // Center of lower-to-upper range
            float mid = (lower + upper) * 0.5;

            // 1/2 of lower-to-upper range
            float halfWidth = (upper - lower) * 0.5 + 1e-2;

            // Map fractions back to spectrum bin indices that contribute to column
            int firstBin = int(pow(2, (float)spectrumBits * lower) + 1e-4);
            int lastBin = int(pow(2, (float)spectrumBits * upper) + 1e-4);

            float totalWeight = 0.0; // Accumulate weight for this bin
            int numBins = lastBin - firstBin + 1;

            columns[column].firstBin = firstBin;
            columns[column].numBins = numBins;

            // Allocate space for bin weights for column.
            // If the allocation fails, guards are in place to make sure we don't crash.
            float* binWeights = new float[numBins]();
            columns[column].binWeights = binWeights;

            if (binWeights == nullptr) {
                continue;
            }
            
            for (int binIndex = firstBin; binIndex <= lastBin; binIndex++) {
                // Find distance from column's overall center to individual bin's
                // center, expressed as 0.0 (bin at center) to 1.0 (bin at limit of
                // lower-to-upper range).
                float binCenter = log2f((float)binIndex + 0.5) / (float)spectrumBits;
                float dist = fabs(binCenter - mid) / halfWidth;
                
                // Filter out a few math stragglers at either end
                if (dist < 1.0) { 
                    // Invert dist so 1.0 is at center
                    dist = 1.0 - dist;
                    // Bin weights have a cubic falloff curve within range:                    
                    float binWeight = (((3.0 - (dist * 2.0)) * dist) * dist);
                    binWeights[binIndex - firstBin] = binWeight;
                    totalWeight += binWeight;
                }
            }

            // Scale bin weights so total is 1.0 for each column, but then mute
            // lower columns slightly and boost higher columns. It graphs better.
            for (int i = 0; i < numBins; i++) {
                binWeights[i] = binWeights[i] / totalWeight * (0.6 + (float)i / float(columnCount) * 2.0);
            }
        }
    }

    ~ColumnSpectrumizer() {
        for (int column = 0; column < columnCount; column++) {
            delete [] columns[column].binWeights;
        }
    }

    void reset() {
        for (int column = 0; column < columnCount; column++) {
            // Start off bottom of graph
            columns[column].top = 6.0;
            columns[column].dot = 6.0;
            columns[column].velocity = 0.0;        
        }
    }

    void update(const int16_t* sampleBuffer, uint32_t dt) {
        int16_t fftBuffer[sampleCount];
        memcpy(fftBuffer, sampleBuffer, sampleCount * sizeof(fftBuffer[0]));

        // Perform the FFT operation on our buffer.
        ZeroFFT(fftBuffer, sampleCount);

        // Convert FFT output to spectrum. log(y) looks better than raw data.
        // Only lowBin to highBin elements are needed.
        uint32_t spectrumSize = sampleCount / 2;
        float spectrum[spectrumSize];

        for(int i = lowBin; i <= highBin; i++) {
            spectrum[i] = (fftBuffer[i] > 0) ? log((float)fftBuffer[i]) : 0.0;
        }

        // Find min & max range of spectrum bin values, with limits.
        float lower = spectrum[lowBin], upper = spectrum[lowBin];
        for (int i = lowBin + 1; i <= highBin; i++) {
            if (spectrum[i] < lower) {
                lower = spectrum[i];
            }

            if (spectrum[i] > upper) {
                upper = spectrum[i];
            }
        }
        if (upper < 2.5) { 
            upper = 2.5;
        }

        // Adjust dynamic level to current spectrum output, keeps the graph
        // 'lively' as ambient volume changes. Sparkle but don't saturate.
        if (upper > dynamicLevel) {
            // Got louder. Move level up quickly but allow initial "bump."
            dynamicLevel = dynamicLevel * 0.5 + upper * 0.5;
        } else {
            // Got quieter. Ease level down, else too many bumps.
            dynamicLevel = dynamicLevel * 0.75 + lower * 0.25;
        }

        // Apply vertical scale to spectrum data. Results may exceed
        // matrix height...that's OK, adds impact!
        float scale = 15.0 / (dynamicLevel - lower);
        for (int i = lowBin; i <= highBin; i++) {
            spectrum[i] = (spectrum[i] - lower) * scale;
        }

        // Set up each column.
        for(int column = 0; column < columnCount; column++) {
            float* binWeights = columns[column].binWeights;
            
            if (binWeights == nullptr) {
                columns[column].top = 0;
                columns[column].dot = column[columns].top - 0.5;
                columns[column].velocity = 0.0;
                continue;
            }

            int firstBin = columns[column].firstBin;

            // Start BELOW matrix and accumulate bin weights UP, saves math.
            float columnTop = 8.0;
            for (int binOffset = 0; binOffset < columns[column].numBins; binOffset++) {
                columnTop -= spectrum[firstBin + binOffset] * binWeights[binOffset];
            }

            // Column top positions are filtered to appear less 'twitchy' --
            // last data still has a 40% influence on current positions.
            columnTop = (columnTop * 0.6) +  (columns[column].top * 0.4);
            columns[column].top = columnTop;

            // Above current falling dot?
            if(columnTop < columns[column].dot) {
                // Move dot up
                columns[column].dot = columnTop - 0.5;

                // and clear out velocity
                columns[column].velocity = 0.0;
            } else {
                float fdt = dt * 0.001;

                // Move dot down and accelerate
                columns[column].dot += columns[column].velocity * fdt;
                columns[column].velocity += 15.0 * fdt;
            }
        }    
    }

    inline float getColumnTop(uint8_t column) const {
        if (column >= columnCount) {
            return 0;
        }

        return columns[column].top;
    }

    inline float getColumnDot(uint8_t column) const {
        if (column >= columnCount) {
            return 0;
        }

        return columns[column].dot;
    }    

private:
    struct Column {
        int firstBin;
        int numBins;
        float* binWeights;
        float top;
        float dot;
        float velocity;
    };

    Column columns[columnCount];
    float dynamicLevel = 10.0;

    const int32_t lowBin;
    const int32_t highBin;    
};