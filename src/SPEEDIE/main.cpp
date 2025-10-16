/*
 * ⚡ SPEEDIE: High-Performance Evolutionary Scout Bot
 * 
 * Hardware Platform: ESP32 Type C CH340 Development Board
 * - ESP-WROOM-32 dual-core 2.4GHz Wi-Fi + Bluetooth + BLE MCU
 * - USB Type-C interface with CH340 TTL chip for stable communication
 * - 40nm low power technology with integrated antenna & RF balance
 * - Support for AP, STA, AP+STA coexist modes, LWIP, FreeRTOS
 * - Capacitive touch, Hall sensors, low noise sensor peripherals
 * - Automatic download mode switching
 * 
 * SPEEDIE Features:
 * - Speed-optimized evolutionary parameters
 * - HC-SR04 ultrasonic + MPU-6050 IMU sensor fusion (NO motion sensor)
 * - Simple red/green LED emotional signaling (no buzzer for max speed)
 * - Immediate activation for ultra-fast start
 * - Ultra-fast obstacle avoidance and learning
 * - 10kHz PWM for smooth high-speed motor control
 * - Rapid evolution cycles (20s-2min adaptive intervals)
 */

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <EEPROM.h>

// ═══════════════════════════════════════════════════════════
// 💡 SPEEDIE LED CONFIGURATION (4 INDIVIDUAL 2-WIRE LEDs)
// ═══════════════════════════════════════════════════════════
// Hardware: 4 simple 2-wire LEDs (NOT RGB, NOT NeoPixel)
// - 2 RED LEDs (left + right sides)
// - 2 GREEN LEDs (left + right sides)
// - Common Anode configuration: LOW = ON, HIGH = OFF

// Left side LEDs (2 individual LEDs)
const int LEFT_LED_R_PIN = 2;    // Left RED LED (pin 2)
const int LEFT_LED_G_PIN = 4;    // Left GREEN LED (pin 4)

// Right side LEDs (2 individual LEDs)  
const int RIGHT_LED_R_PIN = 12;  // Right RED LED (pin 12)
const int RIGHT_LED_G_PIN = 14;  // Right GREEN LED (pin 14)

// PWM Channels for individual LED control
const int PWM_CH_L_R = 4;        // Left RED LED PWM channel
const int PWM_CH_L_G = 5;        // Left GREEN LED PWM channel
const int PWM_CH_R_R = 6;        // Right RED LED PWM channel
const int PWM_CH_R_G = 7;        // Right GREEN LED PWM channel

// ═══════════════════════════════════════════════════════════
// 🔍 SPEEDIE SENSOR CONFIGURATION (NO MOTION SENSOR)
// ═══════════════════════════════════════════════════════════
// SPEEDIE sensors for speed-optimized operation:
// - HC-SR04 Ultrasonic: Primary distance sensing
// - MPU-6050 IMU: Gyroscope and accelerometer for orientation
// - NO Motion Sensor: SPEEDIE relies on timer-based activation for max speed

const int ULTRASONIC_TRIG_PIN = 18;   // HC-SR04 Trigger pin
const int ULTRASONIC_ECHO_PIN = 19;   // HC-SR04 Echo pin
Adafruit_MPU6050 mpu;                 // IMU for sensor fusion

// == MOTOR PINS & PWM ==
const int LEFT_MOTOR_PIN1 = 26;
const int LEFT_MOTOR_PIN2 = 25;
const int RIGHT_MOTOR_PIN1 = 32; 
const int RIGHT_MOTOR_PIN2 = 33;
const int PWM_CHANNEL_LEFT1 = 0;
const int PWM_CHANNEL_LEFT2 = 1;
const int PWM_CHANNEL_RIGHT1 = 2;
const int PWM_CHANNEL_RIGHT2 = 3;
const int PWM_FREQ = 10000; // Higher frequency for SPEEDIE
const int PWM_RESOLUTION = 8;

// ═══════════════════════════════════════════════════════════
// 🧬 EVOLUTIONARY PARAMETERS - SPEEDIE'S GENOME (SPEED-OPTIMIZED)
// ═══════════════════════════════════════════════════════════
struct EvolvingGenome {
  // Movement parameters (mutable) - SPEEDIE optimized for speed
  int motorSpeed = 240;         // Higher base speed than WHEELIE
  int turnSpeed = 180;          // Faster turns
  int backupDuration = 400;     // Shorter backup (quick reactions)
  int turnDuration = 250;       // Quicker turns
  
  // Perception parameters (mutable) - Faster response
  int obstacleThreshold = 150;  // Closer threshold (faster reactions)
  int clearThreshold = 250;     // Closer clear distance
  int scanDelay = 150;          // Faster scanning
  
  // Strategy parameters (mutable) - Aggressive escape
  int aggressiveBackupMultiplier = 3; // More aggressive than WHEELIE
  int spinDegreesWhenTrapped = 270;   // Bigger spins for faster escape
  
  // SPEEDIE-specific parameters
  int maxAcceleration = 50;     // How fast to ramp up speed
  int corneringSpeed = 160;     // Speed when turning
  float gyroSensitivity = 2.0;  // IMU turning sensitivity
  
  // Evolution metadata
  unsigned long successCount = 0;
  unsigned long failureCount = 0;
  float fitnessScore = 0.0;
  unsigned long generation = 0;
};

// 🗣️ EMERGENT LANGUAGE STRUCTURES (Same as WHEELIE)
const int MAX_VOCABULARY = 50;
const int BUZZER_PIN = -1; // No buzzer for SPEEDIE (speed focused)

struct SignalWord {
  int contextType;           // 0=obstacle, 1=success, 2=trapped, 3=clear, 4=evolving
  int emotionalValence;      // -100 to +100
  int generation;           // When this signal was created
  float utility;            // 0.0-1.0 how useful this signal is
  unsigned long timesUsed;
  int patternLength;
  int tonePattern[6];       // Frequencies for each tone
  int durationPattern[6];   // Durations for each tone
  uint8_t r, g, b;          // Associated RGB color for this signal
};

struct EmotionalState {
  int frustrationLevel = 0;   // 0-100
  int confidenceLevel = 60;   // SPEEDIE starts more confident
  int curiosityLevel = 70;    // SPEEDIE is more curious (exploration)
  bool isDistressed = false;
  bool isTriumphant = false;
  unsigned long lastCommunication = 0;
};

// Global variables for emergent language
SignalWord vocabulary[MAX_VOCABULARY];
int vocabularySize = 0;
EmotionalState currentState;
bool hasBuzzer = false;

// ═══════════════════════════════════════════════════════════
// 🧠 LEARNED STRATEGIES - SPEEDIE'S MEMORY (SPEED-FOCUSED)
// ═══════════════════════════════════════════════════════════
struct LearnedStrategy {
  int contextDistance;        // Distance when this worked
  int turnDirection;          // 0=left, 1=right
  int backupTime;            // How long to back up
  int turnTime;              // How long to turn
  float successRate;         // How often this works (0.0-1.0)
  unsigned long timesUsed;   // How many times tried
  unsigned long timesSucceeded; // How many times succeeded
  float avgCompletionTime;   // SPEEDIE tracks time to completion
};

const int MAX_STRATEGIES = 25; // More strategies for SPEEDIE
LearnedStrategy strategyLibrary[MAX_STRATEGIES];
int strategyCount = 0;

// ═══════════════════════════════════════════════════════════
// 🌱 EVOLUTION STATE (SPEEDIE VERSION)
// ═══════════════════════════════════════════════════════════
EvolvingGenome currentGenome;
EvolvingGenome previousGenome;
bool evolutionEnabled = true;
unsigned long lastEvolutionTime = 0;
const unsigned long EVOLUTION_INTERVAL_BASE = 45000; // Faster evolution (45s base)
unsigned long EVOLUTION_INTERVAL = EVOLUTION_INTERVAL_BASE;

// ═══════════════════════════════════════════════════════════
// 📊 PERFORMANCE TRACKING (SPEEDIE METRICS)
// ═══════════════════════════════════════════════════════════
struct PerformanceMetrics {
  unsigned long obstaclesEncountered = 0;
  unsigned long obstaclesCleared = 0;
  unsigned long timesTrapped = 0;
  unsigned long trapEscapes = 0;
  unsigned long totalDistanceTraveled = 0;
  unsigned long missionStartTime = 0;
  float averageEscapeTime = 0.0;
  float averageSpeed = 0.0;         // SPEEDIE tracks average speed
  unsigned long fastestObstacleTime = 999999; // Fastest obstacle clearance
};

PerformanceMetrics metrics;
unsigned long currentObstacleStartTime = 0;

// ═══════════════════════════════════════════════════════════
// 💾 EEPROM PERSISTENCE (SPEEDIE LAYOUT)
// ═══════════════════════════════════════════════════════════
const int EEPROM_SIZE = 4096;
const int GENOME_ADDRESS = 0;
const int STRATEGIES_ADDRESS = sizeof(EvolvingGenome);
const int METRICS_ADDRESS = STRATEGIES_ADDRESS + (sizeof(LearnedStrategy) * MAX_STRATEGIES);
const int VOCABULARY_ADDRESS = METRICS_ADDRESS + sizeof(PerformanceMetrics);

// Other state variables
bool isAwake = false;
bool isAvoiding = false;
unsigned long lastActivityTime = 0;
const unsigned long SLEEP_TIMEOUT_MS = 20000; // Shorter sleep timeout for SPEEDIE
const int SENSOR_ERROR_VALUE = 9999;
int consecutiveSensorErrors = 0;
const int MAX_CONSECUTIVE_ERRORS = 3; // Less tolerance for errors
int trappedAttempts = 0;
const int MAX_TRAPPED_ATTEMPTS = 2; // Escape faster

// IMU data
float currentHeading = 0.0;
float targetHeading = 0.0;

// ═══════════════════════════════════════════════════════════
// 🗣️ EMERGENT LANGUAGE & PERSISTENCE FUNCTIONS (SPEEDIE VERSION)
// ═══════════════════════════════════════════════════════════

void saveGenomeToEEPROM() {
  EEPROM.put(GENOME_ADDRESS, currentGenome);
  EEPROM.commit();
  Serial.println("💾 SPEEDIE Genome saved to persistent memory");
}

void loadGenomeFromEEPROM() {
  EEPROM.get(GENOME_ADDRESS, currentGenome);
  Serial.println("📖 SPEEDIE Genome loaded from memory");
  Serial.print("Generation: ");
  Serial.println(currentGenome.generation);
  Serial.print("Fitness: ");
  Serial.println(currentGenome.fitnessScore);
}

void saveStrategiesToEEPROM() {
  for (int i = 0; i < strategyCount && i < MAX_STRATEGIES; i++) {
    EEPROM.put(STRATEGIES_ADDRESS + (i * sizeof(LearnedStrategy)), strategyLibrary[i]);
  }
  EEPROM.put(STRATEGIES_ADDRESS + (MAX_STRATEGIES * sizeof(LearnedStrategy)), strategyCount);
  EEPROM.commit();
  Serial.print("💾 Saved ");
  Serial.print(strategyCount);
  Serial.println(" SPEEDIE strategies to memory");
}

void loadStrategiesFromEEPROM() {
  EEPROM.get(STRATEGIES_ADDRESS + (MAX_STRATEGIES * sizeof(LearnedStrategy)), strategyCount);
  if (strategyCount > MAX_STRATEGIES) strategyCount = 0;
  
  for (int i = 0; i < strategyCount; i++) {
    EEPROM.get(STRATEGIES_ADDRESS + (i * sizeof(LearnedStrategy)), strategyLibrary[i]);
  }
  Serial.print("📖 Loaded ");
  Serial.print(strategyCount);
  Serial.println(" SPEEDIE strategies from memory");
}

void saveMetricsToEEPROM() {
  EEPROM.put(METRICS_ADDRESS, metrics);
  EEPROM.commit();
}

void loadMetricsFromEEPROM() {
  EEPROM.get(METRICS_ADDRESS, metrics);
}

void saveVocabularyToEEPROM() {
  for (int i = 0; i < vocabularySize && i < MAX_VOCABULARY; i++) {
    EEPROM.put(VOCABULARY_ADDRESS + (i * sizeof(SignalWord)), vocabulary[i]);
  }
  EEPROM.put(VOCABULARY_ADDRESS + (MAX_VOCABULARY * sizeof(SignalWord)), vocabularySize);
  EEPROM.commit();
  Serial.print("💾 Saved ");
  Serial.print(vocabularySize);
  Serial.println(" SPEEDIE words to vocabulary");
}

void loadVocabularyFromEEPROM() {
  EEPROM.get(VOCABULARY_ADDRESS + (MAX_VOCABULARY * sizeof(SignalWord)), vocabularySize);
  if (vocabularySize > MAX_VOCABULARY) vocabularySize = 0;
  
  for (int i = 0; i < vocabularySize; i++) {
    EEPROM.get(VOCABULARY_ADDRESS + (i * sizeof(SignalWord)), vocabulary[i]);
  }
  Serial.print("📖 Loaded ");
  Serial.print(vocabularySize);
  Serial.println(" SPEEDIE words from vocabulary");
}

// Generate SPEEDIE-specific signals (faster, more energetic patterns)
void createNewSignal(int contextType, int emotionalValence) {
  if (vocabularySize >= MAX_VOCABULARY) {
    int worstIndex = 0;
    float worstUtility = vocabulary[0].utility;
    for (int i = 1; i < vocabularySize; i++) {
      if (vocabulary[i].utility < worstUtility) {
        worstUtility = vocabulary[i].utility;
        worstIndex = i;
      }
    }
    vocabularySize = worstIndex;
  }
  
  SignalWord newWord;
  newWord.contextType = contextType;
  newWord.emotionalValence = emotionalValence;
  newWord.generation = currentGenome.generation;
  newWord.utility = 0.5;
  newWord.timesUsed = 0;
  
  newWord.patternLength = random(3, 6); // SPEEDIE prefers shorter, punchier signals
  
  // SPEEDIE color patterns - more vibrant and energetic
  if (emotionalValence < -30) { // Fast distress - bright reds
    newWord.r = random(40, 80);
    newWord.g = random(0, 5);
    newWord.b = random(0, 5);
  } else if (emotionalValence > 30) { // Success - bright blues/greens
    newWord.r = random(0, 5);
    newWord.g = random(30, 70);
    newWord.b = random(40, 80);
  } else { // Neutral - bright whites/cyans
    newWord.r = random(20, 50);
    newWord.g = random(30, 60);
    newWord.b = random(40, 70);
  }
  
  for (int i = 0; i < newWord.patternLength; i++) {
    if (emotionalValence < -30) {
      // SPEEDIE distress - very fast, very high
      newWord.tonePattern[i] = random(2000, 4000);
      newWord.durationPattern[i] = random(30, 80);
    } else if (emotionalValence > 30) {
      // SPEEDIE success - rapid ascending patterns
      int baseFreq = random(800, 1500);
      newWord.tonePattern[i] = baseFreq + (i * 150);
      newWord.durationPattern[i] = random(50, 120);
    } else {
      // SPEEDIE neutral - moderate but quick
      newWord.tonePattern[i] = random(1000, 2500);
      newWord.durationPattern[i] = random(60, 150);
    }
  }
  
  vocabulary[vocabularySize] = newWord;
  vocabularySize++;
  
  Serial.println("⚡ SPEEDIE created new high-energy signal!");
  Serial.print("  Context: ");
  Serial.print(contextType);
  Serial.print(" | Valence: ");
  Serial.print(emotionalValence);
  Serial.print(" | Pattern length: ");
  Serial.println(newWord.patternLength);
}

// ═══════════════════════════════════════════════════════════
// ⚡ SPEEDIE LED EMOTIONAL SIGNALING (4 INDIVIDUAL 2-WIRE LEDs)
// ═══════════════════════════════════════════════════════════
// Hardware Configuration:
// - 2 RED LEDs: Left (pin 2) + Right (pin 12) 
// - 2 GREEN LEDs: Left (pin 4) + Right (pin 14)
// - Common Anode wiring: 0 = LED ON, 255 = LED OFF
// - No buzzer for maximum speed performance
void emitSignal(SignalWord* word) {
  if (word == nullptr) return;
  
  // SPEEDIE uses simple Red/Green LED patterns for speed
  // Convert RGB emotions to Red/Green binary states
  bool isRed = (word->r > word->g); // Red if more red than green
  bool isGreen = (word->g > word->r); // Green if more green than red
  
  // Set LED states based on emotion (Common Anode: 0=ON, 255=OFF)
  uint8_t red_intensity = isRed ? 0 : 255;    // Turn RED LEDs ON/OFF
  uint8_t green_intensity = isGreen ? 0 : 255; // Turn GREEN LEDs ON/OFF

  // Control all 4 individual LEDs simultaneously
  ledcWrite(PWM_CH_L_R, red_intensity);    // Left RED LED
  ledcWrite(PWM_CH_L_G, green_intensity);  // Left GREEN LED  
  ledcWrite(PWM_CH_R_R, red_intensity);    // Right RED LED
  ledcWrite(PWM_CH_R_G, green_intensity);  // Right GREEN LED

  // Quick flash pattern for SPEEDIE
  for (int i = 0; i < word->patternLength; i++) {
    delay(word->durationPattern[i] / 2); // Half duration for speed
    // Quick flash off (both LEDs off)
    ledcWrite(PWM_CH_L_R, 255); ledcWrite(PWM_CH_L_G, 255);
    ledcWrite(PWM_CH_R_R, 255); ledcWrite(PWM_CH_R_G, 255);
    delay(20);
    // Back to color
    ledcWrite(PWM_CH_L_R, red_intensity); ledcWrite(PWM_CH_L_G, green_intensity);
    ledcWrite(PWM_CH_R_R, red_intensity); ledcWrite(PWM_CH_R_G, green_intensity);
  }
  
  Serial.print("⚡ SPEEDIE SIGNAL: ");
  for (int i = 0; i < word->patternLength; i++) {
    Serial.print(word->tonePattern[i]);
    Serial.print("Hz/");
    Serial.print(word->durationPattern[i]);
    Serial.print("ms ");
  }
  Serial.println();
  
  word->timesUsed++;
  currentState.lastCommunication = millis();
}

// Find best matching signal for current context
SignalWord* findSignalForContext(int contextType, int emotionalValence) {
  SignalWord* bestMatch = nullptr;
  float bestScore = -1.0;
  
  for (int i = 0; i < vocabularySize; i++) {
    float contextMatch = (vocabulary[i].contextType == contextType) ? 1.0 : 0.3;
    float emotionSimilarity = 1.0 - (abs(vocabulary[i].emotionalValence - emotionalValence) / 200.0);
    float utilityBonus = vocabulary[i].utility * 0.5;
    
    float score = (contextMatch * 0.5) + (emotionSimilarity * 0.3) + utilityBonus;
    
    if (score > bestScore) {
      bestScore = score;
      bestMatch = &vocabulary[i];
    }
  }
  
  if (bestScore < 0.5 && vocabularySize < MAX_VOCABULARY) {
    createNewSignal(contextType, emotionalValence);
    return &vocabulary[vocabularySize - 1];
  }
  
  return bestMatch;
}

// SPEEDIE emotional state (more aggressive/confident baseline)
void updateEmotionalState() {
  currentState.frustrationLevel = min(100, trappedAttempts * 35 + 
                                     (int)(currentGenome.failureCount * 3));
  
  if (metrics.obstaclesEncountered > 0) {
    float successRate = (float)metrics.obstaclesCleared / metrics.obstaclesEncountered;
    currentState.confidenceLevel = (int)(successRate * 60 + currentGenome.fitnessScore * 40);
  }
  
  currentState.curiosityLevel = 70 - (currentState.frustrationLevel / 3);
  currentState.curiosityLevel = constrain(currentState.curiosityLevel, 30, 90);
  
  currentState.isDistressed = (currentState.frustrationLevel > 60); // Lower threshold
  currentState.isTriumphant = (currentGenome.fitnessScore > 0.7 && 
                              metrics.obstaclesCleared > 3);
}

void expressState(int contextType, int emotionalValence) {
  updateEmotionalState();
  
  Serial.print("⚡ SPEEDIE State: Frustration=");
  Serial.print(currentState.frustrationLevel);
  Serial.print(" Confidence=");
  Serial.print(currentState.confidenceLevel);
  Serial.print(" Curiosity=");
  Serial.println(currentState.curiosityLevel);
  
  SignalWord* signal = findSignalForContext(contextType, emotionalValence);
  if (signal != nullptr) {
    emitSignal(signal);
  }
}

void evolveVocabulary() {
  Serial.println("⚡ SPEEDIE evolving vocabulary...");
  
  for (int i = 0; i < vocabularySize; i++) {
    if (vocabulary[i].timesUsed > 0) {
      float usageBonus = min(1.0, vocabulary[i].timesUsed / 8.0); // Faster usage bonus
      
      float fitnessAlignment = (currentGenome.fitnessScore > 0.5) ? 
        ((vocabulary[i].emotionalValence > 0) ? 0.3 : -0.1) :
        ((vocabulary[i].emotionalValence < 0) ? 0.3 : -0.1);
      
      vocabulary[i].utility = constrain(usageBonus + fitnessAlignment, 0.0, 1.0);
    }
  }
  
  if (random(0, 100) < 40 && vocabularySize > 0) { // More mutation for SPEEDIE
    int mutateIndex = random(0, vocabularySize);
    int elementToMutate = random(0, vocabulary[mutateIndex].patternLength);
    
    vocabulary[mutateIndex].tonePattern[elementToMutate] += random(-300, 301);
    vocabulary[mutateIndex].tonePattern[elementToMutate] = constrain(
      vocabulary[mutateIndex].tonePattern[elementToMutate], 300, 5000
    );
    
    Serial.print("⚡ SPEEDIE mutated signal #");
    Serial.println(mutateIndex);
  }
  
  saveVocabularyToEEPROM();
}

void initializeDefaultVocabulary() {
  if (vocabularySize == 0) {
    Serial.println("⚡ Creating SPEEDIE default vocabulary...");
    
    createNewSignal(0, -50);  // Obstacle detected
    createNewSignal(1, 80);   // Success  
    createNewSignal(2, -90);  // Trapped
    createNewSignal(3, 50);   // Clear path
    createNewSignal(4, 40);   // System/evolving
    
    saveVocabularyToEEPROM();
  }
}

// ═══════════════════════════════════════════════════════════
// 🧬 SPEEDIE MUTATION FUNCTIONS (SPEED-OPTIMIZED)
// ═══════════════════════════════════════════════════════════

void mutateGenome() {
  Serial.println("\n⚡ === SPEEDIE MUTATION EVENT ===");
  previousGenome = currentGenome;
  
  int mutationCount = random(1, 4);
  
  for (int i = 0; i < mutationCount; i++) {
    int mutationType = random(0, 12); // More parameters for SPEEDIE
    
    switch(mutationType) {
      case 0: // Motor speed
        currentGenome.motorSpeed += random(-20, 21);
        currentGenome.motorSpeed = constrain(currentGenome.motorSpeed, 180, 255);
        Serial.print("  ⚡ Motor speed → ");
        Serial.println(currentGenome.motorSpeed);
        break;
        
      case 1: // Turn speed
        currentGenome.turnSpeed += random(-15, 16);
        currentGenome.turnSpeed = constrain(currentGenome.turnSpeed, 120, 220);
        Serial.print("  🔄 Turn speed → ");
        Serial.println(currentGenome.turnSpeed);
        break;
        
      case 2: // Backup duration
        currentGenome.backupDuration += random(-50, 51);
        currentGenome.backupDuration = constrain(currentGenome.backupDuration, 200, 800);
        Serial.print("  ⏪ Backup time → ");
        Serial.println(currentGenome.backupDuration);
        break;
        
      case 3: // Turn duration
        currentGenome.turnDuration += random(-30, 31);
        currentGenome.turnDuration = constrain(currentGenome.turnDuration, 150, 400);
        Serial.print("  🔀 Turn time → ");
        Serial.println(currentGenome.turnDuration);
        break;
        
      case 4: // Obstacle threshold
        currentGenome.obstacleThreshold += random(-20, 21);
        currentGenome.obstacleThreshold = constrain(currentGenome.obstacleThreshold, 80, 300);
        Serial.print("  ⚠️ Obstacle threshold → ");
        Serial.println(currentGenome.obstacleThreshold);
        break;
        
      case 5: // Clear threshold
        currentGenome.clearThreshold += random(-30, 31);
        currentGenome.clearThreshold = constrain(currentGenome.clearThreshold, 150, 400);
        Serial.print("  ✅ Clear threshold → ");
        Serial.println(currentGenome.clearThreshold);
        break;
        
      case 6: // Scan delay
        currentGenome.scanDelay += random(-30, 31);
        currentGenome.scanDelay = constrain(currentGenome.scanDelay, 50, 300);
        Serial.print("  👁️ Scan delay → ");
        Serial.println(currentGenome.scanDelay);
        break;
        
      case 7: // Aggressive backup multiplier
        currentGenome.aggressiveBackupMultiplier = random(2, 6);
        Serial.print("  💪 Aggressive backup → ");
        Serial.println(currentGenome.aggressiveBackupMultiplier);
        break;
        
      case 8: // Spin degrees when trapped
        currentGenome.spinDegreesWhenTrapped = random(180, 360);
        Serial.print("  🌀 Trapped spin → ");
        Serial.println(currentGenome.spinDegreesWhenTrapped);
        break;
        
      case 9: // Max acceleration (SPEEDIE specific)
        currentGenome.maxAcceleration += random(-10, 11);
        currentGenome.maxAcceleration = constrain(currentGenome.maxAcceleration, 20, 100);
        Serial.print("  🚀 Max acceleration → ");
        Serial.println(currentGenome.maxAcceleration);
        break;
        
      case 10: // Cornering speed (SPEEDIE specific)
        currentGenome.corneringSpeed += random(-20, 21);
        currentGenome.corneringSpeed = constrain(currentGenome.corneringSpeed, 100, 200);
        Serial.print("  🏎️ Cornering speed → ");
        Serial.println(currentGenome.corneringSpeed);
        break;
        
      case 11: // Gyro sensitivity (SPEEDIE specific)
        currentGenome.gyroSensitivity += random(-50, 51) / 100.0;
        currentGenome.gyroSensitivity = constrain(currentGenome.gyroSensitivity, 0.5, 5.0);
        Serial.print("  🌀 Gyro sensitivity → ");
        Serial.println(currentGenome.gyroSensitivity);
        break;
    }
  }
  
  currentGenome.generation++;
  Serial.print("⚡ SPEEDIE Generation: ");
  Serial.println(currentGenome.generation);
}

// ═══════════════════════════════════════════════════════════
// 📊 SPEEDIE FITNESS CALCULATION (SPEED-FOCUSED)
// ═══════════════════════════════════════════════════════════

void calculateFitness() {
  float successRate = 0.0;
  if (metrics.obstaclesEncountered > 0) {
    successRate = (float)metrics.obstaclesCleared / (float)metrics.obstaclesEncountered;
  }
  
  float escapeRate = 0.0;
  if (metrics.timesTrapped > 0) {
    escapeRate = (float)metrics.trapEscapes / (float)metrics.timesTrapped;
  }
  
  // SPEEDIE fitness includes speed factor
  float speedFactor = 1.0;
  if (metrics.fastestObstacleTime < 999999) {
    speedFactor = 1.0 + (5000.0 / max(metrics.fastestObstacleTime, 1000UL)); // Bonus for speed
  }
  
  currentGenome.fitnessScore = ((successRate * 0.5) + (escapeRate * 0.3) + (speedFactor * 0.2));
  currentGenome.fitnessScore = constrain(currentGenome.fitnessScore, 0.0, 2.0); // Allow >1.0 for speed bonus
  
  Serial.println("\n⚡ === SPEEDIE FITNESS REPORT ===");
  Serial.print("Success Rate: ");
  Serial.print(successRate * 100);
  Serial.println("%");
  Serial.print("Escape Rate: ");
  Serial.print(escapeRate * 100);
  Serial.println("%");
  Serial.print("Speed Factor: ");
  Serial.println(speedFactor);
  Serial.print("Overall Fitness: ");
  Serial.println(currentGenome.fitnessScore);
}

// ═══════════════════════════════════════════════════════════
// 🧠 SPEEDIE MEMORY & CONSTRAINT MANAGEMENT
// ═══════════════════════════════════════════════════════════

void pruneWeakStrategies() {
  for (int i = strategyCount - 1; i >= 0; i--) {
    if (strategyLibrary[i].timesUsed >= 2 &&  // Lower threshold for SPEEDIE
        strategyLibrary[i].successRate < 0.4) {
      Serial.print("⚡ Pruning weak SPEEDIE strategy #");
      Serial.println(i);
      
      for (int j = i; j < strategyCount - 1; j++) {
        strategyLibrary[j] = strategyLibrary[j + 1];
      }
      strategyCount--;
    }
  }
}

void compactStrategyArray() {
  int writeIndex = 0;
  for (int readIndex = 0; readIndex < strategyCount; readIndex++) {
    if (strategyLibrary[readIndex].timesUsed > 0) {
      if (writeIndex != readIndex) {
        strategyLibrary[writeIndex] = strategyLibrary[readIndex];
      }
      writeIndex++;
    }
  }
  strategyCount = writeIndex;
  Serial.print("⚡ Compacted SPEEDIE strategy array to ");
  Serial.print(strategyCount);
  Serial.println(" strategies");
}

void applyEvolutionaryConstraints() {
  currentGenome.motorSpeed = constrain(currentGenome.motorSpeed, 180, 255);
  currentGenome.turnSpeed = constrain(currentGenome.turnSpeed, 120, 220);
  currentGenome.obstacleThreshold = constrain(currentGenome.obstacleThreshold, 80, 300);
  
  if (currentGenome.clearThreshold <= currentGenome.obstacleThreshold) {
    currentGenome.clearThreshold = currentGenome.obstacleThreshold + 40;
  }
}

// ═══════════════════════════════════════════════════════════
// 🔄 SPEEDIE EVOLUTION ENGINE (FASTER CYCLES)
// ═══════════════════════════════════════════════════════════

void evolutionCycle() {
  if (!evolutionEnabled) return;
  
  unsigned long now = millis();
  
  // SPEEDIE adaptive evolution (faster base, more aggressive adaptation)
  EVOLUTION_INTERVAL = EVOLUTION_INTERVAL_BASE * (0.3 + (currentGenome.fitnessScore * 1.2));
  EVOLUTION_INTERVAL = constrain(EVOLUTION_INTERVAL, 20000, 120000); // 20s to 2min range
  
  if (now - lastEvolutionTime < EVOLUTION_INTERVAL) return;
  
  lastEvolutionTime = now;
  
  Serial.println("\n\n⚡ ═══════════════════════════════════════");
  Serial.println("       SPEEDIE EVOLUTION CYCLE TRIGGERED");
  Serial.println("═══════════════════════════════════════ ⚡\n");
  
  expressState(4, 10); // Context: evolving, slightly positive
  
  calculateFitness();
  
  if (currentGenome.generation > 0) {
    if (currentGenome.fitnessScore >= previousGenome.fitnessScore) {
      Serial.println("⚡ SPEEDIE Mutation SUCCESSFUL - keeping changes");
      currentGenome.successCount++;
      
      expressState(1, 90); // Very positive for success
      
      if (random(0, 100) < 40) { // Higher chance for bonus mutation
        Serial.println("⚡ Bonus SPEEDIE mutation for successful genome");
        mutateGenome();
      }
    } else {
      Serial.println("❌ SPEEDIE Mutation FAILED - reverting to previous genome");
      currentGenome = previousGenome;
      currentGenome.failureCount++;
      currentGenome.generation++;
      
      expressState(1, -50); // More negative reaction
    }
  } else {
    mutateGenome();
  }
  
  if (vocabularySize > 0) {
    evolveVocabulary();
  }
  
  saveGenomeToEEPROM();
  saveMetricsToEEPROM();
  
  applyEvolutionaryConstraints();
  pruneWeakStrategies();
  
  Serial.println("\n⚡ Current SPEEDIE Genome:");
  Serial.print("  Motor Speed: ");
  Serial.println(currentGenome.motorSpeed);
  Serial.print("  Turn Speed: ");
  Serial.println(currentGenome.turnSpeed);
  Serial.print("  Max Acceleration: ");
  Serial.println(currentGenome.maxAcceleration);
  Serial.print("  Vocabulary Size: ");
  Serial.println(vocabularySize);
  Serial.println("\n═══════════════════════════════════════\n");
}

// Continue in next part due to length...

// ═══════════════════════════════════════════════════════════
// 🎓 SPEEDIE STRATEGY LEARNING (SPEED-FOCUSED)
// ═══════════════════════════════════════════════════════════

void learnStrategy(int distance, int direction, int backupTime, int turnTime, bool succeeded, unsigned long completionTime) {
  int similarIndex = -1;
  for (int i = 0; i < strategyCount; i++) {
    if (abs(strategyLibrary[i].contextDistance - distance) < 40 && // Tighter matching for SPEEDIE
        strategyLibrary[i].turnDirection == direction) {
      similarIndex = i;
      break;
    }
  }
  
  if (similarIndex >= 0) {
    strategyLibrary[similarIndex].timesUsed++;
    if (succeeded) {
      strategyLibrary[similarIndex].timesSucceeded++;
      // Update average completion time
      strategyLibrary[similarIndex].avgCompletionTime = 
        (strategyLibrary[similarIndex].avgCompletionTime + completionTime) / 2.0;
    }
    strategyLibrary[similarIndex].successRate = 
      (float)strategyLibrary[similarIndex].timesSucceeded / 
      (float)strategyLibrary[similarIndex].timesUsed;
    
    Serial.print("⚡ Updated SPEEDIE strategy #");
    Serial.print(similarIndex);
    Serial.print(" - Success rate: ");
    Serial.print(strategyLibrary[similarIndex].successRate * 100);
    Serial.print("% - Avg time: ");
    Serial.println(strategyLibrary[similarIndex].avgCompletionTime);
  } else if (strategyCount < MAX_STRATEGIES) {
    strategyLibrary[strategyCount].contextDistance = distance;
    strategyLibrary[strategyCount].turnDirection = direction;
    strategyLibrary[strategyCount].backupTime = backupTime;
    strategyLibrary[strategyCount].turnTime = turnTime;
    strategyLibrary[strategyCount].timesUsed = 1;
    strategyLibrary[strategyCount].timesSucceeded = succeeded ? 1 : 0;
    strategyLibrary[strategyCount].successRate = succeeded ? 1.0 : 0.0;
    strategyLibrary[strategyCount].avgCompletionTime = completionTime;
    
    Serial.print("⚡ Learned new SPEEDIE strategy #");
    Serial.println(strategyCount);
    strategyCount++;
  } else {
    Serial.println("⚡ SPEEDIE strategy library full - applying smart management");
    pruneWeakStrategies();
    compactStrategyArray();
    
    if (strategyCount < MAX_STRATEGIES) {
      strategyLibrary[strategyCount].contextDistance = distance;
      strategyLibrary[strategyCount].turnDirection = direction;
      strategyLibrary[strategyCount].backupTime = backupTime;
      strategyLibrary[strategyCount].turnTime = turnTime;
      strategyLibrary[strategyCount].timesUsed = 1;
      strategyLibrary[strategyCount].timesSucceeded = succeeded ? 1 : 0;
      strategyLibrary[strategyCount].successRate = succeeded ? 1.0 : 0.0;
      strategyLibrary[strategyCount].avgCompletionTime = completionTime;
      
      Serial.print("⚡ Added SPEEDIE strategy after cleanup #");
      Serial.println(strategyCount);
      strategyCount++;
    }
  }
  
  if (random(0, 100) < 30) { // Higher save frequency for SPEEDIE
    saveStrategiesToEEPROM();
  }
}

LearnedStrategy* getBestStrategy(int currentDistance) {
  if (strategyCount == 0) return nullptr;
  
  int bestIndex = -1;
  float bestScore = -1.0;
  
  // SPEEDIE considers both success rate and speed
  for (int i = 0; i < strategyCount; i++) {
    if (abs(strategyLibrary[i].contextDistance - currentDistance) < 80) {
      float successScore = strategyLibrary[i].successRate;
      float speedScore = 1.0 / max(strategyLibrary[i].avgCompletionTime, 100.0f); // Favor faster strategies
      float combinedScore = (successScore * 0.7) + (speedScore * 0.3);
      
      if (combinedScore > bestScore && strategyLibrary[i].timesUsed >= 1) { // Lower threshold
        bestScore = combinedScore;
        bestIndex = i;
      }
    }
  }
  
  if (bestIndex >= 0) {
    Serial.print("⚡ Using fast SPEEDIE strategy #");
    Serial.print(bestIndex);
    Serial.print(" (");
    Serial.print(strategyLibrary[bestIndex].successRate * 100);
    Serial.print("% success, ");
    Serial.print(strategyLibrary[bestIndex].avgCompletionTime);
    Serial.println("ms avg)");
    return &strategyLibrary[bestIndex];
  }
  
  return nullptr;
}

// ═══════════════════════════════════════════════════════════
// 🚗 SPEEDIE MOTOR CONTROL (HIGH-PERFORMANCE)
// ═══════════════════════════════════════════════════════════

void stopMotorsBrake() {
  ledcWrite(PWM_CHANNEL_LEFT1, 255);
  ledcWrite(PWM_CHANNEL_LEFT2, 255);
  ledcWrite(PWM_CHANNEL_RIGHT1, 255);
  ledcWrite(PWM_CHANNEL_RIGHT2, 255);
}

void stopMotorsCoast() {
  ledcWrite(PWM_CHANNEL_LEFT1, 0);
  ledcWrite(PWM_CHANNEL_LEFT2, 0);
  ledcWrite(PWM_CHANNEL_RIGHT1, 0);
  ledcWrite(PWM_CHANNEL_RIGHT2, 0);
}

void stopMotors() {
  stopMotorsBrake(); // SPEEDIE uses brake for quicker stops
}

void moveForward() {
  ledcWrite(PWM_CHANNEL_LEFT1, currentGenome.motorSpeed);
  ledcWrite(PWM_CHANNEL_LEFT2, 0);
  ledcWrite(PWM_CHANNEL_RIGHT1, currentGenome.motorSpeed);
  ledcWrite(PWM_CHANNEL_RIGHT2, 0);
  lastActivityTime = millis();
}

void moveBackward() {
  ledcWrite(PWM_CHANNEL_LEFT1, 0);
  ledcWrite(PWM_CHANNEL_LEFT2, currentGenome.motorSpeed);
  ledcWrite(PWM_CHANNEL_RIGHT1, 0);
  ledcWrite(PWM_CHANNEL_RIGHT2, currentGenome.motorSpeed);
  lastActivityTime = millis();
}

void turnLeft() {
  ledcWrite(PWM_CHANNEL_LEFT1, 0);
  ledcWrite(PWM_CHANNEL_LEFT2, currentGenome.corneringSpeed);
  ledcWrite(PWM_CHANNEL_RIGHT1, currentGenome.corneringSpeed);
  ledcWrite(PWM_CHANNEL_RIGHT2, 0);
  lastActivityTime = millis();
}

void turnRight() {
  ledcWrite(PWM_CHANNEL_LEFT1, currentGenome.corneringSpeed);
  ledcWrite(PWM_CHANNEL_LEFT2, 0);
  ledcWrite(PWM_CHANNEL_RIGHT1, 0);
  ledcWrite(PWM_CHANNEL_RIGHT2, currentGenome.corneringSpeed);
  lastActivityTime = millis();
}

// SPEEDIE progressive acceleration
void accelerateForward(int targetSpeed, int acceleration) {
  static int currentSpeed = 0;
  
  if (currentSpeed < targetSpeed) {
    currentSpeed = min(currentSpeed + acceleration, targetSpeed);
  } else if (currentSpeed > targetSpeed) {
    currentSpeed = max(currentSpeed - acceleration, targetSpeed);
  }
  
  ledcWrite(PWM_CHANNEL_LEFT1, currentSpeed);
  ledcWrite(PWM_CHANNEL_LEFT2, 0);
  ledcWrite(PWM_CHANNEL_RIGHT1, currentSpeed);
  ledcWrite(PWM_CHANNEL_RIGHT2, 0);
  lastActivityTime = millis();
}

// ═══════════════════════════════════════════════════════════
// 👁️ SPEEDIE SENSOR READING (DUAL-SENSOR FUSION)
// ═══════════════════════════════════════════════════════════

int median(int arr[], int n) {
  for (int i = 0; i < n-1; i++) {
    for (int j = 0; j < n-i-1; j++) {
      if (arr[j] > arr[j+1]) {
        int temp = arr[j];
        arr[j] = arr[j+1];
        arr[j+1] = temp;
      }
    }
  }
  return arr[n/2];
}

int readUltrasonicDistance() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  
  unsigned long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (duration == 0) return SENSOR_ERROR_VALUE;
  
  int distance = duration * 0.034 / 2; // Convert to mm
  return distance * 10; // Convert cm to mm for consistency
}

void updateIMU() {
  sensors_event_t a, g, temp;
  if (mpu.getEvent(&a, &g, &temp)) {
    // Simple gyro integration for heading
    currentHeading += g.gyro.z * currentGenome.gyroSensitivity * 0.1; // Rough integration
    
    // Keep heading in 0-360 range
    while (currentHeading >= 360.0) currentHeading -= 360.0;
    while (currentHeading < 0.0) currentHeading += 360.0;
  }
}

int readDistanceFused() {
  // SPEEDIE uses ultrasonic primarily, IMU for confirmation
  int ultrasonicReading = readUltrasonicDistance();
  
  updateIMU();
  
  if (ultrasonicReading == SENSOR_ERROR_VALUE) {
    consecutiveSensorErrors++;
    if (consecutiveSensorErrors >= MAX_CONSECUTIVE_ERRORS) {
      Serial.println("⚡ SPEEDIE sensor recovery...");
      stopMotors();
      delay(50); // Shorter delay for SPEEDIE
    }
    return SENSOR_ERROR_VALUE;
  }
  
  consecutiveSensorErrors = 0;
  return ultrasonicReading;
}

int readDistance() {
  // Multiple sensor readings for reliability (but faster for SPEEDIE)
  int distances[2]; // Only 2 readings for speed
  for(int i = 0; i < 2; i++) {
    distances[i] = readDistanceFused();
    delay(5); // Shorter delay
  }
  
  if (distances[0] == SENSOR_ERROR_VALUE && distances[1] == SENSOR_ERROR_VALUE) {
    return SENSOR_ERROR_VALUE;
  } else if (distances[0] == SENSOR_ERROR_VALUE) {
    return distances[1];
  } else if (distances[1] == SENSOR_ERROR_VALUE) {
    return distances[0];
  } else {
    return (distances[0] + distances[1]) / 2; // Simple average for speed
  }
}

// ═══════════════════════════════════════════════════════════
// 😴 SPEEDIE SLEEP MANAGEMENT (LIGHTER SLEEP)
// ═══════════════════════════════════════════════════════════

void checkSleepTimeout() {
  if (isAwake && (millis() - lastActivityTime > SLEEP_TIMEOUT_MS)) {
    Serial.println("\n⚡ SPEEDIE entering light sleep mode...");
    isAwake = false;
    trappedAttempts = 0;
    
    saveGenomeToEEPROM();
    saveStrategiesToEEPROM();
    saveMetricsToEEPROM();
    
    stopMotorsCoast();
  }
}

// ═══════════════════════════════════════════════════════════
// 🆘 SPEEDIE AGGRESSIVE ESCAPE (ULTRA-FAST)
// ═══════════════════════════════════════════════════════════

void aggressiveEscape() {
  Serial.println("\n⚡ === SPEEDIE TRAPPED! ULTRA-FAST ESCAPE ===");
  metrics.timesTrapped++;
  
  expressState(2, -95); // Very negative
  
  unsigned long escapeStart = millis();
  
  int backupTime = currentGenome.backupDuration * currentGenome.aggressiveBackupMultiplier;
  int spinTime = (currentGenome.spinDegreesWhenTrapped * currentGenome.turnDuration) / 180;
  
  // SPEEDIE's ultra-fast escape sequence
  Serial.print("⚡ Fast backing up for ");
  Serial.print(backupTime / 2); // Half time for speed
  Serial.println("ms");
  moveBackward();
  delay(backupTime / 2);
  stopMotors();
  delay(100); // Shorter pause
  
  Serial.print("⚡ Rapid spinning ");
  Serial.print(currentGenome.spinDegreesWhenTrapped);
  Serial.println(" degrees");
  turnRight();
  delay(spinTime / 2); // Half time for speed
  stopMotors();
  delay(100);
  
  Serial.println("⚡ Power charging forward!");
  accelerateForward(255, currentGenome.maxAcceleration);
  delay(800); // Shorter charge
  stopMotors();
  delay(100);
  
  unsigned long escapeTime = millis() - escapeStart;
  metrics.averageEscapeTime = (metrics.averageEscapeTime + escapeTime) / 2.0;
  
  int checkDistance = readDistance();
  if (checkDistance > currentGenome.clearThreshold) {
    Serial.println("⚡ SPEEDIE escape successful!");
    metrics.trapEscapes++;
    trappedAttempts = 0;
    
    expressState(1, 85); // Very positive
  } else {
    Serial.println("⚡ Still trapped, SPEEDIE will retry");
    expressState(2, -85);
  }
}

// ═══════════════════════════════════════════════════════════
// 🎯 SPEEDIE OBSTACLE AVOIDANCE (SPEED-OPTIMIZED)
// ═══════════════════════════════════════════════════════════

void handleObstacle() {
  if (isAvoiding) return;
  
  isAvoiding = true;
  metrics.obstaclesEncountered++;
  unsigned long obstacleStartTime = millis();
  
  int initialDistance = readDistance();
  Serial.println("\n⚡ === SPEEDIE OBSTACLE DETECTED ===");
  Serial.print("Distance: ");
  Serial.println(initialDistance);
  
  expressState(0, -40);
  
  LearnedStrategy* learnedMove = getBestStrategy(initialDistance);
  bool useLearnedStrategy = (learnedMove != nullptr && random(0, 100) < 85); // Higher confidence
  
  bool succeeded = false;
  int backupTime, turnTime, direction;
  
  if (useLearnedStrategy) {
    Serial.println("⚡ Applying fast learned strategy...");
    
    expressState(1, 30);
    
    backupTime = learnedMove->backupTime / 2; // Faster execution
    turnTime = learnedMove->turnTime / 2;
    direction = learnedMove->turnDirection;
    
    stopMotors();
    delay(50); // Minimal pause
    
    moveBackward();
    delay(backupTime);
    stopMotors();
    delay(100);
    
    if (direction == 0) {
      turnLeft();
    } else {
      turnRight();
    }
    delay(turnTime);
    stopMotors();
    delay(currentGenome.scanDelay);
    
    int finalCheck = readDistance();
    if (finalCheck > currentGenome.clearThreshold || finalCheck == SENSOR_ERROR_VALUE) {
      Serial.println("⚡ Fast strategy worked!");
      accelerateForward(currentGenome.motorSpeed, currentGenome.maxAcceleration);
      delay(400);
      succeeded = true;
      metrics.obstaclesCleared++;
      
      unsigned long completionTime = millis() - obstacleStartTime;
      if (completionTime < metrics.fastestObstacleTime) {
        metrics.fastestObstacleTime = completionTime;
        Serial.print("⚡ NEW SPEED RECORD: ");
        Serial.println(completionTime);
      }
      
      expressState(1, 75);
    } else {
      Serial.println("❌ Fast strategy failed");
      trappedAttempts++;
      expressState(0, -60);
    }
    
    learnStrategy(initialDistance, direction, backupTime * 2, turnTime * 2, succeeded, millis() - obstacleStartTime);
    
  } else {
    Serial.println("⚡ SPEEDIE exploring new fast approach...");
    
    expressState(3, 10);
    
    stopMotors();
    delay(50);
    
    backupTime = currentGenome.backupDuration / 2; // Faster backup
    moveBackward();
    delay(backupTime);
    stopMotors();
    delay(100);
    
    // Quick scan (fewer positions for speed)
    bool clearPathFound = false;
    int bestDirection = 0;
    int bestDistance = 0;
    
    // Quick left scan
    turnLeft();
    delay(currentGenome.turnDuration / 2);
    stopMotors();
    delay(currentGenome.scanDelay / 2);
    
    int leftDistance = readDistance();
    if (leftDistance == SENSOR_ERROR_VALUE) leftDistance = 0;
    
    if (leftDistance > currentGenome.clearThreshold && leftDistance > bestDistance) {
      bestDistance = leftDistance;
      bestDirection = 0;
      clearPathFound = true;
    }
    
    // Return to center quickly
    turnRight();
    delay(currentGenome.turnDuration);
    stopMotors();
    delay(currentGenome.scanDelay / 2);
    
    // Quick right scan if needed
    if (!clearPathFound) {
      turnRight();
      delay(currentGenome.turnDuration / 2);
      stopMotors();
      delay(currentGenome.scanDelay / 2);
      
      int rightDistance = readDistance();
      if (rightDistance == SENSOR_ERROR_VALUE) rightDistance = 0;
      
      if (rightDistance > currentGenome.clearThreshold && rightDistance > bestDistance) {
        bestDistance = rightDistance;
        bestDirection = 1;
        clearPathFound = true;
      }
      
      // Return to center
      turnLeft();
      delay(currentGenome.turnDuration);
      stopMotors();
      delay(currentGenome.scanDelay / 2);
    }
    
    // Execute maneuver
    if (clearPathFound) {
      expressState(3, 60);
      
      turnTime = currentGenome.turnDuration;
      direction = bestDirection;
      
      if (bestDirection == 0) {
        turnLeft();
      } else {
        turnRight();
      }
      delay(turnTime);
      stopMotors();
      delay(50);
      
      int finalCheck = readDistance();
      if (finalCheck > currentGenome.clearThreshold || finalCheck == SENSOR_ERROR_VALUE) {
        accelerateForward(currentGenome.motorSpeed, currentGenome.maxAcceleration);
        delay(400);
        succeeded = true;
        metrics.obstaclesCleared++;
        trappedAttempts = 0;
        
        unsigned long completionTime = millis() - obstacleStartTime;
        if (completionTime < metrics.fastestObstacleTime) {
          metrics.fastestObstacleTime = completionTime;
          Serial.print("⚡ NEW EXPLORATION SPEED RECORD: ");
          Serial.println(completionTime);
        }
        
        expressState(1, 80);
      } else {
        trappedAttempts++;
        expressState(0, -50);
      }
      
      learnStrategy(initialDistance, direction, backupTime * 2, turnTime, succeeded, millis() - obstacleStartTime);
      
    } else {
      trappedAttempts++;
      expressState(2, -70);
      
      if (trappedAttempts >= MAX_TRAPPED_ATTEMPTS) {
        aggressiveEscape();
      } else {
        Serial.println("⚡ No clear path, quick backup");
        moveBackward();
        delay(500);
        stopMotors();
      }
    }
  }
  
  isAvoiding = false;
  
  if (random(0, 100) < 15) { // Higher evolution chance for SPEEDIE
    evolutionCycle();
  }
}

// ═══════════════════════════════════════════════════════════
// 🎬 SPEEDIE SETUP
// ═══════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  ⚡ SPEEDIE: HIGH-PERFORMANCE BOT ⚡  ║");
  Serial.println("║     WITH ULTRA-FAST EVOLUTION 🧬      ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  EEPROM.begin(EEPROM_SIZE);
  
  Serial.println("⚡ Loading SPEEDIE persistent memory...");
  loadGenomeFromEEPROM();
  loadStrategiesFromEEPROM();
  loadMetricsFromEEPROM();
  loadVocabularyFromEEPROM();
  
  if (vocabularySize == 0) {
    initializeDefaultVocabulary();
  }
  
  Serial.println("ℹ️ SPEEDIE uses LED-only communication (no buzzer for max speed)");
  
  // Initialize sensors (NO motion sensor for SPEEDIE)
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  
  Wire.begin();
  if (!mpu.begin()) {
    Serial.println("❌ Failed to initialize MPU6050");
    while(1);
  }
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  Serial.println("⚡ SPEEDIE sensors initialized");
  
  // Initialize motors (high-performance setup)
  ledcSetup(PWM_CHANNEL_LEFT1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_LEFT2, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_RIGHT1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_RIGHT2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LEFT_MOTOR_PIN1, PWM_CHANNEL_LEFT1);
  ledcAttachPin(LEFT_MOTOR_PIN2, PWM_CHANNEL_LEFT2);
  ledcAttachPin(RIGHT_MOTOR_PIN1, PWM_CHANNEL_RIGHT1);
  ledcAttachPin(RIGHT_MOTOR_PIN2, PWM_CHANNEL_RIGHT2);

  // Initialize Red/Green LED PWM channels
  ledcSetup(PWM_CH_L_R, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_L_G, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_R_R, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_R_G, PWM_FREQ, PWM_RESOLUTION);

  // Attach pins to LED PWM channels
  ledcAttachPin(LEFT_LED_R_PIN, PWM_CH_L_R);
  ledcAttachPin(LEFT_LED_G_PIN, PWM_CH_L_G);
  ledcAttachPin(RIGHT_LED_R_PIN, PWM_CH_R_R);
  ledcAttachPin(RIGHT_LED_G_PIN, PWM_CH_R_G);

  Serial.println("⚡ SPEEDIE high-performance LEDs initialized");
  Serial.println("⚡ SPEEDIE motors initialized");
  
  metrics.missionStartTime = millis();
  
  Serial.println("\n⚡ Current SPEEDIE Evolution Status:");
  Serial.print("  Generation: ");
  Serial.println(currentGenome.generation);
  Serial.print("  Fitness Score: ");
  Serial.println(currentGenome.fitnessScore);
  Serial.print("  Strategies Learned: ");
  Serial.println(strategyCount);
  Serial.print("  Vocabulary Size: ");
  Serial.println(vocabularySize);
  Serial.print("  Fastest Obstacle Time: ");
  Serial.println(metrics.fastestObstacleTime);
  
  Serial.println("\n⚡ Initializing SPEEDIE communication protocol...");
  delay(300);
  expressState(4, 50);
  
  Serial.println("\n⚡ SPEEDIE ready for immediate high-speed evolution!\n");
}

// ═══════════════════════════════════════════════════════════
// 🔄 SPEEDIE MAIN LOOP (HIGH-SPEED OPERATION)
// ═══════════════════════════════════════════════════════════

void loop() {
  checkSleepTimeout();
  
  // SPEEDIE uses timer-based activation (no motion sensor for max speed)
  if (!isAwake) {
    Serial.println("\n⚡ SPEEDIE AUTO-ACTIVATION! High-speed start...");
    Serial.println("⚡ Beginning ultra-fast evolution...\n");
    
    expressState(4, 30);
    
    isAwake = true;
    lastActivityTime = millis();
    trappedAttempts = 0;
  }

  if (isAwake && !isAvoiding) {
    int distance = readDistance();
    
    if (distance == SENSOR_ERROR_VALUE) {
      accelerateForward(currentGenome.motorSpeed / 2, currentGenome.maxAcceleration / 2); // Cautious speed
    } else {
      if (distance < currentGenome.obstacleThreshold) {
        handleObstacle();
      } else {
        accelerateForward(currentGenome.motorSpeed, currentGenome.maxAcceleration);
        
        if (random(0, 2000) < 5) { // Less frequent for speed
          expressState(3, 40);
        }
      }
    }
    
    evolutionCycle();
    
  } else if (!isAwake) {
    stopMotorsCoast();
    // Turn off all LEDs (255 = OFF for common anode)
    ledcWrite(PWM_CH_L_R, 255); ledcWrite(PWM_CH_L_G, 255);
    ledcWrite(PWM_CH_R_R, 255); ledcWrite(PWM_CH_R_G, 255);
    delay(50); // Shorter sleep delays
  } else {
    stopMotors();
    delay(50);
  }
}