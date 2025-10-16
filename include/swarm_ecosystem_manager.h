#pragma once

#include <Arduino.h>
#include <vector>
#include <map>
#include "swarm_espnow.h"

// ═══════════════════════════════════════════════════════════
// 🌐 SWARM ECOSYSTEM MANAGER - LAYER 3 INTELLIGENCE
// ═══════════════════════════════════════════════════════════

#define MAX_BOT_PROFILES 16
#define MAX_RELATIONSHIPS 64
#define REPUTATION_UPDATE_INTERVAL 600000  // 10 minutes
#define ECOSYSTEM_ANALYSIS_INTERVAL 3600000  // 1 hour
#define MIN_TRUST_SCORE 0.3f
#define DATA_VERIFICATION_WINDOW 30000  // 30 seconds

// ═══════════════════════════════════════════════════════════
// 📊 BOT HEALTH & REPUTATION TRACKING
// ═══════════════════════════════════════════════════════════

enum BotHealth {
  HEALTH_EXCELLENT = 5,    // >95% functionality
  HEALTH_GOOD = 4,         // 80-95%
  HEALTH_DEGRADED = 3,     // 60-80%
  HEALTH_FAILING = 2,      // 40-60%
  HEALTH_CRITICAL = 1      // <40% (needs immediate attention)
};

enum ComponentHealth {
  COMPONENT_OPERATIONAL = 3,
  COMPONENT_DEGRADED = 2,
  COMPONENT_FAILING = 1,
  COMPONENT_OFFLINE = 0
};

struct BotProfile {
  // ═══════════════════════════════════════
  // IDENTITY
  // ═══════════════════════════════════════
  uint8_t botMAC[6];              // Unique ESP32 MAC address
  char botName[16];               // e.g., "WHEELIE", "SPEEDIE"
  BotType botType;                // SCOUT, MANIPULATOR, etc.
  uint32_t serialNumber;          // Manufacturing/build number
  uint32_t activationTimestamp;   // When first powered on
  
  // ═══════════════════════════════════════
  // EVOLUTION STATUS
  // ═══════════════════════════════════════
  uint32_t generation;            // Current evolution generation
  float fitness;                  // Current fitness score
  uint32_t totalRuntime;          // Lifetime operational hours
  uint32_t totalMissions;         // Tasks completed
  
  // ═══════════════════════════════════════
  // RELIABILITY METRICS
  // ═══════════════════════════════════════
  float dataAccuracy;             // 0.0-1.0 (sensor data quality)
  float missionSuccessRate;       // 0.0-1.0 (task completion)
  float communicationReliability; // 0.0-1.0 (message quality)
  float availabilityScore;        // 0.0-1.0 (uptime/downtime)
  
  uint32_t totalDataSent;         // Messages broadcast
  uint32_t dataVerifiedGood;      // Confirmed accurate by peers
  uint32_t dataVerifiedBad;       // Contradicted by peers
  
  // ═══════════════════════════════════════
  // REPUTATION & TRUST
  // ═══════════════════════════════════════
  float reputationScore;          // 0.0-100.0 (composite)
  uint32_t lastReputationUpdate;  // Timestamp
  
  // ═══════════════════════════════════════
  // HEALTH STATUS
  // ═══════════════════════════════════════
  BotHealth health;               // Overall health assessment
  ComponentHealth sensorHealth[4]; // Individual sensor health
  float estimatedRemainingLife;   // Hours until maintenance
  
  // ═══════════════════════════════════════
  // ECOSYSTEM TRACKING
  // ═══════════════════════════════════════
  uint32_t lastSeenTimestamp;     // Last communication
  uint32_t consecutiveFailures;   // Failed communications in a row
  bool needsInspection;           // Flag for human attention
  bool isBlacklisted;             // Excluded from critical tasks
};

// ═══════════════════════════════════════════════════════════
// 🤝 TRUST NETWORK & RELATIONSHIP MANAGEMENT
// ═══════════════════════════════════════════════════════════

enum InteractionType {
  INTERACTION_DATA_SHARE = 0,     // Shared sensor data
  INTERACTION_TASK_COLLAB = 1,    // Collaborated on task
  INTERACTION_EMERGENCY = 2,      // Emergency assistance
  INTERACTION_LOCALIZATION = 3,   // Position/navigation help
  INTERACTION_RESOURCE_SHARE = 4  // Shared resources/power
};

enum InteractionResult {
  RESULT_SUCCESS = 0,             // Interaction went well
  RESULT_PARTIAL = 1,             // Some issues but overall okay
  RESULT_FAILURE = 2,             // Interaction failed
  RESULT_CONTRADICTED = 3         // Data proven wrong by others
};

struct BotRelationship {
  uint8_t botA_MAC[6];
  uint8_t botB_MAC[6];
  
  // ═══════════════════════════════════════
  // TRUST METRICS
  // ═══════════════════════════════════════
  float trustScore;               // 0.0-1.0
  uint32_t interactionCount;      // Total interactions
  uint32_t successfulInteractions; // Went well
  uint32_t failedInteractions;    // Went poorly
  
  // ═══════════════════════════════════════
  // INTERACTION HISTORY
  // ═══════════════════════════════════════
  InteractionType recentInteractions[10]; // Last 10 interactions
  InteractionResult recentResults[10];    // Results of last 10
  uint32_t recentTimestamps[10];          // When they occurred
  uint8_t interactionIndex;               // Circular buffer index
  
  // ═══════════════════════════════════════
  // DATA QUALITY TRACKING
  // ═══════════════════════════════════════
  uint32_t dataPointsShared;      // Total data points shared
  uint32_t dataPointsVerified;    // Confirmed accurate
  uint32_t dataPointsContradicted; // Proven wrong
  
  uint32_t lastInteraction;       // Timestamp of last interaction
  bool isActive;                  // Relationship actively tracked
};

struct DataVerificationEntry {
  uint8_t senderMAC[6];           // Who sent the data
  uint8_t verifierMAC[6];         // Who verified/contradicted
  uint32_t dataHash;              // Hash of data content
  bool isVerification;            // true = verified, false = contradicted
  uint32_t timestamp;             // When verification occurred
  float dataConfidence;           // Confidence in verification (0.0-1.0)
};

// ═══════════════════════════════════════════════════════════
// 🧠 SWARM ECOSYSTEM INTELLIGENCE CLASS
// ═══════════════════════════════════════════════════════════

class SwarmEcosystemManager {
private:
  BotProfile botProfiles[MAX_BOT_PROFILES];
  uint8_t botCount;
  
  BotRelationship relationships[MAX_RELATIONSHIPS];
  uint8_t relationshipCount;
  
  DataVerificationEntry verificationLog[100];
  uint8_t verificationLogIndex;
  
  unsigned long lastReputationUpdate;
  unsigned long lastEcosystemAnalysis;
  
public:
  // ═══════════════════════════════════════
  // INITIALIZATION & SETUP
  // ═══════════════════════════════════════
  SwarmEcosystemManager();
  void initialize();
  
  // ═══════════════════════════════════════
  // BOT PROFILE MANAGEMENT
  // ═══════════════════════════════════════
  void registerBot(uint8_t* mac, BotType type, const char* name);
  void updateBotStatus(uint8_t* mac, uint32_t generation, float fitness);
  void updateBotHealth(uint8_t* mac, BotHealth health);
  BotProfile* getBotProfile(uint8_t* mac);
  void deactivateBot(uint8_t* mac);
  
  // ═══════════════════════════════════════
  // TRUST & RELATIONSHIP MANAGEMENT
  // ═══════════════════════════════════════
  void recordInteraction(uint8_t* botA, uint8_t* botB, 
                        InteractionType type, InteractionResult result);
  void recordDataVerification(uint8_t* sender, uint8_t* verifier, 
                             uint32_t dataHash, bool isCorrect, float confidence);
  float getTrustScore(uint8_t* botA, uint8_t* botB);
  bool shouldTrustBot(uint8_t* mac, float minTrustThreshold = MIN_TRUST_SCORE);
  
  // ═══════════════════════════════════════
  // REPUTATION CALCULATION
  // ═══════════════════════════════════════
  void updateAllReputations();
  float calculateReputation(uint8_t* mac);
  void updateDataAccuracy(uint8_t* mac);
  
  // ═══════════════════════════════════════
  // ECOSYSTEM ANALYSIS
  // ═══════════════════════════════════════
  void performEcosystemAnalysis();
  void identifyWeakBots();
  void identifyCapabilityGaps();
  void generateUpgradeRecommendations();
  
  // ═══════════════════════════════════════
  // SWARM HEALTH MONITORING
  // ═══════════════════════════════════════
  float getOverallSwarmHealth();
  uint8_t getHealthyBotCount();
  uint8_t getDegradedBotCount();
  BotProfile** getMostTrustedBots(uint8_t* count);
  BotProfile** getLeastTrustedBots(uint8_t* count);
  
  // ═══════════════════════════════════════
  // TASK ASSIGNMENT INTELLIGENCE  
  // ═══════════════════════════════════════
  uint8_t* selectBestBotForTask(TaskType task, uint8_t* candidateMACs, uint8_t candidateCount);
  bool shouldExcludeFromCriticalTasks(uint8_t* mac);
  void blacklistBot(uint8_t* mac, const char* reason);
  void rehabilitateBot(uint8_t* mac);
  
  // ═══════════════════════════════════════
  // PERIODIC UPDATE CYCLE
  // ═══════════════════════════════════════
  void update(); // Call this regularly from main loop
  
  // ═══════════════════════════════════════
  // DEBUG & REPORTING
  // ═══════════════════════════════════════
  void printSwarmReport();
  void printBotProfile(uint8_t* mac);
  void printTrustNetwork();
  void exportEcosystemData(char* buffer, size_t maxSize);
};

// ═══════════════════════════════════════════════════════════
// 🎯 SWARM INTELLIGENCE MESSAGE TYPES
// ═══════════════════════════════════════════════════════════

// Add these to your existing MessageType enum in swarm_espnow.h
enum EcosystemMessageType {
  MSG_REPUTATION_UPDATE = 0x60,     // Reputation score broadcast
  MSG_DATA_VERIFICATION = 0x61,     // Data verification/contradiction
  MSG_TRUST_ANNOUNCEMENT = 0x62,    // Trust relationship announcement
  MSG_HEALTH_STATUS = 0x63,         // Bot health status report
  MSG_ECOSYSTEM_ALERT = 0x64,       // Ecosystem-level alert
  MSG_BOT_INSPECTION_REQUEST = 0x65, // Request for bot inspection
  MSG_BLACKLIST_NOTIFICATION = 0x66, // Bot blacklisted notification
  MSG_CAPABILITY_QUERY = 0x67       // Query bot capabilities
};

struct ReputationPayload {
  uint8_t targetBotMAC[6];
  float reputationScore;
  float trustScore;
  uint32_t interactionCount;
  BotHealth reportedHealth;
  uint32_t timestamp;
};

struct DataVerificationPayload {
  uint8_t originalSenderMAC[6];
  uint32_t dataHash;              // Hash of the original data
  bool isVerification;            // true = verified, false = contradicted
  float confidence;               // 0.0-1.0 confidence in verification
  uint32_t timestamp;
  char description[32];           // Brief description of data type
};

struct HealthStatusPayload {
  BotHealth overallHealth;
  ComponentHealth sensorHealth[4];
  float estimatedRemainingLife;
  uint32_t consecutiveFailures;
  uint32_t lastMaintenanceTime;
  bool needsInspection;
};

// ═══════════════════════════════════════════════════════════
// 🚨 ECOSYSTEM ALERTS & NOTIFICATIONS
// ═══════════════════════════════════════════════════════════

enum EcosystemAlertType {
  ALERT_LOW_TRUST_BOT = 0x01,       // Bot trust score below threshold
  ALERT_CAPABILITY_GAP = 0x02,      // Missing critical capability
  ALERT_SWARM_DEGRADED = 0x03,      // Overall swarm health low
  ALERT_BOT_FAILURE = 0x04,         // Bot complete failure
  ALERT_DATA_CORRUPTION = 0x05,     // Widespread bad data detected
  ALERT_COMMUNICATION_LOSS = 0x06   // Lost communication with key bots
};

struct EcosystemAlertPayload {
  EcosystemAlertType alertType;
  uint8_t relevantBotMAC[6];        // Bot involved in alert (if applicable)
  uint32_t alertSeverity;           // 1-10 severity scale
  uint32_t alertTimestamp;
  char alertDescription[64];        // Human-readable description
};

// ═══════════════════════════════════════════════════════════
// 🔧 INTEGRATION HELPERS
// ═══════════════════════════════════════════════════════════

// Global ecosystem manager instance (to be created in main.cpp)
extern SwarmEcosystemManager* ecosystemManager;

// Helper functions for integration with existing ESP-NOW code
void initializeEcosystemManager();
void handleEcosystemMessage(const uint8_t* senderMac, SwarmMessage* message);
bool verifyDataWithEcosystem(uint8_t* senderMac, uint32_t dataHash, float* trustMultiplier);
void reportInteractionToEcosystem(uint8_t* peerMac, InteractionType type, InteractionResult result);