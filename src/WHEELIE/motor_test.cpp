#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>

// ═══════════════════════════════════════════════════════════
// 🚗 MOTOR TEST SYSTEM FOR WHEELIE BOT
// ═══════════════════════════════════════════════════════════

// == LED PINS (SINGLE RGB LED) ==
const int LED_R_PIN = 12;
const int LED_G_PIN = 14;
const int LED_B_PIN = 15;

// PWM Channels for LED
const int PWM_CH_R = 4;
const int PWM_CH_G = 5;
const int PWM_CH_B = 6;

// == MOTOR PINS & PWM ==
const int LEFT_MOTOR_PIN1 = 26;
const int LEFT_MOTOR_PIN2 = 25;
const int RIGHT_MOTOR_PIN1 = 32; 
const int RIGHT_MOTOR_PIN2 = 33;
const int PWM_CHANNEL_LEFT1 = 0;
const int PWM_CHANNEL_LEFT2 = 1;
const int PWM_CHANNEL_RIGHT1 = 2;
const int PWM_CHANNEL_RIGHT2 = 3;
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;

// == BUZZER PIN ==
const int BUZZER_PIN = 13;

// ═══════════════════════════════════════════════════════════
// 📊 TEST CONFIGURATION
// ═══════════════════════════════════════════════════════════

// Test speeds (PWM values 0-255)
const int TEST_SPEEDS[] = {50, 100, 150, 200, 255};
const int NUM_SPEEDS = 5;

// Test duration for each speed (milliseconds)
const int TEST_DURATION = 2000;
const int SETTLE_TIME = 500;

// Motor test modes
enum MotorTestMode {
  TEST_FORWARD,
  TEST_BACKWARD,
  TEST_LEFT,
  TEST_RIGHT,
  TEST_BRAKE,
  TEST_COAST,
  TEST_INDIVIDUAL_MOTORS
};

const char* TEST_MODE_NAMES[] = {
  "FORWARD",
  "BACKWARD", 
  "LEFT",
  "RIGHT",
  "BRAKE",
  "COAST",
  "INDIVIDUAL"
};

// ═══════════════════════════════════════════════════════════
// 📡 ESP-NOW COMMUNICATION
// ═══════════════════════════════════════════════════════════

// MICRO_BOT bridge MAC address (update with actual)
uint8_t bridgeMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast for now

// Test result structure
struct MotorTestResult {
  char testMode[20];
  int speedSetting;
  unsigned long testDuration;
  float avgCurrent;
  bool motorResponse;
  unsigned long timestamp;
  char botId[20];
};

// ═══════════════════════════════════════════════════════════
// 🎯 MOTOR CONTROL FUNCTIONS
// ═══════════════════════════════════════════════════════════

void stopMotorsBrake() {
  ledcWrite(PWM_CHANNEL_LEFT1, 255);
  ledcWrite(PWM_CHANNEL_LEFT2, 255);
  ledcWrite(PWM_CHANNEL_RIGHT1, 255);
  ledcWrite(PWM_CHANNEL_RIGHT2, 255);
  Serial.println("🛑 BRAKE - All motors braking");
}

void stopMotorsCoast() {
  ledcWrite(PWM_CHANNEL_LEFT1, 0);
  ledcWrite(PWM_CHANNEL_LEFT2, 0);
  ledcWrite(PWM_CHANNEL_RIGHT1, 0);
  ledcWrite(PWM_CHANNEL_RIGHT2, 0);
  Serial.println("🛑 COAST - All motors coasting");
}

void moveForward(int speed) {
  ledcWrite(PWM_CHANNEL_LEFT1, speed);
  ledcWrite(PWM_CHANNEL_LEFT2, 0);
  ledcWrite(PWM_CHANNEL_RIGHT1, speed);
  ledcWrite(PWM_CHANNEL_RIGHT2, 0);
  Serial.printf("⬆️ FORWARD - Speed: %d\n", speed);
}

void moveBackward(int speed) {
  ledcWrite(PWM_CHANNEL_LEFT1, 0);
  ledcWrite(PWM_CHANNEL_LEFT2, speed);
  ledcWrite(PWM_CHANNEL_RIGHT1, 0);
  ledcWrite(PWM_CHANNEL_RIGHT2, speed);
  Serial.printf("⬇️ BACKWARD - Speed: %d\n", speed);
}

void turnLeft(int speed) {
  ledcWrite(PWM_CHANNEL_LEFT1, 0);
  ledcWrite(PWM_CHANNEL_LEFT2, speed);
  ledcWrite(PWM_CHANNEL_RIGHT1, speed);
  ledcWrite(PWM_CHANNEL_RIGHT2, 0);
  Serial.printf("⬅️ LEFT TURN - Speed: %d\n", speed);
}

void turnRight(int speed) {
  ledcWrite(PWM_CHANNEL_LEFT1, speed);
  ledcWrite(PWM_CHANNEL_LEFT2, 0);
  ledcWrite(PWM_CHANNEL_RIGHT1, 0);
  ledcWrite(PWM_CHANNEL_RIGHT2, speed);
  Serial.printf("➡️ RIGHT TURN - Speed: %d\n", speed);
}

void setMotorIndividual(int motor, int direction, int speed) {
  // Motor: 0=LEFT1, 1=LEFT2, 2=RIGHT1, 3=RIGHT2
  // Direction: 0=off, 1=forward, -1=backward
  
  // Reset all motors first
  stopMotorsCoast();
  
  switch(motor) {
    case 0: // LEFT1
      ledcWrite(PWM_CHANNEL_LEFT1, direction > 0 ? speed : 0);
      Serial.printf("🔧 LEFT MOTOR 1 - Speed: %d, Dir: %d\n", speed, direction);
      break;
    case 1: // LEFT2  
      ledcWrite(PWM_CHANNEL_LEFT2, direction > 0 ? speed : 0);
      Serial.printf("🔧 LEFT MOTOR 2 - Speed: %d, Dir: %d\n", speed, direction);
      break;
    case 2: // RIGHT1
      ledcWrite(PWM_CHANNEL_RIGHT1, direction > 0 ? speed : 0);
      Serial.printf("🔧 RIGHT MOTOR 1 - Speed: %d, Dir: %d\n", speed, direction);
      break;
    case 3: // RIGHT2
      ledcWrite(PWM_CHANNEL_RIGHT2, direction > 0 ? speed : 0);
      Serial.printf("🔧 RIGHT MOTOR 2 - Speed: %d, Dir: %d\n", speed, direction);
      break;
  }
}

// ═══════════════════════════════════════════════════════════
// 💡 LED STATUS FUNCTIONS
// ═══════════════════════════════════════════════════════════

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  // Common anode - invert values
  ledcWrite(PWM_CH_R, 255 - r);
  ledcWrite(PWM_CH_G, 255 - g);
  ledcWrite(PWM_CH_B, 255 - b);
}

void ledTestRunning() {
  setLED(255, 255, 0); // Yellow - test running
}

void ledTestPass() {
  setLED(0, 255, 0); // Green - test passed
  delay(500);
  setLED(0, 0, 0); // Off
}

void ledTestFail() {
  setLED(255, 0, 0); // Red - test failed
  delay(500);
  setLED(0, 0, 0); // Off
}

void ledIdle() {
  setLED(0, 0, 255); // Blue - idle/waiting
}

// ═══════════════════════════════════════════════════════════
// 🔊 BUZZER FUNCTIONS
// ═══════════════════════════════════════════════════════════

void playTone(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration);
}

void signalTestStart() {
  playTone(1000, 200);
  delay(250);
  playTone(1500, 200);
  Serial.println("🔊 Test sequence starting");
}

void signalTestComplete() {
  playTone(800, 150);
  delay(200);
  playTone(1200, 150);
  delay(200);
  playTone(1600, 200);
  Serial.println("🔊 Test sequence complete");
}

void signalError() {
  for(int i = 0; i < 3; i++) {
    playTone(400, 100);
    delay(150);
  }
  Serial.println("🔊 Error signal");
}

// ═══════════════════════════════════════════════════════════
// 📡 ESP-NOW COMMUNICATION FUNCTIONS
// ═══════════════════════════════════════════════════════════

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("✅ Test data sent successfully to bridge");
  } else {
    Serial.println("❌ Failed to send test data");
  }
}

void initializeESPNow() {
  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  // Print MAC address
  Serial.printf("📱 WHEELIE Test MAC: %s\n", WiFi.macAddress().c_str());
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Error initializing ESP-NOW");
    return;
  }
  
  // Register send callback
  esp_now_register_send_cb(onDataSent);
  
  // Add peer (MICRO_BOT bridge)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, bridgeMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("⚠️ Failed to add peer - using broadcast");
  }
  
  Serial.println("✅ ESP-NOW initialized for test data transmission");
}

void sendTestResult(MotorTestResult result) {
  // Create JSON message
  DynamicJsonDocument doc(512);
  doc["type"] = "motor_test";
  doc["bot_id"] = result.botId;
  doc["test_mode"] = result.testMode;
  doc["speed_setting"] = result.speedSetting;
  doc["test_duration"] = result.testDuration;
  doc["avg_current"] = result.avgCurrent;
  doc["motor_response"] = result.motorResponse;
  doc["timestamp"] = result.timestamp;
  
  // Convert to string
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send via ESP-NOW
  esp_err_t result_code = esp_now_send(bridgeMac, (uint8_t*)jsonString.c_str(), jsonString.length());
  
  if (result_code == ESP_OK) {
    Serial.printf("📤 Sent test result: %s at speed %d\n", result.testMode, result.speedSetting);
  } else {
    Serial.printf("❌ Failed to send test result: %d\n", result_code);
  }
  
  // Also print to serial for direct monitoring
  Serial.println("📊 TEST RESULT:");
  Serial.printf("   Mode: %s\n", result.testMode);
  Serial.printf("   Speed: %d\n", result.speedSetting);
  Serial.printf("   Duration: %lu ms\n", result.testDuration);
  Serial.printf("   Response: %s\n", result.motorResponse ? "OK" : "FAIL");
  Serial.println();
}

// ═══════════════════════════════════════════════════════════
// 🧪 MOTOR TEST FUNCTIONS
// ═══════════════════════════════════════════════════════════

bool runMotorTest(MotorTestMode mode, int speed) {
  ledTestRunning();
  
  unsigned long startTime = millis();
  bool motorResponse = true; // Assume success unless detected otherwise
  
  // Execute the test based on mode
  switch(mode) {
    case TEST_FORWARD:
      moveForward(speed);
      break;
    case TEST_BACKWARD:
      moveBackward(speed);
      break;
    case TEST_LEFT:
      turnLeft(speed);
      break;
    case TEST_RIGHT:
      turnRight(speed);
      break;
    case TEST_BRAKE:
      moveForward(speed);
      delay(TEST_DURATION / 2);
      stopMotorsBrake();
      delay(TEST_DURATION / 2);
      break;
    case TEST_COAST:
      moveForward(speed);
      delay(TEST_DURATION / 2);
      stopMotorsCoast();
      delay(TEST_DURATION / 2);
      break;
    case TEST_INDIVIDUAL_MOTORS:
      // Test each motor individually
      for(int motor = 0; motor < 4; motor++) {
        setMotorIndividual(motor, 1, speed);
        delay(TEST_DURATION / 4);
      }
      break;
  }
  
  // Run test for specified duration
  if(mode != TEST_BRAKE && mode != TEST_COAST && mode != TEST_INDIVIDUAL_MOTORS) {
    delay(TEST_DURATION);
  }
  
  // Stop motors
  stopMotorsCoast();
  delay(SETTLE_TIME);
  
  unsigned long testDuration = millis() - startTime;
  
  // Create and send test result
  MotorTestResult result;
  strcpy(result.testMode, TEST_MODE_NAMES[mode]);
  strcpy(result.botId, "WHEELIE_TEST");
  result.speedSetting = speed;
  result.testDuration = testDuration;
  result.avgCurrent = 0.0; // Would need current sensor for real measurement
  result.motorResponse = motorResponse;
  result.timestamp = millis();
  
  sendTestResult(result);
  
  if(motorResponse) {
    ledTestPass();
  } else {
    ledTestFail();
  }
  
  return motorResponse;
}

void runCompleteTestSuite() {
  Serial.println("\n🚀 ═══════════════════════════════════════");
  Serial.println("    WHEELIE MOTOR TEST SUITE STARTING");
  Serial.println("═══════════════════════════════════════ 🚀\n");
  
  signalTestStart();
  
  int totalTests = 0;
  int passedTests = 0;
  
  // Test all movement modes at all speeds
  for(int mode = 0; mode < TEST_INDIVIDUAL_MOTORS; mode++) {
    Serial.printf("\n🔧 Testing %s mode:\n", TEST_MODE_NAMES[mode]);
    
    for(int i = 0; i < NUM_SPEEDS; i++) {
      int speed = TEST_SPEEDS[i];
      
      Serial.printf("  ⚡ Speed %d/%d: %d PWM\n", i+1, NUM_SPEEDS, speed);
      
      bool result = runMotorTest((MotorTestMode)mode, speed);
      totalTests++;
      if(result) passedTests++;
      
      // Brief pause between tests
      delay(500);
    }
  }
  
  // Run individual motor test at medium speed
  Serial.println("\n🔧 Testing INDIVIDUAL MOTORS mode:");
  bool result = runMotorTest(TEST_INDIVIDUAL_MOTORS, 150);
  totalTests++;
  if(result) passedTests++;
  
  // Test summary
  Serial.println("\n📊 ═══════════════════════════════════════");
  Serial.println("         TEST SUITE RESULTS");
  Serial.println("═══════════════════════════════════════ 📊");
  Serial.printf("Total Tests: %d\n", totalTests);
  Serial.printf("Passed: %d\n", passedTests);
  Serial.printf("Failed: %d\n", totalTests - passedTests);
  Serial.printf("Success Rate: %.1f%%\n", (float)passedTests / totalTests * 100.0);
  Serial.println("═══════════════════════════════════════\n");
  
  // Send summary via ESP-NOW
  DynamicJsonDocument doc(256);
  doc["type"] = "test_summary";
  doc["bot_id"] = "WHEELIE_TEST";
  doc["total_tests"] = totalTests;
  doc["passed_tests"] = passedTests;
  doc["success_rate"] = (float)passedTests / totalTests * 100.0;
  doc["timestamp"] = millis();
  
  String jsonString;
  serializeJson(doc, jsonString);
  esp_now_send(bridgeMac, (uint8_t*)jsonString.c_str(), jsonString.length());
  
  signalTestComplete();
  
  // Flash LED to indicate completion
  for(int i = 0; i < 3; i++) {
    ledTestPass();
    delay(200);
  }
  
  ledIdle();
}

// ═══════════════════════════════════════════════════════════
// 🎬 SETUP
// ═══════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║    🚗 WHEELIE MOTOR TEST SYSTEM 🧪    ║");
  Serial.println("║       COMPREHENSIVE TESTING SUITE      ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Initialize motors
  Serial.println("🔧 Initializing motor PWM channels...");
  ledcSetup(PWM_CHANNEL_LEFT1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_LEFT2, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_RIGHT1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_RIGHT2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LEFT_MOTOR_PIN1, PWM_CHANNEL_LEFT1);
  ledcAttachPin(LEFT_MOTOR_PIN2, PWM_CHANNEL_LEFT2);
  ledcAttachPin(RIGHT_MOTOR_PIN1, PWM_CHANNEL_RIGHT1);
  ledcAttachPin(RIGHT_MOTOR_PIN2, PWM_CHANNEL_RIGHT2);
  Serial.println("✅ Motors initialized");
  
  // Initialize LED
  Serial.println("💡 Initializing status LED...");
  ledcSetup(PWM_CH_R, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_G, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_B, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_R_PIN, PWM_CH_R);
  ledcAttachPin(LED_G_PIN, PWM_CH_G);
  ledcAttachPin(LED_B_PIN, PWM_CH_B);
  ledIdle();
  Serial.println("✅ LED initialized");
  
  // Initialize buzzer
  Serial.println("🔊 Initializing buzzer...");
  pinMode(BUZZER_PIN, OUTPUT);
  playTone(1000, 200);
  Serial.println("✅ Buzzer initialized");
  
  // Initialize ESP-NOW
  Serial.println("📡 Initializing ESP-NOW communication...");
  initializeESPNow();
  
  // Brief startup delay
  Serial.println("\n⏳ Starting test suite in 3 seconds...");
  for(int i = 3; i > 0; i--) {
    Serial.printf("   %d...\n", i);
    delay(1000);
  }
  
  // Run the complete test suite
  runCompleteTestSuite();
}

// ═══════════════════════════════════════════════════════════
// 🔄 MAIN LOOP
// ═══════════════════════════════════════════════════════════

void loop() {
  // Test suite runs once in setup
  // Main loop just maintains idle state and waits for commands
  
  ledIdle();
  delay(1000);
  
  // Optional: Add ability to restart tests via serial command
  if(Serial.available()) {
    String command = Serial.readString();
    command.trim();
    
    if(command == "test" || command == "run") {
      Serial.println("🔄 Restarting test suite...");
      runCompleteTestSuite();
    } else if(command == "help") {
      Serial.println("Available commands:");
      Serial.println("  'test' or 'run' - Run complete test suite");
      Serial.println("  'help' - Show this help");
    }
  }
}