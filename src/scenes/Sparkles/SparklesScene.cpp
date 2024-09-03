#include "SparklesScene.h"

// #define LOGGER Serial
#include "Logger.h"

SparklesScene::SparklesScene(Device& d)
    : Scene(d) 
{
}

void SparklesScene::enter() {
    Glasses& glasses = getDevice().glasses;

    glasses.fill(0);
    glasses.left_ring.fill(0);
    glasses.right_ring.fill(0);

    Settings& settings = getDevice().settings;
    useCustomColor = settings.sparklesUseCustomColor();
    hue = settings.sparklesHue();

    intensityInvScale = intensityMinInvScale;

    newSparkleTimer = 0;

    getDevice().pdmRecorder.startRecording();
}

void SparklesScene::update(uint32_t dt) {
    Gamepad& gamepad = getDevice().gamepad;
    Settings& settings = getDevice().settings;

    // Use the same filtering trick as the bars scene, and apply 30% 
    // of the last value to the current one to help prevent twitchiness.
    float reading = max(getDevice().pdmRecorder.magnitude() - intensityDeadZone, 0);
    float magnitude = (reading * 0.7) + (lastMagnitude * 0.3);
    lastMagnitude = magnitude;

    float intensity = intensityMin + (magnitude / intensityInvScale) * (intensityMax - intensityMin);
    intensity = min(intensity, intensityMax);

    // speed up spawn time depending on intensity.
    uint32_t dtScaled = dt + (dt * intensity * 10) + (dt * intensity * intensity * intensity * 10);
    newSparkleTimer -= dtScaled;

    // Speed up color change based on intensity.
    currentHue += dt + (intensity * intensity * 300);
    if (currentHue >= 65536.0) {
        currentHue -= 65536.0;
    }

    if (gamepad.isDown(gamepad.buttonC)) {
        useCustomColor = true;
        hue += dt * 10;

        if (hue >= 65535.0) {
            hue = 0.0;
        }
    }
    else if (gamepad.wasReleased(gamepad.buttonC)) {
        settings.sparklesSetUseCustomColor(useCustomColor);        
        settings.sparklesSetHue(hue);
    } 
    else if (gamepad.wasPressed(gamepad.buttonZ)) {
        useCustomColor = false;
        settings.sparklesSetUseCustomColor(useCustomColor);
    }    

    if (newSparkleTimer <= 0) {
        newSparkleTimer = random(minSpawnInterval, maxSpawnInterval);

        // Brighten sparkle based on intensity.
        uint16_t brightness = minBrightness + (intensity * intensity * (maxBrightness - minBrightness));

        // The higher the intensity, the more sparkles spawn.
        int spawnCount = random(2, 4) + (intensity * intensity * 4);

        for (int i = 0; i < spawnCount; i++) {
            uint16_t h = useCustomColor ? hue : currentHue;
            Color::HSV hsv(h, random(128, 255), brightness);
            int16_t ttl = random(minTimeToLive, maxTimeToLive) + int(intensity * intensity * minTimeToLive);
            newSparkle(hsv, ttl);
        }
    }

    for (int i = 0; i < maxSparkles; i++) {
        Sparkle& sparkle = sparkles[i];

        if (!sparkle.isActive) {
            continue;
        }

        sparkle.timeToLive -= dt;

        if (sparkle.timeToLive <= 0) {
            sparkle.timeToLive = 0;
            sparkle.isActive = false;
        }
    }

    #if defined(LOGGER)
    uint32_t numActive = 0;
    for (int i = 0; i < maxSparkles; i++) {
        if (sparkles[i].isActive) {
            numActive++;
        }
    }
    LOGFMT("sparkles: %d, magnitude: %.02f, invScale: %.02f, intensity: %.02f, dt: %d, dtScaled: %d\n", numActive, magnitude, intensityInvScale, intensity, dt, dtScaled);
    #endif

    draw();    

    if (magnitude > intensityInvScale) {
        // Increase the intensity scale
        intensityInvScale += dt * (magnitude - intensityInvScale) * 0.02;
    } 
    else if (magnitude < intensityInvScale) {
        // Decrease the intensity scale
        intensityInvScale -= dt * (intensityInvScale - magnitude) * 0.002;
        intensityInvScale = max(intensityInvScale, intensityMinInvScale);
    }
}

void SparklesScene::draw() {
    Glasses& glasses = getDevice().glasses;

    glasses.fill(0);

    for (int i = 0; i < maxSparkles; i++) {
        Sparkle& sparkle = sparkles[i];

        if (sparkle.isActive) {
            uint8_t scale = map(sparkle.timeToLive, 0, sparkle.duration, 32, 255);
            Color::RGB c = sparkle.hsv.toRGB().scaled(scale);
            glasses.drawPixel(sparkle.x, sparkle.y, c.gammaApplied().packed565());            

        }
    }

    glasses.show();
}

void SparklesScene::exit() {
    getDevice().pdmRecorder.stopRecording();
}

void SparklesScene::newSparkle(const Color::HSV& hsv, int16_t ttl) {
    Glasses& glasses = getDevice().glasses;

    int16_t shortestTTL = maxTimeToLive + 1;
    uint8_t newSparkleIndex = 0;

    for (uint8_t i = 0; i < maxSparkles; i++) {
        Sparkle& sparkle = sparkles[i];

        if (sparkle.isActive) {
            // Find the active particle with the sortest time to live, 
            // we'll use it if there are no inactive sparkles left.
            if (sparkle.timeToLive < shortestTTL) {
                shortestTTL = sparkle.timeToLive;
                newSparkleIndex = i;
            }            
        } else {
            // We found a sparkle we can use, let's use it.
            newSparkleIndex = i;
            break;            
        }
    }

    Sparkle& newSparkle = sparkles[newSparkleIndex];
        newSparkle.isActive = true;
        newSparkle.duration = ttl;
        newSparkle.timeToLive = newSparkle.duration;
        newSparkle.x = random(glasses.width());
        newSparkle.y = random(glasses.height());
        newSparkle.hsv = hsv;
}

void SparklesScene::receivedColor(const Color::RGB& c) {
    Settings& settings = getDevice().settings;

    if (c.isBlack()) {
        useCustomColor = false;
    } 
    else {
        useCustomColor = true;
        Color::HSV hsv = Color::HSV::fromRGB(c);
        hue = hsv.h;

        settings.audioBarsSetHue(hsv.h);
        settings.audioBarsSetSaturation(hsv.s);
    }

    settings.audioBarsSetUseCustomColor(useCustomColor);    
}