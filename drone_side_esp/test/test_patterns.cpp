/**
 * @file test_patterns.cpp
 * @brief Unit tests for LED pattern definitions and conversions
 *
 * Tests written FIRST following TDD methodology:
 * 1. Define expected behavior through tests
 * 2. Verify implementation matches specifications
 * 3. Include edge cases and error handling
 */

#include <Arduino.h>
#include <unity.h>
#include "patterns.h"

// Test stringToPattern with valid pattern names
void test_stringToPattern_valid_names() {
    TEST_ASSERT_EQUAL(LedPattern::IDLE, stringToPattern("IDLE"));
    TEST_ASSERT_EQUAL(LedPattern::TAKING_OFF, stringToPattern("TAKING_OFF"));
    TEST_ASSERT_EQUAL(LedPattern::HOVERING, stringToPattern("HOVERING"));
    TEST_ASSERT_EQUAL(LedPattern::FLYING, stringToPattern("FLYING"));
    TEST_ASSERT_EQUAL(LedPattern::LANDING, stringToPattern("LANDING"));
    TEST_ASSERT_EQUAL(LedPattern::EMERGENCY, stringToPattern("EMERGENCY"));
    TEST_ASSERT_EQUAL(LedPattern::LOW_BATTERY, stringToPattern("LOW_BATTERY"));
}

// Test stringToPattern with invalid names (should default to IDLE)
void test_stringToPattern_invalid_names() {
    TEST_ASSERT_EQUAL(LedPattern::IDLE, stringToPattern(""));
    TEST_ASSERT_EQUAL(LedPattern::IDLE, stringToPattern("INVALID"));
    TEST_ASSERT_EQUAL(LedPattern::IDLE, stringToPattern("idle")); // case sensitive
    TEST_ASSERT_EQUAL(LedPattern::IDLE, stringToPattern("Flying")); // case sensitive
    TEST_ASSERT_EQUAL(LedPattern::IDLE, stringToPattern(nullptr));
}

// Test stringToPattern with edge cases
void test_stringToPattern_edge_cases() {
    TEST_ASSERT_EQUAL(LedPattern::IDLE, stringToPattern("IDLE "));  // trailing space
    TEST_ASSERT_EQUAL(LedPattern::IDLE, stringToPattern(" IDLE"));  // leading space
    TEST_ASSERT_EQUAL(LedPattern::IDLE, stringToPattern("ID"));     // partial match
    TEST_ASSERT_EQUAL(LedPattern::IDLE, stringToPattern("IDLEMORE")); // extra chars
}

// Test patternToString for all valid patterns
void test_patternToString_all_patterns() {
    TEST_ASSERT_EQUAL_STRING("IDLE", patternToString(LedPattern::IDLE));
    TEST_ASSERT_EQUAL_STRING("TAKING_OFF", patternToString(LedPattern::TAKING_OFF));
    TEST_ASSERT_EQUAL_STRING("HOVERING", patternToString(LedPattern::HOVERING));
    TEST_ASSERT_EQUAL_STRING("FLYING", patternToString(LedPattern::FLYING));
    TEST_ASSERT_EQUAL_STRING("LANDING", patternToString(LedPattern::LANDING));
    TEST_ASSERT_EQUAL_STRING("EMERGENCY", patternToString(LedPattern::EMERGENCY));
    TEST_ASSERT_EQUAL_STRING("LOW_BATTERY", patternToString(LedPattern::LOW_BATTERY));
}

// Test roundtrip conversion (pattern -> string -> pattern)
void test_pattern_conversion_roundtrip() {
    LedPattern patterns[] = {
        LedPattern::IDLE,
        LedPattern::TAKING_OFF,
        LedPattern::HOVERING,
        LedPattern::FLYING,
        LedPattern::LANDING,
        LedPattern::EMERGENCY,
        LedPattern::LOW_BATTERY
    };

    for (LedPattern pattern : patterns) {
        const char* str = patternToString(pattern);
        LedPattern converted = stringToPattern(str);
        TEST_ASSERT_EQUAL(pattern, converted);
    }
}

// Test default configurations for IDLE pattern
void test_pattern_defaults_idle() {
    PatternConfig config = PatternDefaults::getDefault(LedPattern::IDLE);

    TEST_ASSERT_EQUAL(LedPattern::IDLE, config.pattern);
    TEST_ASSERT_EQUAL(0, config.color.r);
    TEST_ASSERT_EQUAL(0, config.color.g);
    TEST_ASSERT_EQUAL(255, config.color.b);  // Blue
    TEST_ASSERT_EQUAL(128, config.brightness);
    TEST_ASSERT_EQUAL(0, config.speed);  // Static
}

// Test default configurations for TAKING_OFF pattern
void test_pattern_defaults_taking_off() {
    PatternConfig config = PatternDefaults::getDefault(LedPattern::TAKING_OFF);

    TEST_ASSERT_EQUAL(LedPattern::TAKING_OFF, config.pattern);
    TEST_ASSERT_EQUAL(0, config.color.r);
    TEST_ASSERT_EQUAL(255, config.color.g);  // Green
    TEST_ASSERT_EQUAL(0, config.color.b);
    TEST_ASSERT_EQUAL(128, config.brightness);
    TEST_ASSERT_EQUAL(100, config.speed);  // Flow speed
}

// Test default configurations for HOVERING pattern
void test_pattern_defaults_hovering() {
    PatternConfig config = PatternDefaults::getDefault(LedPattern::HOVERING);

    TEST_ASSERT_EQUAL(LedPattern::HOVERING, config.pattern);
    TEST_ASSERT_EQUAL(0, config.color.r);
    TEST_ASSERT_EQUAL(255, config.color.g);  // Green
    TEST_ASSERT_EQUAL(0, config.color.b);
    TEST_ASSERT_EQUAL(128, config.brightness);
    TEST_ASSERT_EQUAL(1000, config.speed);  // Slow blink
}

// Test default configurations for FLYING pattern
void test_pattern_defaults_flying() {
    PatternConfig config = PatternDefaults::getDefault(LedPattern::FLYING);

    TEST_ASSERT_EQUAL(LedPattern::FLYING, config.pattern);
    TEST_ASSERT_EQUAL(255, config.color.r);  // White
    TEST_ASSERT_EQUAL(255, config.color.g);
    TEST_ASSERT_EQUAL(255, config.color.b);
    TEST_ASSERT_EQUAL(128, config.brightness);
    TEST_ASSERT_EQUAL(200, config.speed);  // Fast blink
}

// Test default configurations for LANDING pattern
void test_pattern_defaults_landing() {
    PatternConfig config = PatternDefaults::getDefault(LedPattern::LANDING);

    TEST_ASSERT_EQUAL(LedPattern::LANDING, config.pattern);
    TEST_ASSERT_EQUAL(255, config.color.r);  // Yellow
    TEST_ASSERT_EQUAL(255, config.color.g);
    TEST_ASSERT_EQUAL(0, config.color.b);
    TEST_ASSERT_EQUAL(128, config.brightness);
    TEST_ASSERT_EQUAL(100, config.speed);  // Flow speed
}

// Test default configurations for EMERGENCY pattern
void test_pattern_defaults_emergency() {
    PatternConfig config = PatternDefaults::getDefault(LedPattern::EMERGENCY);

    TEST_ASSERT_EQUAL(LedPattern::EMERGENCY, config.pattern);
    TEST_ASSERT_EQUAL(255, config.color.r);  // Red
    TEST_ASSERT_EQUAL(0, config.color.g);
    TEST_ASSERT_EQUAL(0, config.color.b);
    TEST_ASSERT_EQUAL(128, config.brightness);
    TEST_ASSERT_EQUAL(200, config.speed);  // Fast blink
}

// Test default configurations for LOW_BATTERY pattern
void test_pattern_defaults_low_battery() {
    PatternConfig config = PatternDefaults::getDefault(LedPattern::LOW_BATTERY);

    TEST_ASSERT_EQUAL(LedPattern::LOW_BATTERY, config.pattern);
    TEST_ASSERT_EQUAL(255, config.color.r);  // Orange
    TEST_ASSERT_EQUAL(165, config.color.g);
    TEST_ASSERT_EQUAL(0, config.color.b);
    TEST_ASSERT_EQUAL(128, config.brightness);
    TEST_ASSERT_EQUAL(1000, config.speed);  // Slow blink
}

// Test that all patterns have valid default configurations
void test_all_patterns_have_defaults() {
    LedPattern patterns[] = {
        LedPattern::IDLE,
        LedPattern::TAKING_OFF,
        LedPattern::HOVERING,
        LedPattern::FLYING,
        LedPattern::LANDING,
        LedPattern::EMERGENCY,
        LedPattern::LOW_BATTERY
    };

    for (LedPattern pattern : patterns) {
        PatternConfig config = PatternDefaults::getDefault(pattern);
        TEST_ASSERT_EQUAL(pattern, config.pattern);
        TEST_ASSERT_GREATER_OR_EQUAL(0, config.brightness);
        TEST_ASSERT_LESS_OR_EQUAL(255, config.brightness);
        TEST_ASSERT_GREATER_OR_EQUAL(0, config.speed);
    }
}

// Test brightness is within valid range
void test_default_brightness_in_range() {
    TEST_ASSERT_GREATER_OR_EQUAL(0, PatternDefaults::DEFAULT_BRIGHTNESS);
    TEST_ASSERT_LESS_OR_EQUAL(255, PatternDefaults::DEFAULT_BRIGHTNESS);
    TEST_ASSERT_EQUAL(128, PatternDefaults::DEFAULT_BRIGHTNESS);
}

// Test color definitions are valid RGB values
void test_color_definitions() {
    // Each color component should be 0-255
    CRGB colors[] = {
        PatternDefaults::COLOR_BLUE,
        PatternDefaults::COLOR_GREEN,
        PatternDefaults::COLOR_WHITE,
        PatternDefaults::COLOR_YELLOW,
        PatternDefaults::COLOR_RED,
        PatternDefaults::COLOR_ORANGE
    };

    for (CRGB color : colors) {
        TEST_ASSERT_LESS_OR_EQUAL(255, color.r);
        TEST_ASSERT_LESS_OR_EQUAL(255, color.g);
        TEST_ASSERT_LESS_OR_EQUAL(255, color.b);
    }
}

void setup() {
    delay(2000); // Wait for serial monitor

    UNITY_BEGIN();

    // String to Pattern conversion tests
    RUN_TEST(test_stringToPattern_valid_names);
    RUN_TEST(test_stringToPattern_invalid_names);
    RUN_TEST(test_stringToPattern_edge_cases);

    // Pattern to String conversion tests
    RUN_TEST(test_patternToString_all_patterns);

    // Roundtrip conversion tests
    RUN_TEST(test_pattern_conversion_roundtrip);

    // Default configuration tests
    RUN_TEST(test_pattern_defaults_idle);
    RUN_TEST(test_pattern_defaults_taking_off);
    RUN_TEST(test_pattern_defaults_hovering);
    RUN_TEST(test_pattern_defaults_flying);
    RUN_TEST(test_pattern_defaults_landing);
    RUN_TEST(test_pattern_defaults_emergency);
    RUN_TEST(test_pattern_defaults_low_battery);
    RUN_TEST(test_all_patterns_have_defaults);

    // Value range tests
    RUN_TEST(test_default_brightness_in_range);
    RUN_TEST(test_color_definitions);

    UNITY_END();
}

void loop() {
    // Tests run once in setup()
}
