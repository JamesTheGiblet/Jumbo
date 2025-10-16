/*
 * 🌊 Emergent Signal Protocol for ESP-NOW
 * ========================================
 * Truly emergent communication where signals evolve from context and experience
 * No predefined vocabulary - signals emerge from environmental interaction
 * 
 * Core Principles:
 * 1. Signals are GENERATED not SELECTED from predefined lists
 * 2. Signal meaning emerges from CONTEXT + EMOTIONAL STATE
 * 3. Signal evolution driven by UTILITY feedback from peers
 * 4. Minimal bootstrap vocabulary grows through interaction
 * 5. Each bot develops UNIQUE signal personality
 * 
 * Inspired by: Animal communication, birdsong evolution, whale languages
 * Author: Project Jumbo Team
 * Version: 2.0.0 - Emergent Language System
 */

#ifndef EMERGENT_SIGNAL_H
#define EMERGENT_SIGNAL_H

#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════
// 🧬 EMERGENT SIGNAL GENERATION SYSTEM
// ═══════════════════════════════════════════════════════════

// Maximum vocabulary size per bot (dynamic growth)
#define MAX_SIGNAL_VOCABULARY 64
#define MAX_SIGNAL_COMPONENTS 8
#define MAX_CONTEXT_MEMORY 32
#define SIGNAL_EVOLUTION_THRESHOLD 0.7f

// Signal component types (building blocks, not predefined meanings)
enum SignalComponent {
  COMPONENT_TONE_LOW = 0x01,      // Low frequency base
  COMPONENT_TONE_MID = 0x02,      // Mid frequency 
  COMPONENT_TONE_HIGH = 0x03,     // High frequency peak
  COMPONENT_PULSE_FAST = 0x04,    // Rapid pulsing
  COMPONENT_PULSE_SLOW = 0x05,    // Slow pulsing
  COMPONENT_SWEEP_UP = 0x06,      // Rising frequency
  COMPONENT_SWEEP_DOWN = 0x07,    // Falling frequency
  COMPONENT_SILENCE = 0x08        // Pause/gap
};

// Context types (environmental situations, not message meanings)
enum EnvironmentalContext {
  CONTEXT_OBSTACLE_NEAR = 0x01,     // Physical obstruction detected
  CONTEXT_OPEN_SPACE = 0x02,        // Clear movement area
  CONTEXT_PEER_DETECTED = 0x03,     // Another bot sensed
  CONTEXT_TASK_SUCCESS = 0x04,      // Goal achieved
  CONTEXT_TASK_FAILURE = 0x05,      // Goal failed
  CONTEXT_RESOURCE_FOUND = 0x06,    // Interesting object/area
  CONTEXT_DANGER_SENSED = 0x07,     // Potential threat
  CONTEXT_EXPLORATION = 0x08,       // Actively exploring
  CONTEXT_WAITING = 0x09,           // Idle/waiting state
  CONTEXT_FOLLOWING = 0x0A,         // Following behavior
  CONTEXT_LEADING = 0x0B,           // Leading behavior
  CONTEXT_UNKNOWN = 0xFF            // New/unclassified situation
};

// Emotional valence (internal state, affects signal generation)
enum EmotionalState {
  EMOTION_VERY_NEGATIVE = -2,    // Frustrated, trapped, failed
  EMOTION_NEGATIVE = -1,         // Concerned, cautious
  EMOTION_NEUTRAL = 0,           // Normal operating state
  EMOTION_POSITIVE = 1,          // Successful, optimistic
  EMOTION_VERY_POSITIVE = 2      // Excited, highly successful
};

// ═══════════════════════════════════════════════════════════
// 🎵 SIGNAL WORD STRUCTURE (Dynamic Vocabulary)
// ═══════════════════════════════════════════════════════════

struct SignalWord {
  // === SEMANTIC PROPERTIES (What does this signal relate to?) ===
  uint8_t contextType;           // EnvironmentalContext when created
  int8_t emotionalValence;       // EmotionalState when created
  uint16_t generation;           // When in bot's life was this created
  
  // === ACOUSTIC PROPERTIES (How does this sound?) ===
  uint8_t componentCount;        // Number of signal components (1-8)
  uint8_t components[MAX_SIGNAL_COMPONENTS]; // SignalComponent sequence
  uint16_t durations[MAX_SIGNAL_COMPONENTS]; // Duration of each component (ms)
  uint8_t intensities[MAX_SIGNAL_COMPONENTS]; // Intensity 0-255
  
  // === EVOLUTIONARY PROPERTIES (How successful is this signal?) ===
  float utility;                 // Success rate (0.0-1.0)
  uint32_t timesUsed;           // Usage frequency
  uint32_t timesUnderstood;     // How often peers responded appropriately
  uint32_t lastUsed;            // Timestamp of last usage
  uint32_t createdAt;           // Timestamp of creation
  
  // === PERSONALITY TRAITS (Bot's unique signature) ===
  uint8_t personalitySignature; // Unique acoustic "accent"
  uint8_t complexityPreference; // Tendency toward simple/complex signals
} __attribute__((packed));

// ═══════════════════════════════════════════════════════════
// 📡 EMERGENT ESP-NOW MESSAGE STRUCTURE
// ═══════════════════════════════════════════════════════════

struct EmergentMessage {
  // === TRANSMISSION METADATA ===
  uint8_t protocolVersion;      // Always 0x02 for emergent protocol
  uint8_t senderMac[6];         // Sender's MAC address
  uint32_t timestamp;           // Message timestamp
  uint8_t sequenceNumber;       // Message sequence (0-255)
  
  // === EMERGENT SIGNAL DATA ===
  SignalWord signal;            // The actual emergent signal being sent
  
  // === CONTEXTUAL METADATA ===
  uint8_t currentContext;       // Sender's current EnvironmentalContext
  int8_t currentEmotion;        // Sender's current EmotionalState
  uint8_t confidence;           // How sure sender is about this signal (0-255)
  uint8_t expectsResponse;      // Does sender expect a response? (0/1)
  
  // === LEARNING METADATA ===
  uint8_t isResponse;           // Is this responding to a previous signal? (0/1)
  uint8_t respondingToSequence; // If response, sequence# of original message
  uint8_t signalAge;            // How long has sender used this signal?
  
  // === INTEGRITY ===
  uint8_t checksum;             // Message integrity check
} __attribute__((packed));

// ═══════════════════════════════════════════════════════════
// 🧠 SIGNAL UNDERSTANDING & RESPONSE SYSTEM
// ═══════════════════════════════════════════════════════════

struct SignalMemory {
  uint8_t senderMac[6];         // Who sent this signal?
  SignalWord receivedSignal;    // What signal did they send?
  uint8_t contextWhenReceived;  // What was happening when we got it?
  uint8_t ourResponse;          // What did we do in response?
  float outcomeScore;           // How well did our response work? (-1.0 to +1.0)
  uint32_t timestamp;           // When did this happen?
};

struct PeerSignalProfile {
  uint8_t peerMac[6];           // Peer's MAC address
  uint8_t signalCount;          // How many different signals from this peer
  SignalWord knownSignals[16];  // Signals we've learned from this peer
  float trustLevel;             // How much we trust signals from this peer (0.0-1.0)
  uint8_t personalitySignature; // This peer's acoustic "accent"
  uint32_t lastInteraction;     // Last time we communicated
};

// ═══════════════════════════════════════════════════════════
// 🎭 SIGNAL GENERATION ALGORITHMS
// ═══════════════════════════════════════════════════════════

class EmergentSignalGenerator {
private:
  SignalWord vocabulary[MAX_SIGNAL_VOCABULARY];
  uint8_t vocabularySize;
  SignalMemory contextMemory[MAX_CONTEXT_MEMORY];
  uint8_t memorySize;
  PeerSignalProfile peerProfiles[8]; // Track up to 8 peers
  uint8_t peerCount;
  
  // Bot's unique signal personality traits
  uint8_t personalitySignature;
  uint8_t complexityPreference;
  uint8_t innovationRate;       // How often to create new signals vs reuse
  uint16_t currentGeneration;   // Bot's current generation/age
  
public:
  EmergentSignalGenerator();
  
  // === CORE SIGNAL GENERATION ===
  SignalWord* generateSignalForContext(EnvironmentalContext context, EmotionalState emotion);
  SignalWord* findExistingSignal(EnvironmentalContext context, EmotionalState emotion);
  SignalWord createNewSignal(EnvironmentalContext context, EmotionalState emotion);
  
  // === SIGNAL EVOLUTION ===
  void updateSignalUtility(SignalWord* signal, float outcome);
  void mutateSignal(SignalWord* signal);
  void pruneUnusedSignals();
  
  // === PEER LEARNING ===
  void learnFromPeerSignal(uint8_t* peerMac, SignalWord* signal, EnvironmentalContext context);
  float evaluateSignalSimilarity(SignalWord* signal1, SignalWord* signal2);
  void updatePeerTrust(uint8_t* peerMac, float outcome);
  
  // === COMMUNICATION INTERFACE ===
  bool sendEmergentMessage(SignalWord* signal, EnvironmentalContext context, EmotionalState emotion);
  void processReceivedMessage(EmergentMessage* message);
  
  // === ANALYTICS & DEBUG ===
  void printVocabularyStats();
  uint8_t getVocabularySize() { return vocabularySize; }
  float getAverageUtility();
  SignalWord* getMostUsedSignal();
};

// ═══════════════════════════════════════════════════════════
// 🔧 UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════

// Generate random signal components
uint8_t generateRandomComponent();
uint16_t generateRandomDuration();
uint8_t generateRandomIntensity();

// Signal comparison and analysis
float calculateAcousticSimilarity(SignalWord* sig1, SignalWord* sig2);
float calculateSemanticSimilarity(SignalWord* sig1, SignalWord* sig2);

// Context analysis
EnvironmentalContext getCurrentContext();
EmotionalState getCurrentEmotionalState();

// Acoustic generation helpers
void playSignalWord(SignalWord* signal);
String signalToString(SignalWord* signal);
String contextToString(EnvironmentalContext context);
String emotionToString(EmotionalState emotion);

// MAC address utilities
String macToString(uint8_t* mac);
bool macEquals(uint8_t* mac1, uint8_t* mac2);

#endif // EMERGENT_SIGNAL_H