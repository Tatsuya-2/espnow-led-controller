/**
 * @file test_json_parsing.cpp
 * @brief Unit tests for JSON message parsing logic
 *
 * Tests written FIRST following TDD methodology:
 * 1. Define expected JSON message formats
 * 2. Test parsing of valid and invalid messages
 * 3. Verify error handling for malformed input
 *
 * Note: These tests verify JSON parsing logic that would be used by ESP-NOW handler.
 * Full integration tests require hardware and are performed separately.
 */

#include <Arduino.h>
#include <unity.h>
#include <ArduinoJson.h>
#include "patterns.h"

// Helper function to parse LED command from JSON (extracted logic from EspNowHandler)
bool parseLedCommand(const char* jsonStr, PatternConfig& outConfig) {
    StaticJsonDocument<250> doc;
    DeserializationError error = deserializeJson(doc, jsonStr);

    if (error) {
        return false;
    }

    // Validate message type
    const char* type = doc["type"];
    if (!type || strcmp(type, "led_command") != 0) {
        return false;
    }

    // Parse command data
    JsonObject dataObj = doc["data"];
    if (!dataObj) {
        return false;
    }

    // Extract pattern
    const char* patternStr = dataObj["pattern"];
    if (!patternStr) {
        return false;
    }

    LedPattern pattern = stringToPattern(patternStr);

    // Get default config and override with received values
    outConfig = PatternDefaults::getDefault(pattern);

    // Override color if provided
    if (dataObj.containsKey("color")) {
        JsonArray colorArray = dataObj["color"];
        if (colorArray.size() >= 3) {
            outConfig.color = CRGB(
                colorArray[0].as<uint8_t>(),
                colorArray[1].as<uint8_t>(),
                colorArray[2].as<uint8_t>()
            );
        }
    }

    // Override brightness if provided
    if (dataObj.containsKey("brightness")) {
        outConfig.brightness = dataObj["brightness"].as<uint8_t>();
    }

    // Override speed if provided
    if (dataObj.containsKey("speed")) {
        outConfig.speed = dataObj["speed"].as<uint16_t>();
    }

    return true;
}

// Test valid minimal LED command (pattern only)
void test_parse_valid_minimal_command() {
    const char* json = R"({"type":"led_command","data":{"pattern":"FLYING"},"timestamp":1699564800000})";
    PatternConfig config;

    bool success = parseLedCommand(json, config);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(LedPattern::FLYING, config.pattern);
    // Should use default values
    TEST_ASSERT_EQUAL(255, config.color.r);
    TEST_ASSERT_EQUAL(255, config.color.g);
    TEST_ASSERT_EQUAL(255, config.color.b);
    TEST_ASSERT_EQUAL(128, config.brightness);
    TEST_ASSERT_EQUAL(200, config.speed);
}

// Test valid full LED command (all fields)
void test_parse_valid_full_command() {
    const char* json = R"({"type":"led_command","data":{"pattern":"EMERGENCY","color":[255,0,0],"brightness":255,"speed":100},"timestamp":1699564800000})";
    PatternConfig config;

    bool success = parseLedCommand(json, config);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(LedPattern::EMERGENCY, config.pattern);
    TEST_ASSERT_EQUAL(255, config.color.r);
    TEST_ASSERT_EQUAL(0, config.color.g);
    TEST_ASSERT_EQUAL(0, config.color.b);
    TEST_ASSERT_EQUAL(255, config.brightness);
    TEST_ASSERT_EQUAL(100, config.speed);
}

// Test command with custom color overriding default
void test_parse_command_custom_color() {
    const char* json = R"({"type":"led_command","data":{"pattern":"IDLE","color":[128,64,32]},"timestamp":1699564800000})";
    PatternConfig config;

    bool success = parseLedCommand(json, config);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(LedPattern::IDLE, config.pattern);
    TEST_ASSERT_EQUAL(128, config.color.r);
    TEST_ASSERT_EQUAL(64, config.color.g);
    TEST_ASSERT_EQUAL(32, config.color.b);
}

// Test command with partial color array (should handle gracefully)
void test_parse_command_partial_color() {
    const char* json = R"({"type":"led_command","data":{"pattern":"IDLE","color":[255,128]},"timestamp":1699564800000})";
    PatternConfig config;

    bool success = parseLedCommand(json, config);

    // Should fail gracefully or use defaults (current implementation uses defaults)
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(LedPattern::IDLE, config.pattern);
    // Color should remain default since array is incomplete
}

// Test all pattern types
void test_parse_all_patterns() {
    const char* patterns[] = {
        "IDLE", "TAKING_OFF", "HOVERING", "FLYING",
        "LANDING", "EMERGENCY", "LOW_BATTERY"
    };

    for (const char* patternName : patterns) {
        char json[200];
        snprintf(json, sizeof(json),
                 R"({"type":"led_command","data":{"pattern":"%s"},"timestamp":1699564800000})",
                 patternName);

        PatternConfig config;
        bool success = parseLedCommand(json, config);

        TEST_ASSERT_TRUE_MESSAGE(success, patternName);
    }
}

// Test invalid JSON syntax
void test_parse_invalid_json_syntax() {
    const char* json = R"({"type":"led_command","data":{"pattern":"FLYING"}})"; // missing closing brace
    PatternConfig config;

    // Current JSON is actually valid, let's use truly invalid JSON
    const char* invalidJson = R"({"type":"led_command","data":{"pattern":"FLYING")";
    bool success = parseLedCommand(invalidJson, config);

    TEST_ASSERT_FALSE(success);
}

// Test missing type field
void test_parse_missing_type() {
    const char* json = R"({"data":{"pattern":"FLYING"},"timestamp":1699564800000})";
    PatternConfig config;

    bool success = parseLedCommand(json, config);

    TEST_ASSERT_FALSE(success);
}

// Test wrong type field
void test_parse_wrong_type() {
    const char* json = R"({"type":"other_command","data":{"pattern":"FLYING"},"timestamp":1699564800000})";
    PatternConfig config;

    bool success = parseLedCommand(json, config);

    TEST_ASSERT_FALSE(success);
}

// Test missing data field
void test_parse_missing_data() {
    const char* json = R"({"type":"led_command","timestamp":1699564800000})";
    PatternConfig config;

    bool success = parseLedCommand(json, config);

    TEST_ASSERT_FALSE(success);
}

// Test missing pattern field
void test_parse_missing_pattern() {
    const char* json = R"({"type":"led_command","data":{"brightness":128},"timestamp":1699564800000})";
    PatternConfig config;

    bool success = parseLedCommand(json, config);

    TEST_ASSERT_FALSE(success);
}

// Test empty pattern string
void test_parse_empty_pattern() {
    const char* json = R"({"type":"led_command","data":{"pattern":""},"timestamp":1699564800000})";
    PatternConfig config;

    bool success = parseLedCommand(json, config);

    TEST_ASSERT_TRUE(success);
    // Empty pattern should default to IDLE
    TEST_ASSERT_EQUAL(LedPattern::IDLE, config.pattern);
}

// Test invalid pattern name (should default to IDLE)
void test_parse_invalid_pattern_name() {
    const char* json = R"({"type":"led_command","data":{"pattern":"INVALID_PATTERN"},"timestamp":1699564800000})";
    PatternConfig config;

    bool success = parseLedCommand(json, config);

    TEST_ASSERT_TRUE(success);
    // Invalid pattern should default to IDLE
    TEST_ASSERT_EQUAL(LedPattern::IDLE, config.pattern);
}

// Test brightness boundary values
void test_parse_brightness_boundaries() {
    // Brightness = 0
    const char* json1 = R"({"type":"led_command","data":{"pattern":"IDLE","brightness":0},"timestamp":1699564800000})";
    PatternConfig config1;
    TEST_ASSERT_TRUE(parseLedCommand(json1, config1));
    TEST_ASSERT_EQUAL(0, config1.brightness);

    // Brightness = 255
    const char* json2 = R"({"type":"led_command","data":{"pattern":"IDLE","brightness":255},"timestamp":1699564800000})";
    PatternConfig config2;
    TEST_ASSERT_TRUE(parseLedCommand(json2, config2));
    TEST_ASSERT_EQUAL(255, config2.brightness);
}

// Test speed boundary values
void test_parse_speed_boundaries() {
    // Speed = 0 (static)
    const char* json1 = R"({"type":"led_command","data":{"pattern":"IDLE","speed":0},"timestamp":1699564800000})";
    PatternConfig config1;
    TEST_ASSERT_TRUE(parseLedCommand(json1, config1));
    TEST_ASSERT_EQUAL(0, config1.speed);

    // Speed = 65535 (max uint16_t)
    const char* json2 = R"({"type":"led_command","data":{"pattern":"IDLE","speed":65535},"timestamp":1699564800000})";
    PatternConfig config2;
    TEST_ASSERT_TRUE(parseLedCommand(json2, config2));
    TEST_ASSERT_EQUAL(65535, config2.speed);
}

// Test color boundary values
void test_parse_color_boundaries() {
    // All zeros (black)
    const char* json1 = R"({"type":"led_command","data":{"pattern":"IDLE","color":[0,0,0]},"timestamp":1699564800000})";
    PatternConfig config1;
    TEST_ASSERT_TRUE(parseLedCommand(json1, config1));
    TEST_ASSERT_EQUAL(0, config1.color.r);
    TEST_ASSERT_EQUAL(0, config1.color.g);
    TEST_ASSERT_EQUAL(0, config1.color.b);

    // All max (white)
    const char* json2 = R"({"type":"led_command","data":{"pattern":"IDLE","color":[255,255,255]},"timestamp":1699564800000})";
    PatternConfig config2;
    TEST_ASSERT_TRUE(parseLedCommand(json2, config2));
    TEST_ASSERT_EQUAL(255, config2.color.r);
    TEST_ASSERT_EQUAL(255, config2.color.g);
    TEST_ASSERT_EQUAL(255, config2.color.b);
}

// Test very long JSON (near buffer limit)
void test_parse_long_json() {
    // ArduinoJson buffer is 250 bytes
    char json[300];
    snprintf(json, sizeof(json),
             R"({"type":"led_command","data":{"pattern":"FLYING","color":[255,255,255],"brightness":128,"speed":200,"extra_field_1":"padding","extra_field_2":"more_padding"},"timestamp":1699564800000})");

    PatternConfig config;
    // This might fail due to buffer size, which is expected
    parseLedCommand(json, config);
    // Just verify it doesn't crash
}

// Test null input
void test_parse_null_input() {
    PatternConfig config;
    bool success = parseLedCommand(nullptr, config);

    TEST_ASSERT_FALSE(success);
}

// Test empty string
void test_parse_empty_string() {
    PatternConfig config;
    bool success = parseLedCommand("", config);

    TEST_ASSERT_FALSE(success);
}

void setup() {
    delay(2000); // Wait for serial monitor

    UNITY_BEGIN();

    // Valid command tests
    RUN_TEST(test_parse_valid_minimal_command);
    RUN_TEST(test_parse_valid_full_command);
    RUN_TEST(test_parse_command_custom_color);
    RUN_TEST(test_parse_command_partial_color);
    RUN_TEST(test_parse_all_patterns);

    // Invalid JSON tests
    RUN_TEST(test_parse_invalid_json_syntax);
    RUN_TEST(test_parse_missing_type);
    RUN_TEST(test_parse_wrong_type);
    RUN_TEST(test_parse_missing_data);
    RUN_TEST(test_parse_missing_pattern);
    RUN_TEST(test_parse_empty_pattern);
    RUN_TEST(test_parse_invalid_pattern_name);

    // Boundary value tests
    RUN_TEST(test_parse_brightness_boundaries);
    RUN_TEST(test_parse_speed_boundaries);
    RUN_TEST(test_parse_color_boundaries);

    // Edge case tests
    RUN_TEST(test_parse_long_json);
    RUN_TEST(test_parse_null_input);
    RUN_TEST(test_parse_empty_string);

    UNITY_END();
}

void loop() {
    // Tests run once in setup()
}
