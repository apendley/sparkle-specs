#pragma once

#include <Arduino.h>
#include "hsv2rgb.h"

namespace Color {
    uint8_t sine8(uint8_t theta);
    uint8_t gamma8(uint8_t x);

    inline uint8_t scale8(uint8_t c, uint8_t brightness) {
        return ((uint16_t)c * brightness) / 255;        
    }
}

namespace Color {
    struct RGB {
        inline RGB() : r(0), g(0), b(0) { }
        inline RGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) { }
        inline RGB(uint32_t packedColor) : r(packedColor >> 16), g(packedColor >> 8), b(packedColor) { }

        static RGB gray(uint8_t brightness) {
            return RGB(brightness, brightness, brightness);
        }

        inline bool operator==(const RGB& other) const {
            return r == other.r && g == other.g && b == other.b;
        }

        inline bool isBlack() const {
            return (r == 0) && (g == 0) & (b == 0);
        }

        inline uint32_t packed() const {
            return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
        }

        inline uint16_t packed565() const {
            return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        inline void scale(uint8_t brightness) {
            r = scale8(r, brightness);
            g = scale8(g, brightness);
            b = scale8(b, brightness);
        }

        inline RGB scaled(uint8_t brightness) const {
            return RGB(scale8(r, brightness), scale8(g, brightness), scale8(b, brightness));
        }

        inline RGB gammaApplied() const {
            return RGB(gamma8(r), gamma8(g), gamma8(b));
        }

        static inline RGB fromHexString(const char* colorString) {
            if (colorString == nullptr || strlen(colorString) < 7) { 
                return RGB();
            }

            char colorBuffer[3] = {0};
            colorBuffer[2] = 0;

            // skip the #                    
            uint8_t i = 1;

            // First two bytes are red
            colorBuffer[0] = colorString[i++];
            colorBuffer[1] = colorString[i++];
            uint8_t red = strtol(colorBuffer, NULL, 16);

            // Next two bytes are green
            colorBuffer[0] = colorString[i++];
            colorBuffer[1] = colorString[i++];
            uint8_t green = strtol(colorBuffer, NULL, 16);

            // Next two bytes are blue
            colorBuffer[0] = colorString[i++];
            colorBuffer[1] = colorString[i++];
            uint8_t blue = strtol(colorBuffer, NULL, 16);                

            return RGB(red, green, blue);
        }           

        uint8_t r;
        uint8_t g;
        uint8_t b;
    } __attribute__((packed));

    struct HSV {
        inline HSV(uint16_t h, uint8_t s = 255, uint8_t v = 255) 
            : h(h), s(s), v(v)
        {
        }

        HSV withValue(uint8_t value) {
            return HSV(h, s, value);
        }

        static inline HSV fromRGB(uint8_t r, uint8_t g, uint8_t b) {
            uint8_t rangeMin = min(min(r, g), b);
            uint8_t rangeMax = max(max(r, g), b);
            
            uint8_t v = rangeMax;            
            if (v == 0) {
                return HSV(0, 0, 0);
            }

            uint8_t s = 255 * long(rangeMax - rangeMin) / v;
            if (s == 0) {
                return HSV(0, 0, v);
            }

            uint16_t h;            
        
            if (rangeMax == r) {
                h = 0 + 11008 * (g - b) / (rangeMax - rangeMin);
            }
            else if (rangeMax == g) {
                h = 21760 + 11008 * (b - r) / (rangeMax - rangeMin);
            }
            else {
                h = 43776 + 11008 * (r - g) / (rangeMax - rangeMin);
            }
        
            return HSV(h, s, v);
        }

        static inline HSV fromRGB(RGB rgb) {
            return fromRGB(rgb.r, rgb.g, rgb.b);
        }

        inline RGB toRGB() const {
            uint8_t r, g, b;
            hsv2rgb(h, s, v, r, g, b);
            return RGB(r, g, b);
        }

        uint16_t h;
        uint8_t s;
        uint8_t v;
    } __attribute__((packed));

    // # Splits rainbow into (palette_size - 2) indices, 
    // # and assigns black to index 0, and white to index 1, and the
    // color spectrum spread across the remainder.
    inline RGB withIndex(uint16_t index, uint16_t paletteSize, uint8_t brightness) {
        paletteSize = max(paletteSize, 2);
        index = min(index, paletteSize);

        // 0 is black
        if (index == 0) {
            return RGB();
        }
        // 1 is white/gray
        else if (index == 1) {
            return RGB(brightness, brightness, brightness);
        }
        // The color spectrum is spread from 2 to (paletteSize - 2)
        else {
            uint16_t hue = int((index - 2) * (65536 / (paletteSize - 2)));
            return HSV(hue, 255, brightness).toRGB();
        }            
    } 
}
