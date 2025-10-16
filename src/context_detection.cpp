/*
 * 🌍 Environmental Context Detection
 * ==================================
 * Translates sensor data into environmental contexts for emergent signal generation
 * This is where the bot's sensors inform the communication system about "what's happening"
 * 
 * Key Functions:
 * - Translate sensor readings into meaningful contexts
 * - Track emotional state based on success/failure patterns
 * - Provide real-time context for signal generation
 * - Bridge between hardware sensors and communication system
 * 
 * Author: Project Jumbo Team
 * Version: 2.0.0 - Context-Aware Communication
 */

#include "emergent_signal.h"

// ═══════════════════════════════════════════════════════════
// 🎯 GLOBAL CONTEXT STATE
// ═══════════════════════════════════════════════════════════

static EnvironmentalContext currentContextState = CONTEXT_UNKNOWN;
static EmotionalState currentEmotionalState = EMOTION_NEUTRAL;
static unsigned long lastContextUpdate = 0;
static unsigned long lastSuccessTime = 0;
static unsigned long lastFailureTime = 0;
static uint8_t consecutiveSuccesses = 0;
static uint8_t consecutiveFailures = 0;

// External sensor data (these would be updated by main sensor loops)
extern int distance_cm;              // From HC-SR04 or VL53L0X
extern bool motionDetected;          // From RCWL-0516 (WHEELIE only)
extern float accelerationMagnitude;  // From MPU-6050 (SPEEDIE only)
extern bool isMoving;               // From movement detection
extern bool taskInProgress;        // From main task loop
extern bool taskSuccessful;        // From task completion detection
extern unsigned long lastPeerContact; // From ESP-NOW peer detection

// Forward declarations
void recordSuccess();
void recordFailure();
bool isStuck();

// ═══════════════════════════════════════════════════════════
// 🔍 CORE CONTEXT DETECTION
// ═══════════════════════════════════════════════════════════

EnvironmentalContext getCurrentContext() {
  unsigned long now = millis();
  
  // Update context maximum once per 500ms to avoid rapid changes
  if (now - lastContextUpdate < 500) {
    return currentContextState;
  }
  
  EnvironmentalContext newContext = CONTEXT_UNKNOWN;
  
  // Priority 1: Immediate danger/obstacles
  if (distance_cm > 0 && distance_cm < 15) {
    newContext = CONTEXT_OBSTACLE_NEAR;
  }
  // Priority 2: Task-related contexts
  else if (taskInProgress) {
    if (taskSuccessful) {
      newContext = CONTEXT_TASK_SUCCESS;
      recordSuccess();
    } else if (isStuck()) {
      newContext = CONTEXT_TASK_FAILURE;
      recordFailure();
    } else {
      newContext = CONTEXT_EXPLORATION; // Actively working on task
    }
  }
  // Priority 3: Peer interaction
  else if (now - lastPeerContact < 5000) { // Peer contact within 5 seconds
    newContext = CONTEXT_PEER_DETECTED;
  }
  // Priority 4: Movement states
  else if (isMoving) {
    if (distance_cm > 100 || distance_cm == 0) { // Clear path ahead
      newContext = CONTEXT_OPEN_SPACE;
    } else {
      newContext = CONTEXT_EXPLORATION;
    }
  }
  // Priority 5: Idle states
  else {
    newContext = CONTEXT_WAITING;
  }
  
  // Special contexts based on sensors
  #ifdef WHEELIE_BOT
  // WHEELIE has motion sensor - can detect interesting activity
  if (motionDetected && distance_cm > 30) {
    newContext = CONTEXT_RESOURCE_FOUND; // Something interesting detected
  }
  #endif
  
  #ifdef SPEEDIE_BOT
  // SPEEDIE has accelerometer - can detect if being moved/disturbed
  if (accelerationMagnitude > 2.0f && !isMoving) {
    newContext = CONTEXT_DANGER_SENSED; // Being moved unexpectedly
  }
  #endif
  
  // Update state
  currentContextState = newContext;
  lastContextUpdate = now;
  
  return newContext;
}

EmotionalState getCurrentEmotionalState() {
  unsigned long now = millis();
  
  // Emotional state is influenced by recent success/failure patterns
  
  // Very positive: Recent successes, no failures
  if (consecutiveSuccesses >= 3 && (now - lastFailureTime) > 30000) {
    currentEmotionalState = EMOTION_VERY_POSITIVE;
  }
  // Positive: More recent successes than failures
  else if (consecutiveSuccesses >= 2 && (now - lastSuccessTime) < 10000) {
    currentEmotionalState = EMOTION_POSITIVE;
  }
  // Very negative: Recent failures, stuck situations
  else if (consecutiveFailures >= 3 && (now - lastSuccessTime) > 30000) {
    currentEmotionalState = EMOTION_VERY_NEGATIVE;
  }
  // Negative: Recent failure or being stuck
  else if (consecutiveFailures >= 2 && (now - lastFailureTime) < 10000) {
    currentEmotionalState = EMOTION_NEGATIVE;
  }
  // Neutral: Balanced or no recent strong events
  else {
    currentEmotionalState = EMOTION_NEUTRAL;
  }
  
  // Decay extreme emotions over time
  if (now - lastSuccessTime > 60000 && now - lastFailureTime > 60000) {
    if (currentEmotionalState != EMOTION_NEUTRAL) {
      // Gradually return to neutral
      if (currentEmotionalState > EMOTION_NEUTRAL) {
        currentEmotionalState = (EmotionalState)(currentEmotionalState - 1);
      } else {
        currentEmotionalState = (EmotionalState)(currentEmotionalState + 1);
      }
    }
  }
  
  return currentEmotionalState;
}

// ═══════════════════════════════════════════════════════════
// 📊 SUCCESS/FAILURE TRACKING
// ═══════════════════════════════════════════════════════════

void recordSuccess() {
  lastSuccessTime = millis();
  consecutiveSuccesses++;
  consecutiveFailures = 0; // Reset failure counter
  
  if (consecutiveSuccesses > 10) {
    consecutiveSuccesses = 10; // Cap at 10
  }
  
  Serial.printf("✅ Success recorded (consecutive: %d)\n", consecutiveSuccesses);
}

void recordFailure() {
  lastFailureTime = millis();
  consecutiveFailures++;
  consecutiveSuccesses = 0; // Reset success counter
  
  if (consecutiveFailures > 10) {
    consecutiveFailures = 10; // Cap at 10
  }
  
  Serial.printf("❌ Failure recorded (consecutive: %d)\n", consecutiveFailures);
}

bool isStuck() {
  // Define "stuck" as: trying to move but distance hasn't changed significantly
  static int lastDistance = 0;
  static unsigned long lastMovementAttempt = 0;
  static uint8_t stuckCounter = 0;
  
  unsigned long now = millis();
  
  if (isMoving) {
    if (abs(distance_cm - lastDistance) < 5) {
      // Distance hasn't changed much despite movement
      stuckCounter++;
    } else {
      stuckCounter = 0; // Reset if we're making progress
    }
    
    lastDistance = distance_cm;
    lastMovementAttempt = now;
  }
  
  // Consider stuck if no progress for 3 consecutive movement attempts
  return (stuckCounter >= 3);
}

// ═══════════════════════════════════════════════════════════
// 🔧 CONTEXT ANALYSIS UTILITIES
// ═══════════════════════════════════════════════════════════

bool isContextUrgent(EnvironmentalContext context) {
  return (context == CONTEXT_DANGER_SENSED || 
          context == CONTEXT_OBSTACLE_NEAR || 
          context == CONTEXT_TASK_FAILURE);
}

bool isContextPositive(EnvironmentalContext context) {
  return (context == CONTEXT_TASK_SUCCESS || 
          context == CONTEXT_RESOURCE_FOUND || 
          context == CONTEXT_OPEN_SPACE);
}

bool isContextSocial(EnvironmentalContext context) {
  return (context == CONTEXT_PEER_DETECTED || 
          context == CONTEXT_FOLLOWING || 
          context == CONTEXT_LEADING);
}

float getContextIntensity(EnvironmentalContext context) {
  switch (context) {
    case CONTEXT_DANGER_SENSED:
    case CONTEXT_TASK_FAILURE:
      return 0.9f; // High intensity
      
    case CONTEXT_OBSTACLE_NEAR:
    case CONTEXT_TASK_SUCCESS:
      return 0.7f; // Medium-high intensity
      
    case CONTEXT_PEER_DETECTED:
    case CONTEXT_RESOURCE_FOUND:
    case CONTEXT_EXPLORATION:
      return 0.5f; // Medium intensity
      
    case CONTEXT_OPEN_SPACE:
    case CONTEXT_FOLLOWING:
    case CONTEXT_LEADING:
      return 0.3f; // Low-medium intensity
      
    case CONTEXT_WAITING:
    default:
      return 0.1f; // Low intensity
  }
}

// ═══════════════════════════════════════════════════════════
// 📈 CONTEXT HISTORY & PATTERNS
// ═══════════════════════════════════════════════════════════

#define CONTEXT_HISTORY_SIZE 20
static EnvironmentalContext contextHistory[CONTEXT_HISTORY_SIZE];
static unsigned long contextTimestamps[CONTEXT_HISTORY_SIZE];
static uint8_t historyIndex = 0;
static bool historyFull = false;

void updateContextHistory(EnvironmentalContext context) {
  contextHistory[historyIndex] = context;
  contextTimestamps[historyIndex] = millis();
  
  historyIndex = (historyIndex + 1) % CONTEXT_HISTORY_SIZE;
  if (historyIndex == 0) {
    historyFull = true;
  }
}

EnvironmentalContext getMostFrequentRecentContext() {
  uint8_t contextCounts[12] = {0}; // Count for each context type
  uint8_t maxCount = 0;
  EnvironmentalContext mostFrequent = CONTEXT_UNKNOWN;
  
  uint8_t itemsToCheck = historyFull ? CONTEXT_HISTORY_SIZE : historyIndex;
  
  for (uint8_t i = 0; i < itemsToCheck; i++) {
    if (contextHistory[i] < 12) { // Valid context
      contextCounts[contextHistory[i]]++;
      if (contextCounts[contextHistory[i]] > maxCount) {
        maxCount = contextCounts[contextHistory[i]];
        mostFrequent = contextHistory[i];
      }
    }
  }
  
  return mostFrequent;
}

float getContextStability() {
  // Measure how stable the context has been recently
  if (!historyFull && historyIndex < 5) {
    return 0.5f; // Not enough data
  }
  
  uint8_t itemsToCheck = min(10, historyFull ? CONTEXT_HISTORY_SIZE : historyIndex);
  uint8_t changes = 0;
  
  for (uint8_t i = 1; i < itemsToCheck; i++) {
    uint8_t prevIndex = (historyIndex - i - 1 + CONTEXT_HISTORY_SIZE) % CONTEXT_HISTORY_SIZE;
    uint8_t currIndex = (historyIndex - i + CONTEXT_HISTORY_SIZE) % CONTEXT_HISTORY_SIZE;
    
    if (contextHistory[prevIndex] != contextHistory[currIndex]) {
      changes++;
    }
  }
  
  // Stability = 1.0 - (changes / possible_changes)
  return 1.0f - (float)changes / (float)(itemsToCheck - 1);
}

// ═══════════════════════════════════════════════════════════
// 🎮 CONTEXT DEBUGGING & MONITORING
// ═══════════════════════════════════════════════════════════

void printContextState() {
  Serial.println("🌍 === CURRENT ENVIRONMENTAL STATE ===");
  Serial.printf("Context: %s (intensity: %.1f)\n", 
                contextToString(currentContextState).c_str(), 
                getContextIntensity(currentContextState));
  Serial.printf("Emotion: %s\n", emotionToString(currentEmotionalState).c_str());
  Serial.printf("Consecutive successes: %d, failures: %d\n", 
                consecutiveSuccesses, consecutiveFailures);
  Serial.printf("Context stability: %.2f\n", getContextStability());
  Serial.printf("Most frequent recent: %s\n", 
                contextToString(getMostFrequentRecentContext()).c_str());
  
  // Sensor summary
  Serial.printf("Sensors: dist=%dcm, motion=%s, moving=%s\n", 
                distance_cm, motionDetected ? "YES" : "NO", isMoving ? "YES" : "NO");
  
  Serial.println("========================================");
}