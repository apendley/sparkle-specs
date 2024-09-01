#pragma once

#include <Arduino.h>
#include "Color.h"

class GlassesBuffer {
public:
	static constexpr uint8_t ringPixelCount = 24;
	static constexpr uint16_t matrixWidth = 18;
	static constexpr uint16_t matrixHeight = 5;
	static constexpr uint16_t matrixPixelCount = matrixWidth * matrixHeight;

    void fillMatrix(const Color::RGB& c) {
		for (uint32_t i = 0; i < matrixPixelCount; i++) {
            matrix[i] = c;
		}        
    }

    void fillLeftRing(const Color::RGB& c) {
        for (uint32_t i = 0; i < ringPixelCount; i++) {
            leftRing[i] = c;
		}
    }

    void fillRightRing(const Color::RGB& c) {
		for (uint32_t i = 0; i < ringPixelCount; i++) {
            rightRing[i] = c;
		}
    }

    void fillRings(const Color::RGB& c) {
		for (uint32_t i = 0; i < ringPixelCount; i++) {
            leftRing[i] = c;
            rightRing[i] = c;
		}        
    }

	void fill(const Color::RGB& c) {
        fillRings(c);
		fillMatrix(c);
	}

	// Left ring pixels

	Color::RGB getLeftRingColor(uint32_t index) {
		if (index >= ringPixelCount) {
			return Color::RGB();
		}

        return leftRing[index];
	}

	void setLeftRingColor(uint32_t index, const Color::RGB& c) {
		if (index >= ringPixelCount) {
			return;
		}

        leftRing[index] = c;
	}

	// Right ring pixels

	Color::RGB getRightRingColor(uint32_t index) {
		if (index >= ringPixelCount) {
			return Color::RGB();
		}

    	return rightRing[index];
	}

	void setRightRingColor(uint32_t index, const Color::RGB& c) {
		if (index >= ringPixelCount) {
			return;
		}

        rightRing[index] = c;
	}

	// Matrix pixels

	Color::RGB getMatrixColor(uint32_t index) {
		if (index >= matrixPixelCount) {
			return Color::RGB();
		}

		return matrix[index];
	}

	Color::RGB getMatrixColor(uint32_t x, uint32_t y) {
        uint32_t index = (y * matrixWidth) + x;
        return getMatrixColor(index);
	}    

	void setMatrixColor(uint32_t index, const Color::RGB& c) {
		if (index >= matrixPixelCount) {
			return;
		}

        matrix[index] = c;
	}

	void setMatrixColor(uint32_t x, uint32_t y, const Color::RGB& c) {
        uint32_t index = (y * matrixWidth) + x;
        setMatrixColor(index, c);
	}    

	// Fading

    void fade(uint8_t scale) {
        fadeLeftRing(scale);
        fadeRightRing(scale);
        fadeMatrix(scale);
    }

	void fadeLeftRing(uint8_t scale) {
		fadeBuffer(leftRing, ringPixelCount, scale);
	}

	void fadeRightRing(uint8_t scale) {
		fadeBuffer(rightRing, ringPixelCount, scale);
	}	

	void fadeMatrix(uint8_t scale) {
	    fadeBuffer(matrix, matrixPixelCount, scale);
	}

private:
	void fadeBuffer(Color::RGB* buffer, uint32_t numPixels, uint8_t scale) {
	    for(uint32_t i = 0; i < numPixels; i++) { 
            buffer[i].scale(scale);
	    }    		
	}

private:
    Color::RGB leftRing[ringPixelCount];
    Color::RGB rightRing[ringPixelCount];
    Color::RGB matrix[matrixPixelCount];
};
