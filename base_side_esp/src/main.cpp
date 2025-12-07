#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// Configuration
#define ESPNOW_CHANNEL 1
#define MAX_MESSAGE_SIZE 250
#define SERIAL_BUFFER_SIZE 512

// Drone ESP32 MAC address (must be configured)
// Format: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
uint8_t droneMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // Placeholder - MUST BE UPDATED
bool peerRegistered = false;

// Statistics
uint32_t messagesSent = 0;
uint32_t sendErrors = 0;
unsigned long lastStatsTime = 0;
const unsigned long STATS_INTERVAL = 10000; // 10 seconds

// Serial buffer for incoming JSON commands
String serialBuffer = "";

// ESP-NOW send callback
void onDataSent(const uint8_t* mac, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        Serial.println("[ESP-NOW] Message sent successfully");
        messagesSent++;
    } else {
        Serial.println("[ESP-NOW] Message send failed");
        sendErrors++;
    }
}

bool initEspNow() {
    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Print MAC address
    Serial.print("[ESP-NOW] Base MAC Address: ");
    Serial.println(WiFi.macAddress());

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("[ESP-NOW] Initialization failed!");
        return false;
    }

    Serial.println("[ESP-NOW] Initialization successful");

    // Register send callback
    esp_now_register_send_cb(onDataSent);

    return true;
}

bool registerDronePeer() {
    // Check if MAC address is still placeholder
    bool isPlaceholder = true;
    for (int i = 0; i < 6; i++) {
        if (droneMacAddress[i] != 0xFF) {
            isPlaceholder = false;
            break;
        }
    }

    if (isPlaceholder) {
        Serial.println("[ESP-NOW] WARNING: Drone side esp32MAC address not configured!");
        Serial.println("[ESP-NOW] Please update droneMacAddress[] in main.cpp");
        Serial.println("[ESP-NOW] Messages will not be sent until configured.");
        return false;
    }

    // Register peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, droneMacAddress, 6);
    peerInfo.channel = ESPNOW_CHANNEL;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("[ESP-NOW] Failed to add peer");
        return false;
    }

    Serial.printf("[ESP-NOW] Drone peer registered: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  droneMacAddress[0], droneMacAddress[1], droneMacAddress[2],
                  droneMacAddress[3], droneMacAddress[4], droneMacAddress[5]);
    peerRegistered = true;
    return true;
}

void sendLedCommand(const String& jsonCommand) {
    if (!peerRegistered) {
        Serial.println("[ESP-NOW] Cannot send: peer not registered");
        return;
    }

    // Validate JSON
    StaticJsonDocument<MAX_MESSAGE_SIZE> doc;
    DeserializationError error = deserializeJson(doc, jsonCommand);

    if (error) {
        Serial.printf("[ERROR] Invalid JSON: %s\n", error.c_str());
        return;
    }

    // Validate message structure
    if (!doc.containsKey("type") || !doc.containsKey("data")) {
        Serial.println("[ERROR] Missing required fields (type, data)");
        return;
    }

    // Serialize and send
    char buffer[MAX_MESSAGE_SIZE];
    size_t len = serializeJson(doc, buffer, sizeof(buffer));

    if (len == 0) {
        Serial.println("[ERROR] Failed to serialize JSON");
        return;
    }

    // Send via ESP-NOW
    esp_err_t result = esp_now_send(droneMacAddress, (uint8_t*)buffer, len);

    if (result == ESP_OK) {
        Serial.printf("[ESP-NOW] Sending command (%d bytes): %s\n", len, buffer);
    } else {
        Serial.println("[ESP-NOW] Send error");
        sendErrors++;
    }
}

void processSerialCommand(const String& command) {
    // Trim whitespace
    String trimmed = command;
    trimmed.trim();

    if (trimmed.length() == 0) {
        return;
    }

    Serial.printf("[SERIAL] Received: %s\n", trimmed.c_str());

    // Check for special commands
    if (trimmed.startsWith("MAC:")) {
        // Update MAC address command: MAC:AA:BB:CC:DD:EE:FF
        String macStr = trimmed.substring(4);
        int values[6];
        if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x",
                   &values[0], &values[1], &values[2],
                   &values[3], &values[4], &values[5]) == 6) {
            for (int i = 0; i < 6; i++) {
                droneMacAddress[i] = (uint8_t)values[i];
            }
            Serial.println("[CONFIG] Drone MAC address updated");
            registerDronePeer();
        } else {
            Serial.println("[ERROR] Invalid MAC format. Use: MAC:AA:BB:CC:DD:EE:FF");
        }
        return;
    }

    if (trimmed == "STATUS") {
        // Print status
        Serial.println("========================================");
        Serial.println("        Base ESP32 Status              ");
        Serial.println("========================================");
        Serial.printf("Uptime:         %lu seconds\n", millis() / 1000);
        Serial.printf("Free heap:      %u bytes\n", ESP.getFreeHeap());
        Serial.printf("Messages sent:  %u\n", messagesSent);
        Serial.printf("Send errors:    %u\n", sendErrors);
        Serial.printf("Peer status:    %s\n", peerRegistered ? "REGISTERED" : "NOT REGISTERED");
        Serial.printf("Drone MAC:      %02X:%02X:%02X:%02X:%02X:%02X\n",
                      droneMacAddress[0], droneMacAddress[1], droneMacAddress[2],
                      droneMacAddress[3], droneMacAddress[4], droneMacAddress[5]);
        Serial.println("========================================\n");
        return;
    }

    // Otherwise, treat as JSON LED command
    sendLedCommand(trimmed);
}

void printHelp() {
    Serial.println("\n========================================");
    Serial.println("   DJI Drone LED Controller - Base    ");
    Serial.println("      ESP32 + ESP-NOW Transmitter     ");
    Serial.println("========================================\n");
    Serial.println("Commands:");
    Serial.println("  MAC:AA:BB:CC:DD:EE:FF - Set drone MAC address");
    Serial.println("  STATUS - Print system status");
    Serial.println("  {JSON} - Send LED command (see below)\n");
    Serial.println("LED Command Format:");
    Serial.println("{");
    Serial.println("  \"type\": \"led_command\",");
    Serial.println("  \"data\": {");
    Serial.println("    \"pattern\": \"FLYING\",");
    Serial.println("    \"color\": [255, 255, 255],");
    Serial.println("    \"brightness\": 128,");
    Serial.println("    \"speed\": 100");
    Serial.println("  },");
    Serial.println("  \"timestamp\": 1699564800000");
    Serial.println("}\n");
    Serial.println("Patterns: IDLE, TAKING_OFF, HOVERING, FLYING,");
    Serial.println("          LANDING, EMERGENCY, LOW_BATTERY");
    Serial.println("========================================\n");
}

void setup() {
    // Initialize serial
    Serial.begin(115200);
    delay(1000);

    printHelp();

    // Initialize ESP-NOW
    if (!initEspNow()) {
        Serial.println("[FATAL] ESP-NOW initialization failed!");
        return;
    }

    // Try to register drone peer
    registerDronePeer();

    Serial.println("[MAIN] System ready - waiting for commands...\n");
}

void loop() {
    // Read serial input
    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\n' || c == '\r') {
            if (serialBuffer.length() > 0) {
                processSerialCommand(serialBuffer);
                serialBuffer = "";
            }
        } else {
            serialBuffer += c;

            // Prevent buffer overflow
            if (serialBuffer.length() >= SERIAL_BUFFER_SIZE) {
                Serial.println("[ERROR] Serial buffer overflow - command too long");
                serialBuffer = "";
            }
        }
    }

    // Print periodic stats
    unsigned long now = millis();
    if (now - lastStatsTime >= STATS_INTERVAL) {
        lastStatsTime = now;
        Serial.printf("[STATS] Uptime: %lu s, Sent: %u, Errors: %u\n",
                      now / 1000, messagesSent, sendErrors);
    }

    delay(1);
}
