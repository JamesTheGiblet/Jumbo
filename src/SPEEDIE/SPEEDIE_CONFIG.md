# ⚡ SPEEDIE Configuration Guide

## Bot Identity

- **Name**: SPEEDIE
- **Type**: High-Performance Scout
- **Focus**: Speed, Agility, Rapid Response

## Key Differences from WHEELIE

### Hardware

- **Sensors**: HC-SR04 Ultrasonic + MPU-6050 IMU (NO motion sensor for max speed)
- **LEDs**: 4 individual 2-wire LEDs (2 red + 2 green, common anode)
- **PWM Frequency**: 10kHz (vs 5kHz) for smoother motor control
- **No Buzzer**: Audio communication removed for maximum speed

### Performance Parameters

- **Base Motor Speed**: 240 (vs 200)
- **Turn Speed**: 180 (vs 150)  
- **Backup Duration**: 400ms (vs 600ms)
- **Evolution Interval**: 45s base (vs 60s)
- **Sleep Timeout**: 20s (vs 30s)

### Behavioral Differences

- **Immediate Activation**: No motion sensor - starts immediately for max speed
- **More Aggressive**: Higher confidence, faster reactions
- **Speed Tracking**: Records fastest obstacle clearance times
- **Tighter Strategy Matching**: 40mm tolerance (vs 50mm)
- **Progressive Acceleration**: Ramps up to max speed
- **Ultra-Fast Escape**: Half-time emergency maneuvers

### Evolution Adaptations

- **Faster Cycles**: 20s-2min range (vs 30s-3min)
- **More Parameters**: 12 mutable genes (vs 9)
- **Speed-Based Fitness**: Includes completion time bonus
- **Higher Mutation Rate**: 40% vocabulary mutation (vs 30%)

## LED Signaling Strategy

- **🔴 Red LEDs**: Negative emotions (obstacles, trapped, failure)
- **🟢 Green LEDs**: Positive emotions (success, clear path, confidence)
- **⚫ Both Off**: Neutral/sleeping
- **⚡ Fast Flash**: Activity/evolution/thinking

## Communication Style

- **LED-Only**: No audio delays for maximum speed
- **Binary States**: Simple red/green emotional mapping
- **High-Energy Patterns**: Fast flash sequences
- **Shorter Durations**: Half-time signal patterns

## Speed Optimizations

- **Minimal Delays**: Reduced pause times throughout
- **Quick Decisions**: 85% learned strategy usage (vs 80%)
- **Fast Scanning**: Half-time sensor delays
- **Efficient Memory**: Lower pruning thresholds (25 vs 20 strategies)
- **Rapid Evolution**: 15% evolution trigger (vs 10%)

## Sensor Fusion Strategy

1. **Primary**: HC-SR04 ultrasonic for distance
2. **Secondary**: MPU-6050 gyro for heading/orientation
3. **Fusion**: Combine for enhanced spatial awareness
4. **Error Handling**: 3 max consecutive errors (vs 5)

## Hardware Platform

### ESP32 Type C CH340 Development Board

- **Interface**: USB Type-C with CH340 TTL chip for stable serial communication
- **Core**: ESP-WROOM-32 dual-core 2.4GHz Wi-Fi + Bluetooth + BLE MCU
- **Technology**: 40nm low power technology with integrated antenna
- **Features**: Support for AP, STA, AP+STA coexist modes, LWIP protocol, FreeRTOS
- **Peripherals**: Capacitive touch, Hall sensors, low noise sensors
- **Download**: Automatic download without manual mode switching
- **Compatibility**: Windows system support (Cygwin, MSYS32)

## ⚡ SPEEDIE Wiring Pinout Guide

### 💡 LED Configuration (4 Individual 2-Wire LEDs)

```txt
Left Side LEDs:
  Pin 2  → Left RED LED (anode)
  Pin 4  → Left GREEN LED (anode)

Right Side LEDs:
  Pin 12 → Right RED LED (anode)
  Pin 14 → Right GREEN LED (anode)

LED Wiring (Common Anode):
  LED Cathode (-) → GND
  LED Anode (+)   → ESP32 Pin via 220Ω resistor
```

### 🚗 Motor Driver Connections

```txt
Left Motor:
  Pin 26 → Left Motor Driver IN1
  Pin 25 → Left Motor Driver IN2

Right Motor:
  Pin 32 → Right Motor Driver IN1
  Pin 33 → Right Motor Driver IN2

Motor Driver Power:
  VCC → 5V or 7.4V (battery voltage)
  GND → ESP32 GND
```

### 📡 HC-SR04 Ultrasonic Sensor

```txt
HC-SR04 Connections:
  Pin 18 → TRIG (Trigger)
  Pin 19 → ECHO (Echo)
  5V     → VCC
  GND    → GND

Distance Range: 2cm - 400cm
Operating Voltage: 5V
```

### 🧭 MPU-6050 IMU (I2C)

```txt
MPU-6050 Connections:
  Pin 21 (SDA) → SDA (Data)
  Pin 22 (SCL) → SCL (Clock)
  3.3V         → VCC
  GND          → GND

I2C Address: 0x68 (default)
Features: 3-axis gyro + 3-axis accelerometer
```

### 🔋 Power Distribution

```txt
ESP32 Power:
  USB Type-C → 5V input (programming/testing)
  VIN        → 7.4V (2S LiPo battery for mobile operation)
  3.3V       → Internal regulator output
  GND        → Common ground for all components

Battery Recommendations:
  2S LiPo (7.4V) 1000-2200mAh for mobile operation
  Include power switch for easy on/off
```

### 📋 Complete Pinout Summary

```txt
ESP32 Pin | Component        | Function
----------|------------------|------------------
2         | Left RED LED     | Emotional signaling
4         | Left GREEN LED   | Emotional signaling
12        | Right RED LED    | Emotional signaling
14        | Right GREEN LED  | Emotional signaling
18        | HC-SR04 TRIG     | Ultrasonic trigger
19        | HC-SR04 ECHO     | Ultrasonic echo
21        | MPU-6050 SDA     | I2C data
22        | MPU-6050 SCL     | I2C clock
25        | Left Motor IN2   | Motor control
26        | Left Motor IN1   | Motor control
32        | Right Motor IN1  | Motor control
33        | Right Motor IN2  | Motor control
GND       | All GND          | Common ground
3.3V      | MPU-6050 VCC     | Sensor power
5V        | HC-SR04 VCC      | Sensor power
VIN       | Battery +        | Main power input
```

### ⚠️ Important Wiring Notes

- **NO Motion Sensor**: SPEEDIE doesn't use pin 27 (motion sensor removed for speed)
- **PWM Frequency**: 10kHz for smooth motor control
- **LED Current**: Use 220Ω resistors to limit LED current
- **I2C Pullups**: ESP32 has internal pullups for I2C (SDA/SCL)
- **Power Isolation**: Consider isolating motor power from sensor power
- **Wire Gauge**: Use appropriate wire gauge for motor current (18-20 AWG recommended)

## Build Notes

- Ensure HC-SR04 is properly connected (Trig=18, Echo=19)
- MPU-6050 should be mounted stable for accurate gyro readings
- Use simple 2-wire common anode LEDs (not RGB or NeoPixel)
- Higher PWM frequency (10kHz) works well with ESP32 Type C board
- CH340 chip ensures reliable serial communication during development
- Test at lower speeds initially to verify sensor calibration
- ESP32's dual-core architecture allows for smooth sensor fusion

## Firmware Upload

```bash
# Build and upload SPEEDIE firmware
pio run -e SPEEDIE --target upload

# Monitor SPEEDIE serial output  
pio device monitor -e SPEEDIE
```

## Expected Behavior

- **Startup**: Fast boot sequence with red/green LED patterns
- **Movement**: Aggressive acceleration, high-speed navigation
- **Obstacles**: Lightning-fast reactions, efficient scanning
- **Evolution**: Rapid adaptation cycles, speed-focused fitness
- **Communication**: High-energy LED signals, no audio delays
- **Trapping**: Ultra-fast escape sequences with power maneuvers
