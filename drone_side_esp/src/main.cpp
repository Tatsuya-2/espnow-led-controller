#include <Arduino.h>
#include "esp_now_handler.h"
#include "led_controller.h"

// Global instances
EspNowHandler espNow;
LedController ledController;

// Statistics
unsigned long lastStatsTime = 0;
const unsigned long STATS_INTERVAL = 10000; // 10 seconds

// Callback for LED commands from ESP-NOW
void onLedCommand(const PatternConfig& config) {
    ledController.setPattern(config);
}

void printStats() {
    Serial.println("========================================");
    Serial.println("          XIAO ESP32S3 Status          ");
    Serial.println("========================================");
    Serial.printf("Uptime:         %lu seconds\n", millis() / 1000);
    Serial.printf("Free heap:      %u bytes\n", ESP.getFreeHeap());
    Serial.printf("Messages RX:    %u\n", espNow.getMessageCount());
    Serial.printf("Last message:   %lu ms ago\n", millis() - espNow.getLastMessageTime());
    Serial.printf("ESP-NOW status: %s\n", espNow.isConnected() ? "CONNECTED" : "DISCONNECTED");

    PatternConfig currentConfig = ledController.getCurrentConfig();
    Serial.printf("Current pattern: %s\n", patternToString(currentConfig.pattern));
    Serial.printf("LED color:      R:%d G:%d B:%d\n",
                  currentConfig.color.r, currentConfig.color.g, currentConfig.color.b);
    Serial.printf("Brightness:     %d\n", currentConfig.brightness);
    Serial.printf("Speed:          %d ms\n", currentConfig.speed);
    Serial.println("========================================\n");
}

void setup() {
    // Initialize serial
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("   DJI Drone LED Controller - XIAO    ");
    Serial.println("     ESP32S3 + ESP-NOW + WS2813       ");
    Serial.println("========================================\n");

    // Initialize LED controller
    ledController.begin();
    Serial.println("[MAIN] LED controller initialized");

    // Initialize ESP-NOW
    espNow.begin(onLedCommand);
    Serial.println("[MAIN] ESP-NOW handler initialized");

    Serial.println("[MAIN] System ready - waiting for commands...\n");

    // Print initial stats
    printStats();
}

void loop() {
    // Update LED pattern
    ledController.update();

    // Print stats periodically
    unsigned long now = millis();
    if (now - lastStatsTime >= STATS_INTERVAL) {
        lastStatsTime = now;
        printStats();
    }

    // Small delay to prevent watchdog timeout
    delay(1);
}
