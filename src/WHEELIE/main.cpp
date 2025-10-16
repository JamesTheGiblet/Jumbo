#include <Arduino.h>
#include <Adafruit_VL53L0X.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "emergent_signal.h"

// Include emergent signal and context detection implementation
#include "../emergent_signal.cpp"
#include "../context_detection.cpp"

// == LED PINS (4-PIN RGB LED) ==
// Left LED
const int LEFT_LED_R_PIN = 2;
const int LEFT_LED_G_PIN = 4;
const int LEFT_LED_B_PIN = 5;
// Right LED
const int RIGHT_LED_R_PIN = 12;
const int RIGHT_LED_G_PIN = 14;
const int RIGHT_LED_B_PIN = 15;

// PWM Channels for LEDs
const int PWM_CH_L_R = 4;
const int PWM_CH_L_G = 5;
const int PWM_CH_L_B = 6;
const int PWM_CH_R_R = 7;
const int PWM_CH_R_G = 8;
const int PWM_CH_R_B = 9;

// == SENSOR PIN ==
const int MOTION_SENSOR_PIN = 27;
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

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

// Buzzer pin for audio signals (set to -1 if no buzzer)
const int BUZZER_PIN = -1;  // No buzzer on WHEELIE by default
bool hasBuzzer = false;

// ═══════════════════════════════════════════════════════════
// 🧬 EVOLUTIONARY PARAMETERS - THE ROBOT'S GENOME
// ═══════════════════════════════════════════════════════════
struct EvolvingGenome {
  // Movement parameters (mutable)
  int motorSpeed = 200;
  int turnSpeed = 150;
  int backupDuration = 600;
  int turnDuration = 350;
  
  // Perception parameters (mutable)
  int obstacleThreshold = 200;
  int clearThreshold = 300;
  int scanDelay = 300;
  
  // Strategy parameters (mutable)
  int aggressiveBackupMultiplier = 2; // How much more to back up when trapped
  int spinDegreesWhenTrapped = 180;   // How far to spin when trapped
  
  // Evolution metadata
  unsigned long successCount = 0;
  unsigned long failureCount = 0;
  float fitnessScore = 0.0;
  unsigned long generation = 0;
};

// ═══════════════════════════════════════════════════════════
// 🌊 EMERGENT COMMUNICATION SYSTEM
// ═══════════════════════════════════════════════════════════

// Global emergent signal generator
EmergentSignalGenerator signalGenerator;

// External variables for context detection (defined in context_detection.cpp)
int distance_cm = 0;
bool motionDetected = false;
bool isMoving = false;
bool taskInProgress = false;
bool taskSuccessful = false;
unsigned long lastPeerContact = 0;

#define WHEELIE_BOT  // Enable WHEELIE-specific context detection

// ═══════════════════════════════════════════════════════════
// 🧠 LEARNED STRATEGIES - THE ROBOT'S MEMORY
// ═══════════════════════════════════════════════════════════
struct LearnedStrategy {
  int contextDistance;        // Distance when this worked
  int turnDirection;          // 0=left, 1=right
  int backupTime;            // How long to back up
  int turnTime;              // How long to turn
  float successRate;         // How often this works (0.0-1.0)
  unsigned long timesUsed;   // How many times tried
  unsigned long timesSucceeded; // How many times succeeded
};

const int MAX_STRATEGIES = 20;
LearnedStrategy strategyLibrary[MAX_STRATEGIES];
int strategyCount = 0;

// ═══════════════════════════════════════════════════════════
// 📡 EMERGENT ESP-NOW COMMUNICATION SYSTEM
// ═══════════════════════════════════════════════════════════

// Bot type definitions for emergent system
enum BotType {
  BOT_SPEEDIE = 0,
  BOT_WHEELIE = 1
};

// Simple peer structure for emergent communication
struct SwarmPeer {
  uint8_t macAddress[6];
  bool isActive = false;
  unsigned long lastSeen = 0;
  BotType botType = BOT_WHEELIE;
};

const int MAX_SWARM_PEERS = 8;
SwarmPeer swarmPeers[MAX_SWARM_PEERS];

// Simple peer tracking for emergent system
int activePeerCount = 0;
BotType myBotType = BOT_WHEELIE;
uint8_t sequenceNumber = 0;

// Message timers
unsigned long lastCommunicationTime = 0;

// Swarm coordination state
bool hasLeader = false;
uint8_t leaderMac[6] = {0};

// ESP-NOW buffers for emergent communication
EmergentMessage outgoingEmergentMessage;
EmergentMessage incomingEmergentMessage;

// ═══════════════════════════════════════════════════════════
// 🌱 EVOLUTION STATE
// ═══════════════════════════════════════════════════════════
EvolvingGenome currentGenome;
EvolvingGenome previousGenome;
bool evolutionEnabled = true;
unsigned long lastEvolutionTime = 0;
const unsigned long EVOLUTION_INTERVAL_BASE = 60000; // Base: 60 seconds
unsigned long EVOLUTION_INTERVAL = EVOLUTION_INTERVAL_BASE; // Dynamic interval

// ═══════════════════════════════════════════════════════════
// 📊 PERFORMANCE TRACKING
// ═══════════════════════════════════════════════════════════
struct PerformanceMetrics {
  unsigned long obstaclesEncountered = 0;
  unsigned long obstaclesCleared = 0;
  unsigned long timesTrapped = 0;
  unsigned long trapEscapes = 0;
  unsigned long totalDistanceTraveled = 0;
  unsigned long missionStartTime = 0;
  float averageEscapeTime = 0.0;
};

PerformanceMetrics metrics;
unsigned long currentObstacleStartTime = 0;

// ═══════════════════════════════════════════════════════════
// 💾 EEPROM PERSISTENCE
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
const unsigned long SLEEP_TIMEOUT_MS = 30000;
const int SENSOR_ERROR_VALUE = 9999;
int consecutiveSensorErrors = 0;
const int MAX_CONSECUTIVE_ERRORS = 5;
int trappedAttempts = 0;
const int MAX_TRAPPED_ATTEMPTS = 3;

// ═══════════════════════════════════════════════════════════
// � FUNCTION FORWARD DECLARATIONS  
// ═══════════════════════════════════════════════════════════
// Emergent communication functions
void communicateCurrentState();
void updateSignalFeedback(bool wasSuccessful);
void onEmergentMessageReceived(const uint8_t *mac, const uint8_t *incomingData, int len);
void displaySignalOnLEDs(SignalWord* signal, EnvironmentalContext context, EmotionalState emotion);
int findPeer(const uint8_t* mac);

// ═══════════════════════════════════════════════════════════
// �🗣️ EMERGENT LANGUAGE & PERSISTENCE FUNCTIONS
//  PERSISTENT MEMORY FUNCTIONS
// ═══════════════════════════════════════════════════════════

void saveGenomeToEEPROM() {
  EEPROM.put(GENOME_ADDRESS, currentGenome);
  EEPROM.commit();
  Serial.println("💾 Genome saved to persistent memory");
}

void loadGenomeFromEEPROM() {
  EEPROM.get(GENOME_ADDRESS, currentGenome);
  Serial.println("📖 Genome loaded from memory");
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
  Serial.println(" strategies to memory");
}

void loadStrategiesFromEEPROM() {
  EEPROM.get(STRATEGIES_ADDRESS + (MAX_STRATEGIES * sizeof(LearnedStrategy)), strategyCount);
  if (strategyCount > MAX_STRATEGIES) strategyCount = 0; // Corrupted data
  
  for (int i = 0; i < strategyCount; i++) {
    EEPROM.get(STRATEGIES_ADDRESS + (i * sizeof(LearnedStrategy)), strategyLibrary[i]);
  }
  Serial.print("📖 Loaded ");
  Serial.print(strategyCount);
  Serial.println(" strategies from memory");
}

void saveMetricsToEEPROM() {
  EEPROM.put(METRICS_ADDRESS, metrics);
  EEPROM.commit();
}

void loadMetricsFromEEPROM() {
  EEPROM.get(METRICS_ADDRESS, metrics);
}

// All old vocabulary functions removed - using emergent signal system instead

// ═══════════════════════════════════════════════════════════
// 🧬 MUTATION FUNCTIONS - HOW THE ROBOT EVOLVES
// ═══════════════════════════════════════════════════════════

void mutateGenome() {
  Serial.println("\n🧬 === MUTATION EVENT ===");
  previousGenome = currentGenome; // Save backup
  
  // Randomly mutate 1-3 parameters
  int mutationCount = random(1, 4);
  
  for (int i = 0; i < mutationCount; i++) {
    int mutationType = random(0, 9);
    
    switch(mutationType) {
      case 0: // Motor speed
        currentGenome.motorSpeed += random(-30, 31);
        currentGenome.motorSpeed = constrain(currentGenome.motorSpeed, 100, 255);
        Serial.print("  ↗️ Motor speed → ");
        Serial.println(currentGenome.motorSpeed);
        break;
        
      case 1: // Turn speed
        currentGenome.turnSpeed += random(-20, 21);
        currentGenome.turnSpeed = constrain(currentGenome.turnSpeed, 80, 200);
        Serial.print("  🔄 Turn speed → ");
        Serial.println(currentGenome.turnSpeed);
        break;
        
      case 2: // Backup duration
        currentGenome.backupDuration += random(-100, 101);
        currentGenome.backupDuration = constrain(currentGenome.backupDuration, 300, 1200);
        Serial.print("  ⏪ Backup time → ");
        Serial.println(currentGenome.backupDuration);
        break;
        
      case 3: // Turn duration
        currentGenome.turnDuration += random(-50, 51);
        currentGenome.turnDuration = constrain(currentGenome.turnDuration, 200, 600);
        Serial.print("  🔀 Turn time → ");
        Serial.println(currentGenome.turnDuration);
        break;
        
      case 4: // Obstacle threshold
        currentGenome.obstacleThreshold += random(-30, 31);
        currentGenome.obstacleThreshold = constrain(currentGenome.obstacleThreshold, 100, 400);
        Serial.print("  ⚠️ Obstacle threshold → ");
        Serial.println(currentGenome.obstacleThreshold);
        break;
        
      case 5: // Clear threshold
        currentGenome.clearThreshold += random(-40, 41);
        currentGenome.clearThreshold = constrain(currentGenome.clearThreshold, 200, 500);
        Serial.print("  ✅ Clear threshold → ");
        Serial.println(currentGenome.clearThreshold);
        break;
        
      case 6: // Scan delay
        currentGenome.scanDelay += random(-50, 51);
        currentGenome.scanDelay = constrain(currentGenome.scanDelay, 100, 500);
        Serial.print("  👁️ Scan delay → ");
        Serial.println(currentGenome.scanDelay);
        break;
        
      case 7: // Aggressive backup multiplier
        currentGenome.aggressiveBackupMultiplier = random(2, 5);
        Serial.print("  💪 Aggressive backup → ");
        Serial.println(currentGenome.aggressiveBackupMultiplier);
        break;
        
      case 8: // Spin degrees when trapped
        currentGenome.spinDegreesWhenTrapped = random(90, 271);
        Serial.print("  🌀 Trapped spin → ");
        Serial.println(currentGenome.spinDegreesWhenTrapped);
        break;
    }
  }
  
  currentGenome.generation++;
  Serial.print("📈 Generation: ");
  Serial.println(currentGenome.generation);
}

// ═══════════════════════════════════════════════════════════
// 📊 FITNESS CALCULATION - HOW WELL IS THE ROBOT DOING?
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
  
  // Fitness is weighted combination of success metrics
  currentGenome.fitnessScore = (successRate * 0.6) + (escapeRate * 0.4);
  
  Serial.println("\n📊 === FITNESS REPORT ===");
  Serial.print("Success Rate: ");
  Serial.print(successRate * 100);
  Serial.println("%");
  Serial.print("Escape Rate: ");
  Serial.print(escapeRate * 100);
  Serial.println("%");
  Serial.print("Overall Fitness: ");
  Serial.println(currentGenome.fitnessScore);
}

// ═══════════════════════════════════════════════════════════
// 🧠 MEMORY & CONSTRAINT MANAGEMENT
// ═══════════════════════════════════════════════════════════

void pruneWeakStrategies() {
  // Remove strategies with poor success rates
  for (int i = strategyCount - 1; i >= 0; i--) {
    if (strategyLibrary[i].timesUsed >= 3 && 
        strategyLibrary[i].successRate < 0.3) {
      Serial.print("🧹 Pruning weak strategy #");
      Serial.println(i);
      
      // Shift remaining strategies down
      for (int j = i; j < strategyCount - 1; j++) {
        strategyLibrary[j] = strategyLibrary[j + 1];
      }
      strategyCount--;
    }
  }
}

void compactStrategyArray() {
  // Remove any gaps in the strategy array and ensure contiguous storage
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
  Serial.print("🗜️ Compacted strategy array to ");
  Serial.print(strategyCount);
  Serial.println(" strategies");
}

void applyEvolutionaryConstraints() {
  // Ensure parameters stay within reasonable bounds
  currentGenome.motorSpeed = constrain(currentGenome.motorSpeed, 120, 255);
  currentGenome.turnSpeed = constrain(currentGenome.turnSpeed, 100, 200);
  currentGenome.obstacleThreshold = constrain(currentGenome.obstacleThreshold, 150, 350);
  
  // Ensure logical relationships
  if (currentGenome.clearThreshold <= currentGenome.obstacleThreshold) {
    currentGenome.clearThreshold = currentGenome.obstacleThreshold + 50;
  }
}

// ═══════════════════════════════════════════════════════════
// 🔄 EVOLUTION ENGINE - SELECTION AND ADAPTATION
// ═══════════════════════════════════════════════════════════

void evolutionCycle() {
  if (!evolutionEnabled) return;
  
  unsigned long now = millis();
  
  // Consider adaptive evolution intervals
  // Higher fitness = longer intervals (more exploration when doing well)
  // Lower fitness = shorter intervals (more mutation when struggling)
  EVOLUTION_INTERVAL = EVOLUTION_INTERVAL_BASE * (0.5 + (currentGenome.fitnessScore * 1.5));
  EVOLUTION_INTERVAL = constrain(EVOLUTION_INTERVAL, 30000, 180000); // 30s to 3min range
  
  if (now - lastEvolutionTime < EVOLUTION_INTERVAL) return;
  
  lastEvolutionTime = now;
  
  Serial.println("\n\n🌟 ═══════════════════════════════════════");
  Serial.println("       EVOLUTION CYCLE TRIGGERED");
  Serial.println("═══════════════════════════════════════ 🌟\n");
  
  // Express excitement about evolving
  communicateCurrentState(); // Emergent signal based on environmental context
  
  // Calculate current fitness
  calculateFitness();
  
  // Compare to previous generation
  if (currentGenome.generation > 0) {
    if (currentGenome.fitnessScore >= previousGenome.fitnessScore) {
      // Keep mutation - it helped!
      Serial.println("✅ Mutation SUCCESSFUL - keeping changes");
      currentGenome.successCount++;
      
      // Express success!
      communicateCurrentState(); // Emergent success signal
      updateSignalFeedback(true);
      
      // Small additional mutation to continue improving
      if (random(0, 100) < 30) { // 30% chance
        Serial.println("🎲 Bonus mutation for successful genome");
        mutateGenome();
      }
    } else {
      // Revert mutation - it hurt performance
      Serial.println("❌ Mutation FAILED - reverting to previous genome");
      currentGenome = previousGenome;
      currentGenome.failureCount++;
      currentGenome.generation++; // Still increment generation
      
      // Express disappointment
      communicateCurrentState(); // Emergent signal for expected success
    }
  } else {
    // First generation - always mutate
    mutateGenome();
  }
  
  // Evolve emergent signal system alongside behavior
  signalGenerator.pruneUnusedSignals();
  
  // Save evolved genome to persistent memory
  saveGenomeToEEPROM();
  saveMetricsToEEPROM();
  
  // Add constraints and cleanup
  applyEvolutionaryConstraints();
  pruneWeakStrategies();
  
  // Print genome summary
  Serial.println("\n🧬 Current Genome:");
  Serial.print("  Motor Speed: ");
  Serial.println(currentGenome.motorSpeed);
  Serial.print("  Turn Speed: ");
  Serial.println(currentGenome.turnSpeed);
  Serial.print("  Obstacle Threshold: ");
  Serial.println(currentGenome.obstacleThreshold);
  Serial.print("  Signal Pool Size: ");
  Serial.println(signalGenerator.getVocabularySize());
  Serial.println("\n═══════════════════════════════════════\n");
}

// ═══════════════════════════════════════════════════════════
// 🎓 STRATEGY LEARNING - BUILDING A KNOWLEDGE BASE
// ═══════════════════════════════════════════════════════════

void learnStrategy(int distance, int direction, int backupTime, int turnTime, bool succeeded) {
  // Try to find existing similar strategy
  int similarIndex = -1;
  for (int i = 0; i < strategyCount; i++) {
    if (abs(strategyLibrary[i].contextDistance - distance) < 50 &&
        strategyLibrary[i].turnDirection == direction) {
      similarIndex = i;
      break;
    }
  }
  
  if (similarIndex >= 0) {
    // Update existing strategy
    strategyLibrary[similarIndex].timesUsed++;
    if (succeeded) {
      strategyLibrary[similarIndex].timesSucceeded++;
    }
    strategyLibrary[similarIndex].successRate = 
      (float)strategyLibrary[similarIndex].timesSucceeded / 
      (float)strategyLibrary[similarIndex].timesUsed;
    
    Serial.print("📚 Updated strategy #");
    Serial.print(similarIndex);
    Serial.print(" - Success rate: ");
    Serial.println(strategyLibrary[similarIndex].successRate * 100);
  } else if (strategyCount < MAX_STRATEGIES) {
    // Add new strategy
    strategyLibrary[strategyCount].contextDistance = distance;
    strategyLibrary[strategyCount].turnDirection = direction;
    strategyLibrary[strategyCount].backupTime = backupTime;
    strategyLibrary[strategyCount].turnTime = turnTime;
    strategyLibrary[strategyCount].timesUsed = 1;
    strategyLibrary[strategyCount].timesSucceeded = succeeded ? 1 : 0;
    strategyLibrary[strategyCount].successRate = succeeded ? 1.0 : 0.0;
    
    Serial.print("🆕 Learned new strategy #");
    Serial.println(strategyCount);
    strategyCount++;
  } else {
    // Strategy array is full - apply dynamic management
    Serial.println("📚 Strategy library full - applying smart management");
    pruneWeakStrategies();
    compactStrategyArray();
    
    // Try to add again if space was freed
    if (strategyCount < MAX_STRATEGIES) {
      strategyLibrary[strategyCount].contextDistance = distance;
      strategyLibrary[strategyCount].turnDirection = direction;
      strategyLibrary[strategyCount].backupTime = backupTime;
      strategyLibrary[strategyCount].turnTime = turnTime;
      strategyLibrary[strategyCount].timesUsed = 1;
      strategyLibrary[strategyCount].timesSucceeded = succeeded ? 1 : 0;
      strategyLibrary[strategyCount].successRate = succeeded ? 1.0 : 0.0;
      
      Serial.print("🆕 Added strategy after cleanup #");
      Serial.println(strategyCount);
      strategyCount++;
    } else {
      Serial.println("⚠️ Could not add strategy - all slots contain valuable data");
    }
  }
  
  // Periodically save strategies
  if (random(0, 100) < 20) { // 20% chance to save
    saveStrategiesToEEPROM();
  }
}

LearnedStrategy* getBestStrategy(int currentDistance) {
  if (strategyCount == 0) return nullptr;
  
  int bestIndex = -1;
  float bestScore = -1.0;
  
  // Find strategy with best success rate for similar distance
  for (int i = 0; i < strategyCount; i++) {
    if (abs(strategyLibrary[i].contextDistance - currentDistance) < 100) {
      float score = strategyLibrary[i].successRate;
      if (score > bestScore && strategyLibrary[i].timesUsed >= 2) {
        bestScore = score;
        bestIndex = i;
      }
    }
  }
  
  if (bestIndex >= 0) {
    Serial.print("🎯 Using learned strategy #");
    Serial.print(bestIndex);
    Serial.print(" (");
    Serial.print(strategyLibrary[bestIndex].successRate * 100);
    Serial.println("% success rate)");
    return &strategyLibrary[bestIndex];
  }
  
  return nullptr;
}

// ═══════════════════════════════════════════════════════════
// 🚗 MOTOR CONTROL
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
  stopMotorsBrake();
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
  ledcWrite(PWM_CHANNEL_LEFT2, currentGenome.turnSpeed);
  ledcWrite(PWM_CHANNEL_RIGHT1, currentGenome.turnSpeed);
  ledcWrite(PWM_CHANNEL_RIGHT2, 0);
  lastActivityTime = millis();
}

void turnRight() {
  ledcWrite(PWM_CHANNEL_LEFT1, currentGenome.turnSpeed);
  ledcWrite(PWM_CHANNEL_LEFT2, 0);
  ledcWrite(PWM_CHANNEL_RIGHT1, 0);
  ledcWrite(PWM_CHANNEL_RIGHT2, currentGenome.turnSpeed);
  lastActivityTime = millis();
}

// ═══════════════════════════════════════════════════════════
// 👁️ SENSOR READING
// ═══════════════════════════════════════════════════════════

// Helper function to calculate median of an array
int median(int arr[], int n) {
  // Simple bubble sort for small arrays
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

int readDistanceSingle() {
  VL53L0X_RangingMeasurementData_t measure;
  
  // Add timeout protection
  unsigned long startTime = millis();
  lox.rangingTest(&measure, false);
  
  if (measure.RangeStatus != 4) {
    consecutiveSensorErrors = 0;
    return measure.RangeMilliMeter;
  } else {
    consecutiveSensorErrors++;
    
    // If we get too many errors, try to reset the sensor
    if (consecutiveSensorErrors >= MAX_CONSECUTIVE_ERRORS) {
      Serial.println("🔄 Attempting sensor recovery...");
      stopMotors();
      delay(100);
      // You could add sensor reinitialization here if needed
    }
    
    return SENSOR_ERROR_VALUE;
  }
}

int readDistance() {
  // Multiple sensor readings for reliability
  int distances[3];
  for(int i = 0; i < 3; i++) {
    distances[i] = readDistanceSingle();
    delay(10);
  }
  int avgDistance = median(distances, 3);
  return avgDistance;
}

// ═══════════════════════════════════════════════════════════
// 😴 SLEEP MANAGEMENT
// ═══════════════════════════════════════════════════════════

void checkSleepTimeout() {
  if (isAwake && (millis() - lastActivityTime > SLEEP_TIMEOUT_MS)) {
    Serial.println("\n💤 Entering sleep mode (saving state...)");
    isAwake = false;
    trappedAttempts = 0;
    
    // Save state before sleep
    saveGenomeToEEPROM();
    saveStrategiesToEEPROM();
    saveMetricsToEEPROM();
    
    stopMotorsCoast();
    // digitalWrite(STATUS_LED_PIN, LOW); // Turn off LED when sleeping
  }
}

// ═══════════════════════════════════════════════════════════
// 🆘 AGGRESSIVE ESCAPE FOR TRAPPED SITUATIONS
// ═══════════════════════════════════════════════════════════

void aggressiveEscape() {
  Serial.println("\n🆘 === TRAPPED! AGGRESSIVE ESCAPE ===");
  metrics.timesTrapped++;
  
  // Express DISTRESS
  communicateCurrentState(); // Emergent distress signal for being trapped
  updateSignalFeedback(false);
  
  unsigned long escapeStart = millis();
  
  // Use evolved parameters
  int backupTime = currentGenome.backupDuration * currentGenome.aggressiveBackupMultiplier;
  int spinTime = (currentGenome.spinDegreesWhenTrapped * currentGenome.turnDuration) / 180;
  
  Serial.print("Backing up for ");
  Serial.print(backupTime);
  Serial.println("ms");
  moveBackward();
  delay(backupTime);
  stopMotors();
  delay(300);
  
  Serial.print("Spinning ");
  Serial.print(currentGenome.spinDegreesWhenTrapped);
  Serial.println(" degrees");
  turnRight();
  delay(spinTime);
  stopMotors();
  delay(300);
  
  Serial.println("Charging forward to escape");
  moveForward();
  delay(1200);
  stopMotors();
  delay(300);
  
  unsigned long escapeTime = millis() - escapeStart;
  metrics.averageEscapeTime = (metrics.averageEscapeTime + escapeTime) / 2.0;
  
  // Check if escape succeeded
  int checkDistance = readDistance();
  if (checkDistance > currentGenome.clearThreshold) {
    Serial.println("✅ Escape successful!");
    metrics.trapEscapes++;
    trappedAttempts = 0;
    
    // Express RELIEF/SUCCESS
    communicateCurrentState(); // Emergent signal for clearing obstacle
    updateSignalFeedback(true);
  } else {
    Serial.println("⚠️ Still trapped, will retry");
    
    // Express continued distress
    communicateCurrentState(); // Emergent signal for trapped state
  }
}

// ═══════════════════════════════════════════════════════════
// 🎯 INTELLIGENT OBSTACLE AVOIDANCE WITH LEARNING
// ═══════════════════════════════════════════════════════════

void handleObstacle() {
  if (isAvoiding) return;
  
  isAvoiding = true;
  metrics.obstaclesEncountered++;
  currentObstacleStartTime = millis();
  
  int initialDistance = readDistance();
  Serial.println("\n⚠️ === OBSTACLE DETECTED ===");
  Serial.print("Distance: ");
  Serial.println(initialDistance);
  
  // Express awareness of obstacle
  communicateCurrentState(); // Emergent signal for obstacle detection
  
  // // Blink the status LED to indicate thinking/avoiding
  // for(int i=0; i<3; i++) {
  //   digitalWrite(STATUS_LED_PIN, LOW);
  //   delay(100);
  //   digitalWrite(STATUS_LED_PIN, HIGH);
  //   delay(100);
  // }
  
  // Check if we have a learned strategy for this situation
  LearnedStrategy* learnedMove = getBestStrategy(initialDistance);
  bool useLearnedStrategy = (learnedMove != nullptr && random(0, 100) < 80); // 80% use learned, 20% explore
  
  bool succeeded = false;
  int backupTime, turnTime, direction;
  
  if (useLearnedStrategy) {
    // USE LEARNED STRATEGY
    Serial.println("🎓 Applying learned strategy...");
    
    // Express confidence in using known strategy
    communicateCurrentState(); // Emergent signal for applying strategy
    
    backupTime = learnedMove->backupTime;
    turnTime = learnedMove->turnTime;
    direction = learnedMove->turnDirection;
    
    stopMotors();
    delay(200);
    
    moveBackward();
    delay(backupTime);
    stopMotors();
    delay(300);
    
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
      Serial.println("✅ Learned strategy worked!");
      moveForward();
      delay(800);
      succeeded = true;
      metrics.obstaclesCleared++;
      
      // Express success!
      communicateCurrentState(); // Emergent signal for success
    } else {
      Serial.println("❌ Learned strategy failed");
      trappedAttempts++;
      
      // Express disappointment
      communicateCurrentState(); // Emergent signal for strategy failure
    }
    
    learnStrategy(initialDistance, direction, backupTime, turnTime, succeeded);
    
  } else {
    // EXPLORE NEW STRATEGY
    Serial.println("🔬 Exploring new approach...");
    
    // Express curiosity/uncertainty
    communicateCurrentState(); // Emergent signal for exploration
    
    stopMotors();
    delay(200);
    
    backupTime = currentGenome.backupDuration;
    moveBackward();
    delay(backupTime);
    stopMotors();
    delay(300);
    
    // Scan for clear path
    bool clearPathFound = false;
    int bestDirection = 0;
    int bestDistance = 0;
    
    // Scan left
    for (int scan = 1; scan <= 3; scan++) {
      turnLeft();
      delay(currentGenome.turnDuration);
      stopMotors();
      delay(currentGenome.scanDelay);
      
      int distance = readDistance();
      if (distance == SENSOR_ERROR_VALUE) distance = 0;
      
      if (distance > currentGenome.clearThreshold && distance > bestDistance) {
        bestDistance = distance;
        bestDirection = 0;
        clearPathFound = true;
      }
    }
    
    // Return to center
    turnRight();
    delay(currentGenome.turnDuration * 3);
    stopMotors();
    delay(currentGenome.scanDelay);
    
    // Scan right if needed
    if (!clearPathFound) {
      for (int scan = 1; scan <= 3; scan++) {
        turnRight();
        delay(currentGenome.turnDuration);
        stopMotors();
        delay(currentGenome.scanDelay);
        
        int distance = readDistance();
        if (distance == SENSOR_ERROR_VALUE) distance = 0;
        
        if (distance > currentGenome.clearThreshold && distance > bestDistance) {
          bestDistance = distance;
          bestDirection = 1;
          clearPathFound = true;
        }
      }
      
      // Return to center
      turnLeft();
      delay(currentGenome.turnDuration * 3);
      stopMotors();
      delay(currentGenome.scanDelay);
    }
    
    // Execute best maneuver
    if (clearPathFound) {
      // Express finding a path!
      communicateCurrentState(); // Emergent signal for finding path
      
      turnTime = currentGenome.turnDuration * 2;
      direction = bestDirection;
      
      if (bestDirection == 0) {
        turnLeft();
      } else {
        turnRight();
      }
      delay(turnTime);
      stopMotors();
      delay(200);
      
      int finalCheck = readDistance();
      if (finalCheck > currentGenome.clearThreshold || finalCheck == SENSOR_ERROR_VALUE) {
        moveForward();
        delay(800);
        succeeded = true;
        metrics.obstaclesCleared++;
        trappedAttempts = 0;
        
        // Express success!
        communicateCurrentState(); // Emergent signal for exploration success
      } else {
        trappedAttempts++;
        
        // Express confusion
        communicateCurrentState(); // Emergent signal for confusion
      }
      
      // Learn from this experience
      learnStrategy(initialDistance, direction, backupTime, turnTime, succeeded);
      
    } else {
      // No clear path - trapped situation
      trappedAttempts++;
      
      // Express frustration
      communicateCurrentState(); // Emergent signal for frustration
      
      if (trappedAttempts >= MAX_TRAPPED_ATTEMPTS) {
        aggressiveEscape();
      } else {
        Serial.println("⚠️ No clear path, backing up more");
        moveBackward();
        delay(1000);
        stopMotors();
      }
    }
  }
  
  isAvoiding = false;
  
  // Trigger evolution check after obstacle
  if (random(0, 100) < 10) { // 10% chance to evolve after obstacle
    evolutionCycle();
  }
}

// ═══════════════════════════════════════════════════════════
// 📡 ESP-NOW SWARM COMMUNICATION SYSTEM
// ═══════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════
// 🌊 EMERGENT ESP-NOW CALLBACKS 
// ═══════════════════════════════════════════════════════════

// ESP-NOW message sent callback
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.printf("✅ Emergent signal sent successfully to %s\n", macToString((uint8_t*)mac_addr).c_str());
  } else {
    Serial.printf("❌ Emergent signal send failed to %s\n", macToString((uint8_t*)mac_addr).c_str());
  }
}

// Initialize ESP-NOW communication
void initializeSwarmCommunication() {
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  // Print MAC address
  Serial.printf("📱 WHEELIE MAC: %s\n", WiFi.macAddress().c_str());
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Error initializing ESP-NOW");
    return;
  }
  
  // Register emergent communication callbacks
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onEmergentMessageReceived);
  
  // Set long range mode for better coverage
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
  
  Serial.println("✅ ESP-NOW initialized for emergent communication");
}

// ═══════════════════════════════════════════════════════════
// 🌊 EMERGENT COMMUNICATION SYSTEM 
// ═══════════════════════════════════════════════════════════

void communicateCurrentState() {
  // Get current environmental context and emotional state
  EnvironmentalContext context = getCurrentContext();
  EmotionalState emotion = getCurrentEmotionalState();
  
  Serial.printf("🌊 Generating emergent signal for context: %s, emotion: %s\n", 
                contextToString(context).c_str(), emotionToString(emotion).c_str());
  
  // Generate appropriate signal for current situation
  SignalWord* signal = signalGenerator.generateSignalForContext(context, emotion);
  
  if (signal != nullptr) {
    // Send emergent message via ESP-NOW
    bool sent = signalGenerator.sendEmergentMessage(signal, context, emotion);
    
    if (sent) {
      Serial.println("📡 Emergent signal broadcast successfully");
      displaySignalOnLEDs(signal, context, emotion);
    } else {
      Serial.println("❌ Failed to broadcast emergent signal");
    }
  } else {
    Serial.println("⚠️ No signal generated for current context");
  }
}

void displaySignalOnLEDs(SignalWord* signal, EnvironmentalContext context, EmotionalState emotion) {
  if (signal == nullptr) return;
  
  // Generate LED colors based on context and emotion
  uint8_t r, g, b;
  
  // Base color influenced by context
  switch (context) {
    case CONTEXT_DANGER_SENSED:
    case CONTEXT_OBSTACLE_NEAR:
      r = 255; g = 0; b = 0; // Red for danger
      break;
    case CONTEXT_TASK_SUCCESS:
    case CONTEXT_RESOURCE_FOUND:
      r = 0; g = 255; b = 0; // Green for success
      break;
    case CONTEXT_PEER_DETECTED:
    case CONTEXT_FOLLOWING:
    case CONTEXT_LEADING:
      r = 0; g = 0; b = 255; // Blue for social
      break;
    case CONTEXT_EXPLORATION:
      r = 255; g = 255; b = 0; // Yellow for exploration
      break;
    case CONTEXT_OPEN_SPACE:
      r = 0; g = 255; b = 255; // Cyan for open space
      break;
    default:
      r = 128; g = 128; b = 128; // Gray for neutral
      break;
  }
  
  // Modify intensity based on emotion
  float intensityMultiplier = 1.0f;
  switch (emotion) {
    case EMOTION_VERY_POSITIVE:
      intensityMultiplier = 1.0f; // Full brightness
      break;
    case EMOTION_POSITIVE:
      intensityMultiplier = 0.8f;
      break;
    case EMOTION_NEUTRAL:
      intensityMultiplier = 0.6f;
      break;
    case EMOTION_NEGATIVE:
      intensityMultiplier = 0.4f;
      break;
    case EMOTION_VERY_NEGATIVE:
      intensityMultiplier = 0.2f; // Dim for negative emotions
      break;
  }
  
  r = (uint8_t)(r * intensityMultiplier);
  g = (uint8_t)(g * intensityMultiplier);
  b = (uint8_t)(b * intensityMultiplier);
  
  // Set LEDs (assuming common anode - invert values)
  ledcWrite(PWM_CH_L_R, 255 - r);
  ledcWrite(PWM_CH_L_G, 255 - g);
  ledcWrite(PWM_CH_L_B, 255 - b);
  ledcWrite(PWM_CH_R_R, 255 - r);
  ledcWrite(PWM_CH_R_G, 255 - g);
  ledcWrite(PWM_CH_R_B, 255 - b);
  
  // Keep LEDs on for signal duration (sum of all component durations)
  uint16_t totalDuration = 0;
  for (uint8_t i = 0; i < signal->componentCount; i++) {
    totalDuration += signal->durations[i];
  }
  
  delay(constrain(totalDuration, 0, 2000)); // Cap at 2 seconds max
  
  // Return to dim white/off
  ledcWrite(PWM_CH_L_R, 240);
  ledcWrite(PWM_CH_L_G, 240);
  ledcWrite(PWM_CH_L_B, 240);
  ledcWrite(PWM_CH_R_R, 240);
  ledcWrite(PWM_CH_R_G, 240);
  ledcWrite(PWM_CH_R_B, 240);
}

// Update signal utility based on outcome
void updateSignalFeedback(bool wasSuccessful) {
  if (wasSuccessful) {
    recordSuccess();
  } else {
    recordFailure();
  }
}

// ESP-NOW callback for receiving emergent messages
void onEmergentMessageReceived(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(EmergentMessage)) {
    EmergentMessage* message = (EmergentMessage*)incomingData;
    
    Serial.printf("📨 Received emergent message from %s\n", macToString((uint8_t*)mac).c_str());
    
    // Update last peer contact time
    lastPeerContact = millis();
    
    // Process the message through the signal generator
    signalGenerator.processReceivedMessage(message);
    
    // Visual indication of receiving peer message
    // Flash blue briefly to show peer communication
    ledcWrite(PWM_CH_L_R, 255);
    ledcWrite(PWM_CH_L_G, 255);
    ledcWrite(PWM_CH_L_B, 100); // Blue flash
    ledcWrite(PWM_CH_R_R, 255);
    ledcWrite(PWM_CH_R_G, 255);
    ledcWrite(PWM_CH_R_B, 100);
    delay(200);
    
    // Return to normal
    ledcWrite(PWM_CH_L_R, 240);
    ledcWrite(PWM_CH_L_G, 240);
    ledcWrite(PWM_CH_L_B, 240);
    ledcWrite(PWM_CH_R_R, 240);
    ledcWrite(PWM_CH_R_G, 240);
    ledcWrite(PWM_CH_R_B, 240);
  }
}

// Old message handling functions removed - using emergent communication

// ═══════════════════════════════════════════════════════════
// 🔧 UTILITY FUNCTIONS FOR ESP-NOW
// ═══════════════════════════════════════════════════════════

// Convert MAC address to string for display
String macToString(const uint8_t* mac) {
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

// Check if two MAC addresses are equal
bool macEquals(const uint8_t* mac1, const uint8_t* mac2) {
  for (int i = 0; i < 6; i++) {
    if (mac1[i] != mac2[i]) return false;
  }
  return true;
}

// Simple checksum calculation for message integrity
uint8_t calculateChecksum(const uint8_t* data, size_t length) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < length; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

// Find or create peer in swarm list
int findOrCreatePeer(const uint8_t* mac) {
  // First try to find existing peer
  int existingIndex = findPeer(mac);
  if (existingIndex >= 0) {
    return existingIndex;
  }
  
  // Find first inactive slot
  for (int i = 0; i < MAX_SWARM_PEERS; i++) {
    if (!swarmPeers[i].isActive) {
      memcpy(swarmPeers[i].macAddress, mac, 6);
      swarmPeers[i].isActive = true;
      swarmPeers[i].lastSeen = millis();
      activePeerCount++;
      return i;
    }
  }
  
  return -1; // No space for new peer
}

// Find existing peer by MAC address
int findPeer(const uint8_t* mac) {
  for (int i = 0; i < MAX_SWARM_PEERS; i++) {
    if (swarmPeers[i].isActive && macEquals(swarmPeers[i].macAddress, mac)) {
      return i;
    }
  }
  return -1;
}

// Old ESP-NOW sending functions removed - using emergent communication

// ═══════════════════════════════════════════════════════════
// 🎬 SETUP
// ═══════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  🤖 EVOLVING SENTRY ROBOT SYSTEM 🧬  ║");
  Serial.println("║        WITH EMERGENT LANGUAGE 🗣️      ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  
  // Load persistent memory
  Serial.println("📖 Loading persistent memory...");
  loadGenomeFromEEPROM();
  loadStrategiesFromEEPROM();
  loadMetricsFromEEPROM();
  // Old vocabulary system removed - using emergent signals now
  
  // Emergent signal generator ready
  
  // Initialize buzzer if available
  if (BUZZER_PIN >= 0) {
    pinMode(BUZZER_PIN, OUTPUT);
    hasBuzzer = true;
    Serial.println("✅ Communication system initialized (buzzer)");
  } else {
    Serial.println("ℹ️ Communication via serial only (no buzzer)");
  }
  
  // Initialize sensors
  pinMode(MOTION_SENSOR_PIN, INPUT);
  Wire.begin();
  if (!lox.begin()) {
    Serial.println("❌ Failed to boot VL53L0X sensor");
    while(1);
  }
  Serial.println("✅ Sensor initialized");
  
  // Initialize motors
  ledcSetup(PWM_CHANNEL_LEFT1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_LEFT2, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_RIGHT1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_RIGHT2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LEFT_MOTOR_PIN1, PWM_CHANNEL_LEFT1);
  ledcAttachPin(LEFT_MOTOR_PIN2, PWM_CHANNEL_LEFT2);
  ledcAttachPin(RIGHT_MOTOR_PIN1, PWM_CHANNEL_RIGHT1);
  ledcAttachPin(RIGHT_MOTOR_PIN2, PWM_CHANNEL_RIGHT2);

  // Initialize RGB LED PWM channels
  ledcSetup(PWM_CH_L_R, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_L_G, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_L_B, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_R_R, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_R_G, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CH_R_B, PWM_FREQ, PWM_RESOLUTION);

  // Attach pins to LED PWM channels
  ledcAttachPin(LEFT_LED_R_PIN, PWM_CH_L_R);
  ledcAttachPin(LEFT_LED_G_PIN, PWM_CH_L_G);
  ledcAttachPin(LEFT_LED_B_PIN, PWM_CH_L_B);
  ledcAttachPin(RIGHT_LED_R_PIN, PWM_CH_R_R);
  ledcAttachPin(RIGHT_LED_G_PIN, PWM_CH_R_G);
  ledcAttachPin(RIGHT_LED_B_PIN, PWM_CH_R_B);

  Serial.println("✅ LEDs initialized");
  Serial.println("✅ Motors initialized");
  
  metrics.missionStartTime = millis();
  
  Serial.println("\n🧬 Current Evolution Status:");
  Serial.print("  Generation: ");
  Serial.println(currentGenome.generation);
  Serial.print("  Fitness Score: ");
  Serial.println(currentGenome.fitnessScore);
  Serial.print("  Strategies Learned: ");
  Serial.println(strategyCount);
  Serial.print("  Signal Vocabulary: ");
  Serial.println(signalGenerator.getVocabularySize());
  
  // Initialize ESP-NOW swarm communication
  Serial.println("\n📡 Initializing ESP-NOW swarm system...");
  initializeSwarmCommunication();
  
  // Boot greeting
  Serial.println("\n🗣️ Initializing communication protocol...");
  // Express "awake and ready"
  delay(500);
  communicateCurrentState(); // Emergent signal for evolution
  
  Serial.println("\n👁️ Waiting for motion to begin evolution...\n");
}

// ═══════════════════════════════════════════════════════════
// 🔄 MAIN LOOP
// ═══════════════════════════════════════════════════════════

void loop() {
  // Check for sleep
  checkSleepTimeout();
  
  // Check for motion
  if (digitalRead(MOTION_SENSOR_PIN) == HIGH) {
    if (!isAwake) {
      Serial.println("\n👁️ MOTION DETECTED! Waking up...");
      Serial.println("🧬 Resuming evolution...\n");
      
      // Express awakening/alertness
      communicateCurrentState(); // Emergent signal for mutation
      
      isAwake = true;
      lastActivityTime = millis();
      trappedAttempts = 0;
    }
  }

  if (isAwake && !isAvoiding) {
    int distance = readDistance();
    
    if (distance == SENSOR_ERROR_VALUE) {
      moveForward(); // Cautious forward on sensor error
    } else {
      if (distance < currentGenome.obstacleThreshold) {
        handleObstacle();
      } else {
        moveForward();
        
        // Occasionally express contentment while cruising
        if (random(0, 1000) < 5) { // 0.5% chance per loop
          communicateCurrentState(); // Emergent signal for clear path
        }
      }
    }
    
    // Periodic evolution check
    evolutionCycle();
    
  } else if (!isAwake) {
    // If sleeping, turn off LEDs and motors
    stopMotorsCoast();
    // Turn off LEDs (set PWM to max for common anode)
    ledcWrite(PWM_CH_L_R, 255); ledcWrite(PWM_CH_L_G, 255); ledcWrite(PWM_CH_L_B, 255);
    ledcWrite(PWM_CH_R_R, 255); ledcWrite(PWM_CH_R_G, 255); ledcWrite(PWM_CH_R_B, 255);
    delay(100);
  } else {
    stopMotors();
    delay(100);
  }
  
  // Emergent communication happens through sensors and context detection
}