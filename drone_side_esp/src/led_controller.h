#pragma once

#include <FastLED.h>
#include "patterns.h"

// LED Configuration
#define LED_PIN 2           // XIAO ESP32S3 GPIO2 for data line
#define NUM_LEDS 30         // Default 30 LEDs (adjustable for 60 LED/m)
#define LED_TYPE WS2813     // WS2813 LED strip with signal line redundancy
#define COLOR_ORDER GRB     // Color order for WS2813

class LedController {
public:
    LedController() : currentConfig(PatternDefaults::getDefault(LedPattern::IDLE)),
                      cycleStart(0), currentStep(0) {
        FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
        FastLED.setBrightness(PatternDefaults::DEFAULT_BRIGHTNESS);
        FastLED.clear();
        FastLED.show();
    }

    void begin() {
        Serial.println("[LED] Controller initialized");
        setPattern(LedPattern::IDLE);
    }

    void setPattern(LedPattern pattern) {
        setPattern(PatternDefaults::getDefault(pattern));
    }

    void setPattern(const PatternConfig& config) {
        currentConfig = config;
        FastLED.setBrightness(config.brightness);
        cycleStart = millis();
        currentStep = 0;
        Serial.printf("[LED] Pattern set: %s, Brightness: %d, Speed: %d ms\n",
                      patternToString(config.pattern), config.brightness, config.speed);
    }

    void update() {
        unsigned long now = millis();

        switch (currentConfig.pattern) {
            case LedPattern::IDLE:
                updateStatic();
                break;
            case LedPattern::TAKING_OFF:
                updateFlowUp(now);
                break;
            case LedPattern::HOVERING:
                updateBlink(now);
                break;
            case LedPattern::FLYING:
                updateBlink(now);
                break;
            case LedPattern::LANDING:
                updateFlowDown(now);
                break;
            case LedPattern::EMERGENCY:
                updateBlink(now);
                break;
            case LedPattern::LOW_BATTERY:
                updateBlink(now);
                break;
        }

        FastLED.show();
    }

    PatternConfig getCurrentConfig() const {
        return currentConfig;
    }

private:
    CRGB leds[NUM_LEDS];
    PatternConfig currentConfig;
    unsigned long cycleStart;
    uint8_t currentStep;

    void updateStatic() {
        fill_solid(leds, NUM_LEDS, currentConfig.color);
    }

    void updateBlink(unsigned long now) {
        unsigned long elapsed = now - cycleStart;

        if (elapsed >= currentConfig.speed) {
            cycleStart = now;
            currentStep = !currentStep;
        }

        if (currentStep) {
            fill_solid(leds, NUM_LEDS, currentConfig.color);
        } else {
            FastLED.clear();
        }
    }

    void updateFlowUp(unsigned long now) {
        unsigned long elapsed = now - cycleStart;

        // Calculate steps for smooth flow
        uint8_t stepsPerCycle = NUM_LEDS + 10; // Extra steps for gap
        uint16_t stepDuration = currentConfig.speed / stepsPerCycle;

        if (elapsed >= stepDuration) {
            cycleStart = now;
            currentStep = (currentStep + 1) % stepsPerCycle;
        }

        // Clear all LEDs
        FastLED.clear();

        // Draw flowing pattern (bottom to top)
        uint8_t tailLength = 10;
        for (uint8_t i = 0; i < tailLength; i++) {
            int ledIndex = currentStep - i;
            if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
                uint8_t brightness = 255 * (tailLength - i) / tailLength;
                leds[ledIndex] = currentConfig.color;
                leds[ledIndex].nscale8(brightness);
            }
        }
    }

    void updateFlowDown(unsigned long now) {
        unsigned long elapsed = now - cycleStart;

        // Calculate steps for smooth flow
        uint8_t stepsPerCycle = NUM_LEDS + 10;
        uint16_t stepDuration = currentConfig.speed / stepsPerCycle;

        if (elapsed >= stepDuration) {
            cycleStart = now;
            currentStep = (currentStep + 1) % stepsPerCycle;
        }

        // Clear all LEDs
        FastLED.clear();

        // Draw flowing pattern (top to bottom)
        uint8_t tailLength = 10;
        for (uint8_t i = 0; i < tailLength; i++) {
            int ledIndex = (NUM_LEDS - 1) - (currentStep - i);
            if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
                uint8_t brightness = 255 * (tailLength - i) / tailLength;
                leds[ledIndex] = currentConfig.color;
                leds[ledIndex].nscale8(brightness);
            }
        }
    }
};
