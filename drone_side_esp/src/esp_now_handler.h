#pragma once

#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "patterns.h"

// ESP-NOW Configuration
#define ESPNOW_CHANNEL 1
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define MAX_MESSAGE_SIZE 250

// Callback function type
typedef void (*LedCommandCallback)(const PatternConfig& config);

class EspNowHandler {
public:
    EspNowHandler() : commandCallback(nullptr), lastMessageTime(0), messageCount(0) {}

    void begin(LedCommandCallback callback) {
        commandCallback = callback;

        // Initialize WiFi in station mode
        WiFi.mode(ESPNOW_WIFI_MODE);
        WiFi.disconnect();

        // Print MAC address
        Serial.print("[ESP-NOW] MAC Address: ");
        Serial.println(WiFi.macAddress());

        // Initialize ESP-NOW
        if (esp_now_init() != ESP_OK) {
            Serial.println("[ESP-NOW] Initialization failed!");
            return;
        }

        Serial.println("[ESP-NOW] Initialization successful");

        // Register receive callback
        esp_now_register_recv_cb(onDataRecv);

        // Set static instance for callback
        instance = this;
    }

    uint32_t getMessageCount() const {
        return messageCount;
    }

    unsigned long getLastMessageTime() const {
        return lastMessageTime;
    }

    bool isConnected() const {
        // Consider connected if we received a message in the last 5 seconds
        return (millis() - lastMessageTime) < 5000;
    }

private:
    static EspNowHandler* instance;
    LedCommandCallback commandCallback;
    unsigned long lastMessageTime;
    uint32_t messageCount;

    // ESP-NOW receive callback (must be static)
    static void onDataRecv(const uint8_t* mac, const uint8_t* data, int len) {
        if (instance) {
            instance->handleReceivedData(mac, data, len);
        }
    }

    void handleReceivedData(const uint8_t* mac, const uint8_t* data, int len) {
        lastMessageTime = millis();
        messageCount++;

        // Log received message
        Serial.printf("[ESP-NOW] Received %d bytes from %02X:%02X:%02X:%02X:%02X:%02X\n",
                      len, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        // Parse JSON
        StaticJsonDocument<MAX_MESSAGE_SIZE> doc;
        DeserializationError error = deserializeJson(doc, data, len);

        if (error) {
            Serial.printf("[ESP-NOW] JSON parse error: %s\n", error.c_str());
            return;
        }

        // Validate message type
        const char* type = doc["type"];
        if (!type || strcmp(type, "led_command") != 0) {
            Serial.println("[ESP-NOW] Invalid message type");
            return;
        }

        // Parse command data
        JsonObject dataObj = doc["data"];
        if (!dataObj) {
            Serial.println("[ESP-NOW] Missing data object");
            return;
        }

        // Extract pattern
        const char* patternStr = dataObj["pattern"];
        if (!patternStr) {
            Serial.println("[ESP-NOW] Missing pattern field");
            return;
        }

        LedPattern pattern = stringToPattern(patternStr);

        // Get default config and override with received values
        PatternConfig config = PatternDefaults::getDefault(pattern);

        // Override color if provided
        if (dataObj.containsKey("color")) {
            JsonArray colorArray = dataObj["color"];
            if (colorArray.size() >= 3) {
                config.color = CRGB(
                    colorArray[0].as<uint8_t>(),
                    colorArray[1].as<uint8_t>(),
                    colorArray[2].as<uint8_t>()
                );
            }
        }

        // Override brightness if provided
        if (dataObj.containsKey("brightness")) {
            config.brightness = dataObj["brightness"].as<uint8_t>();
        }

        // Override speed if provided
        if (dataObj.containsKey("speed")) {
            config.speed = dataObj["speed"].as<uint16_t>();
        }

        // Log parsed command
        uint64_t timestamp = doc["timestamp"].as<uint64_t>();
        Serial.printf("[ESP-NOW] Command: %s, RGB: [%d,%d,%d], Brightness: %d, Speed: %d, Timestamp: %llu\n",
                      patternStr, config.color.r, config.color.g, config.color.b,
                      config.brightness, config.speed, timestamp);

        // Execute callback
        if (commandCallback) {
            commandCallback(config);
        }
    }
};

// Initialize static instance
EspNowHandler* EspNowHandler::instance = nullptr;
