#pragma once

#include <Arduino.h>

// LED Pattern Types
enum class LedPattern {
    IDLE,           // Static blue
    TAKING_OFF,     // Bottom-to-top flow green
    HOVERING,       // Slow blink green
    FLYING,         // Fast blink white
    LANDING,        // Top-to-bottom flow yellow
    EMERGENCY,      // Fast blink red
    LOW_BATTERY,    // Slow blink orange
    BRAINWAVE       // BCI control: flowing blue-purple-pink gradient (brainwave visualization)
};

// Pattern Configuration
struct PatternConfig {
    LedPattern pattern;
    CRGB color;
    uint8_t brightness;
    uint16_t speed;  // milliseconds per cycle
};

// Default pattern configurations
namespace PatternDefaults {
    constexpr uint8_t DEFAULT_BRIGHTNESS = 128;

    // Colors
    constexpr CRGB COLOR_BLUE = CRGB(0, 0, 255);
    constexpr CRGB COLOR_GREEN = CRGB(0, 255, 0);
    constexpr CRGB COLOR_WHITE = CRGB(255, 255, 255);
    constexpr CRGB COLOR_YELLOW = CRGB(255, 255, 0);
    constexpr CRGB COLOR_RED = CRGB(255, 0, 0);
    constexpr CRGB COLOR_ORANGE = CRGB(255, 165, 0);
    constexpr CRGB COLOR_CYAN_BLUE = CRGB(0, 100, 255);  // Brainwave base color

    // Pattern speeds (ms per cycle)
    constexpr uint16_t SPEED_STATIC = 0;
    constexpr uint16_t SPEED_SLOW_BLINK = 1000;
    constexpr uint16_t SPEED_FAST_BLINK = 200;
    constexpr uint16_t SPEED_FLOW = 100;
    constexpr uint16_t SPEED_BRAINWAVE = 50;  // Fast flowing for brainwave effect

    // Get default config for a pattern
    inline PatternConfig getDefault(LedPattern pattern) {
        switch (pattern) {
            case LedPattern::IDLE:
                return {pattern, COLOR_BLUE, DEFAULT_BRIGHTNESS, SPEED_STATIC};
            case LedPattern::TAKING_OFF:
                return {pattern, COLOR_GREEN, DEFAULT_BRIGHTNESS, SPEED_FLOW};
            case LedPattern::HOVERING:
                return {pattern, COLOR_GREEN, DEFAULT_BRIGHTNESS, SPEED_SLOW_BLINK};
            case LedPattern::FLYING:
                return {pattern, COLOR_WHITE, DEFAULT_BRIGHTNESS, SPEED_FAST_BLINK};
            case LedPattern::LANDING:
                return {pattern, COLOR_YELLOW, DEFAULT_BRIGHTNESS, SPEED_FLOW};
            case LedPattern::EMERGENCY:
                return {pattern, COLOR_RED, DEFAULT_BRIGHTNESS, SPEED_FAST_BLINK};
            case LedPattern::LOW_BATTERY:
                return {pattern, COLOR_ORANGE, DEFAULT_BRIGHTNESS, SPEED_SLOW_BLINK};
            case LedPattern::BRAINWAVE:
                return {pattern, COLOR_CYAN_BLUE, 180, SPEED_BRAINWAVE};  // Brighter for BCI visibility
            default:
                return {LedPattern::IDLE, COLOR_BLUE, DEFAULT_BRIGHTNESS, SPEED_STATIC};
        }
    }
}

// Convert string to LedPattern
inline LedPattern stringToPattern(const char* str) {
    if (strcmp(str, "IDLE") == 0) return LedPattern::IDLE;
    if (strcmp(str, "TAKING_OFF") == 0) return LedPattern::TAKING_OFF;
    if (strcmp(str, "HOVERING") == 0) return LedPattern::HOVERING;
    if (strcmp(str, "FLYING") == 0) return LedPattern::FLYING;
    if (strcmp(str, "LANDING") == 0) return LedPattern::LANDING;
    if (strcmp(str, "EMERGENCY") == 0) return LedPattern::EMERGENCY;
    if (strcmp(str, "LOW_BATTERY") == 0) return LedPattern::LOW_BATTERY;
    if (strcmp(str, "BRAINWAVE") == 0) return LedPattern::BRAINWAVE;
    return LedPattern::IDLE;
}

// Convert LedPattern to string
inline const char* patternToString(LedPattern pattern) {
    switch (pattern) {
        case LedPattern::IDLE: return "IDLE";
        case LedPattern::TAKING_OFF: return "TAKING_OFF";
        case LedPattern::HOVERING: return "HOVERING";
        case LedPattern::FLYING: return "FLYING";
        case LedPattern::LANDING: return "LANDING";
        case LedPattern::EMERGENCY: return "EMERGENCY";
        case LedPattern::LOW_BATTERY: return "LOW_BATTERY";
        case LedPattern::BRAINWAVE: return "BRAINWAVE";
        default: return "UNKNOWN";
    }
}
