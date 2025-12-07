# ESP32 LED Control System

ESP-NOW based LED control system for DJI drone visual feedback.

## Overview

This directory contains firmware for two ESP32 microcontrollers:
- **base_side_esp**: Jetson-side ESP32 (transmitter via ESP-NOW)
- **drone_side_esp**: Drone-side XIAO ESP32S3 (receiver + LED controller)

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         Jetson Orin Nano                        │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │ ROS2: led_controller_bridge_node                         │   │
│  │ - Subscribes: /flight_planner/state, /drone_status       │   │
│  │ - Sends: LED commands (JSON via serial)                  │   │
│  └──────────────────┬───────────────────────────────────────┘   │
│                     │ USB Serial (115200 baud)                  │
│  ┌──────────────────▼───────────────────────────────────────┐   │
│  │ ESP32 (base_side_esp)                                    │   │
│  │ - Receives: JSON commands via serial                     │   │
│  │ - Transmits: ESP-NOW to drone                            │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ ESP-NOW (2.4GHz)
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     Drone (DJI Mavic Mini 4 Pro)                │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │ XIAO ESP32S3 (drone_side_esp)                            │   │
│  │ - Receives: ESP-NOW commands                             │   │
│  │ - Controls: WS2813 LED strip (GPIO2)                     │   │
│  │ - Patterns: 7 flight states                              │   │
│  └──────────────────┬───────────────────────────────────────┘   │
│                     │ Data line                                 │
│  ┌──────────────────▼───────────────────────────────────────┐   │
│  │ WS2813 LED Strip (30-60 LEDs, 5V)                        │   │
│  │ - Power: 1S LiPo + 5V boost converter                    │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

## Hardware Requirements

### Base Side (Jetson)
- ESP32 development board (e.g., ESP32-DevKitC)
- USB cable (Jetson ↔ ESP32)
- Power: USB 5V

### Drone Side
- Seeed Studio XIAO ESP32S3
- WS2813 LED strip (5V, 30-60 LED/m)
  - Signal line redundancy (more reliable than WS2812B)
- 1S LiPo battery (500mAh recommended)
- 5V boost converter (e.g., MT3608)
- Wiring:
  - LED Data: GPIO2
  - LED 5V: From boost converter
  - LED GND: Common ground
- **Total weight**: < 50g (critical for Mini 4 Pro payload)

## Setup

### 1. Install PlatformIO

```bash
# Install PlatformIO CLI
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -o get-platformio.py
python3 get-platformio.py

# Or install via pip
pip install platformio
```

### 2. Build and Upload Firmware

#### Drone Side (XIAO ESP32S3)

```bash
cd esp32/drone_side_esp

# Build
pio run -e seeed_xiao_esp32s3

# Upload
pio run -e seeed_xiao_esp32s3 -t upload

# Monitor serial output
pio device monitor -e seeed_xiao_esp32s3
```

**Important**: Note the MAC address printed at startup:
```
[ESP-NOW] MAC Address: AA:BB:CC:DD:EE:FF
```

#### Base Side (Jetson ESP32)

```bash
cd esp32/base_side_esp

# Build
pio run -e esp32dev

# Upload
pio run -e esp32dev -t upload

# Monitor serial output
pio device monitor -e esp32dev
```

### 3. Configure Drone MAC Address

#### Method 1: Via Configuration File (Recommended)

Edit `jetson_prod/config/drone/dji-drone.conf`:
```bash
LED_DRONE_MAC_ADDRESS=AA:BB:CC:DD:EE:FF  # Replace with actual MAC from step 2
```

#### Method 2: Via Serial Console

Connect to base ESP32:
```bash
screen /dev/ttyUSB0 115200
```

Send command:
```
MAC:AA:BB:CC:DD:EE:FF
```

### 4. Test Communication

#### Manual Test (Base ESP32 Serial)

Connect to base ESP32 and send test command:
```json
{"type":"led_command","data":{"pattern":"FLYING","brightness":128},"timestamp":1699564800000}
```

Expected response on drone ESP32:
```
[ESP-NOW] Received 82 bytes from XX:XX:XX:XX:XX:XX
[ESP-NOW] Command: FLYING, RGB: [255,255,255], Brightness: 128, Speed: 200
[LED] Pattern set: FLYING, Brightness: 128, Speed: 200 ms
```

#### ROS2 Integration Test

```bash
# Publish flight state
ros2 topic pub /flight_planner/state std_msgs/String \
  "{data: '{\"state\": \"FLYING\"}'}" --once

# Check LED controller logs
tail -f /tmp/led_controller.log
```

## LED Patterns

| Pattern | Color | Behavior | Trigger |
|---------|-------|----------|---------|
| **IDLE** | Blue | Static | Flight state: IDLE |
| **TAKING_OFF** | Green | Bottom→Top flow | Flight state: TAKING_OFF |
| **HOVERING** | Green | Slow blink (1s) | Flight state: HOVERING |
| **FLYING** | White | Fast blink (200ms) | Flight state: FLYING |
| **LANDING** | Yellow | Top→Bottom flow | Flight state: LANDING |
| **EMERGENCY** | Red | Fast blink (200ms) | Flight state: EMERGENCY |
| **LOW_BATTERY** | Orange | Slow blink (1s) | Battery < 20% |

## ESP-NOW Message Format

```json
{
  "type": "led_command",
  "data": {
    "pattern": "FLYING",
    "color": [255, 255, 255],
    "brightness": 128,
    "speed": 100
  },
  "timestamp": 1699564800000
}
```

**Fields:**
- `type`: Always "led_command"
- `data.pattern`: One of IDLE, TAKING_OFF, HOVERING, FLYING, LANDING, EMERGENCY, LOW_BATTERY
- `data.color`: Optional RGB array [R, G, B] (0-255), overrides default
- `data.brightness`: Optional brightness (0-255), default 128
- `data.speed`: Optional speed in milliseconds per cycle
- `timestamp`: Unix timestamp in milliseconds

## Troubleshooting

### ESP-NOW Communication Issues

**Symptom**: Drone ESP32 not receiving messages

1. **Verify MAC address**:
   - Check drone ESP32 serial output for actual MAC
   - Update `LED_DRONE_MAC_ADDRESS` in config
   - Restart ROS2 node or send `MAC:` command

2. **Check WiFi channel**:
   - Both ESP32s must be on same channel (default: 1)
   - Verify in both firmware `ESPNOW_CHANNEL` constant

3. **Check power**:
   - Ensure both ESP32s are powered on
   - Drone ESP32 battery level sufficient

4. **Monitor logs**:
   ```bash
   # Drone side
   pio device monitor -e seeed_xiao_esp32s3

   # Base side
   pio device monitor -e esp32dev
   ```

### LED Not Working

**Symptom**: ESP-NOW working but LEDs not lighting

1. **Check connections**:
   - Data: GPIO2 → LED Data In
   - Power: 5V, GND properly connected
   - Verify polarity

2. **Check power supply**:
   - WS2813 requires stable 5V
   - Current: ~60mA per LED at full brightness
   - 30 LEDs = up to 1.8A at max

3. **Test with pattern command**:
   ```json
   {"type":"led_command","data":{"pattern":"IDLE","brightness":255},"timestamp":1699564800000}
   ```

4. **Monitor serial**:
   ```
   [LED] Controller initialized
   [LED] Pattern set: IDLE, Brightness: 255, Speed: 0 ms
   ```

### Serial Connection Issues (Base ESP32)

**Symptom**: ROS2 node can't connect to base ESP32

1. **Check permissions**:
   ```bash
   sudo usermod -a -G dialout $USER
   # Log out and log back in
   ```

2. **List available ports**:
   ```bash
   ls /dev/ttyUSB*
   ls /dev/ttyACM*
   ```

3. **Update config** if port is different:
   ```bash
   LED_SERIAL_PORT=/dev/ttyACM0  # or detected port
   ```

4. **Enable auto-detect**:
   ```bash
   LED_AUTO_DETECT_PORT=true
   ```

## Development

### Adding New Patterns

1. Edit `drone_side_esp/src/patterns.h`:
   - Add new enum value to `LedPattern`
   - Add default config in `PatternDefaults::getDefault()`
   - Add string conversion in `stringToPattern()` and `patternToString()`

2. Edit `drone_side_esp/src/led_controller.h`:
   - Add case in `LedController::update()` switch statement
   - Implement pattern update function (e.g., `updateNewPattern()`)

3. Rebuild and upload:
   ```bash
   pio run -e seeed_xiao_esp32s3 -t upload
   ```

### Testing

#### ESP32 Unit Tests

The drone side ESP32 includes comprehensive unit tests following TDD methodology:

**Test Files:**
- `test/test_patterns.cpp` - Pattern conversion and default configuration tests
- `test/test_json_parsing.cpp` - JSON message parsing and validation tests

**Run tests:**
```bash
cd drone_side_esp

# Install PlatformIO if not already installed
pip install platformio

# Run all tests
pio test -e seeed_xiao_esp32s3

# Run with verbose output
pio test -e seeed_xiao_esp32s3 -v
```

**Test Coverage:**
- ✓ Pattern string ↔ enum conversion (7 patterns)
- ✓ Default configurations for all patterns
- ✓ JSON parsing (valid/invalid messages)
- ✓ Input validation (brightness, color, speed)
- ✓ Edge cases (null input, malformed JSON)

#### ROS2 Unit Tests

The LED controller bridge includes 35 comprehensive unit tests:

**Test File:**
- `isaac_ros_ws/src/led_controller_bridge/test/test_led_controller_node.py`

**Run tests:**
```bash
# Run all tests
./jetson_prod/scripts/run-system-tests.sh --unit

# Run LED controller tests only
cd isaac_ros_ws
source /opt/ros/humble/setup.bash
source install/setup.bash
python3 -m pytest src/led_controller_bridge/test/ -v

# Run with specific filter
python3 -m pytest src/led_controller_bridge/test/ -k "battery" -v
```

**Test Coverage:**
- ✓ State transitions (IDLE → FLYING → LANDING, etc.)
- ✓ Battery monitoring (normal → low battery alert)
- ✓ LED command generation and serialization
- ✓ Rate limiting (command cooldown)
- ✓ Error handling (serial failures, invalid JSON)
- ✓ Input validation (color, brightness, speed ranges)

#### Manual Pattern Test

Connect to base ESP32 serial and send test commands:
```bash
screen /dev/ttyUSB0 115200

# Test FLYING pattern
{"type":"led_command","data":{"pattern":"FLYING"},"timestamp":1699564800000}

# Test custom color
{"type":"led_command","data":{"pattern":"IDLE","color":[128,64,255],"brightness":200},"timestamp":1699564800000}
```

## Power Consumption

- XIAO ESP32S3: ~80mA (WiFi active)
- WS2813 LEDs:
  - Per LED: ~60mA max (all white, full brightness)
  - 30 LEDs: ~1.8A max
  - Typical (128 brightness, half on): ~700mA
- Total: ~800mA typical, 2A max
- Runtime on 500mAh LiPo: ~30-40 minutes

## Weight Budget

| Component | Weight |
|-----------|--------|
| XIAO ESP32S3 | 5g |
| WS2813 strip (30 LEDs, 50cm) | 15g |
| 1S LiPo 500mAh | 15g |
| 5V boost converter | 3g |
| Wiring, heat shrink | 5g |
| **Total** | **43g** |

> DJI Mavic Mini 4 Pro max payload: ~249g total weight. Keep LED system < 50g.

## References

- [FastLED Library](https://github.com/FastLED/FastLED)
- [ESP-NOW Protocol](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [WS2813 Datasheet](https://www.led-color.com/upload/201809/WS2813%20LED%20Datasheet.pdf)
- [Seeed XIAO ESP32S3](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)
