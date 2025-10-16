# 🌐 Swarm Ecosystem Manager (SEM)

## A meta-evolutionary system for managing bot relationships, identifying weaknesses, and evolving swarm composition

---

## 📋 Table of Contents

1. [Core Concepts](#core-concepts)
2. [Bot Identity & Tracking](#bot-identity--tracking)
3. [Relationship Management](#relationship-management)
4. [Data Quality Assessment](#data-quality-assessment)
5. [Weakness Detection](#weakness-detection)
6. [Upgrade Recommendations](#upgrade-recommendations)
7. [New Bot Creation](#new-bot-creation)
8. [Implementation](#implementation)
9. [Example Scenarios](#example-scenarios)

---

## Core Concepts

### The Three Layers of Evolution

```txt
Layer 1: INDIVIDUAL EVOLUTION (Current)
┌─────────────────────────────────────┐
│ Bot evolves its own parameters      │
│ • Motor speed, thresholds, etc.     │
│ • Fitness-based selection           │
│ • ~60 second cycles                 │
└─────────────────────────────────────┘

Layer 2: RELATIONSHIP EVOLUTION (New!)
┌─────────────────────────────────────┐
│ Bots build trust with each other    │
│ • Track reliability of peers        │
│ • Prefer proven collaborators       │
│ • Identify unreliable bots          │
│ • ~10 minute evaluation cycles      │
└─────────────────────────────────────┘

Layer 3: ECOSYSTEM EVOLUTION (New!)
┌─────────────────────────────────────┐
│ MCU manages swarm composition       │
│ • Identifies capability gaps        │
│ • Recommends upgrades               │
│ • Suggests new bot types            │
│ • ~1 hour analysis cycles           │
└─────────────────────────────────────┘
```

### Key Principle: Bad Data = Bad Friend

```txt
Unreliable Bot Consequences:
┌──────────────────────────────────────┐
│ SPEEDY reports obstacle at (5, 10)  │
│ WHEELIE verifies: Nothing there!     │
│                                      │
│ → WHEELIE decreases trust in SPEEDY  │
│ → Future messages weighted less      │
│ → SPEEDY gets lower priority tasks   │
│ → Eventually flagged for inspection  │
└──────────────────────────────────────┘
```

---

## Bot Identity & Tracking

### Bot Profile Structure

```cpp
struct BotProfile {
  // ═══════════════════════════════════════
  // IDENTITY
  // ═══════════════════════════════════════
  uint8_t botMAC[6];              // Unique ESP32 MAC address
  char botName[16];               // e.g., "WHEELIE", "SPEEDY"
  BotType botType;                // SCOUT, MANIPULATOR, HEAVY, AERIAL
  uint32_t serialNumber;          // Manufacturing/build number
  uint32_t activationTimestamp;   // When first powered on
  
  // ═══════════════════════════════════════
  // HARDWARE CAPABILITIES
  // ═══════════════════════════════════════
  SensorType sensors[8];          // What sensors equipped?
  ActuatorType actuators[4];      // What actuators?
  float maxSpeed;                 // Physical capability
  float maxPayload;               // Can it carry things?
  float batteryCapacity;          // Power endurance
  
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
  // REPUTATION
  // ═══════════════════════════════════════
  float reputationScore;          // 0.0-100.0 (composite)
  uint32_t lastReputationUpdate;  // Timestamp
  
  // ═══════════════════════════════════════
  // HEALTH STATUS
  // ═══════════════════════════════════════
  BotHealth health;               // HEALTHY, DEGRADED, FAILING
  ComponentHealth sensors_health[8];
  ComponentHealth actuators_health[4];
  float estimatedRemainingLife;   // Hours until maintenance
  
  // ═══════════════════════════════════════
  // UPGRADE RECOMMENDATIONS
  // ═══════════════════════════════════════
  bool needsUpgrade;
  UpgradeType recommendedUpgrades[4];
  uint32_t upgradeUrgency;        // 0-100
};

enum BotType {
  BOT_SCOUT,           // WHEELIE, SPEEDY
  BOT_MANIPULATOR,     // GRABBER
  BOT_HEAVY,           // TANK
  BOT_AERIAL,          // DRONE
  BOT_SPECIALIST       // Custom roles
};

enum BotHealth {
  HEALTH_EXCELLENT,    // >95% functionality
  HEALTH_GOOD,         // 80-95%
  HEALTH_DEGRADED,     // 60-80%
  HEALTH_FAILING,      // 40-60%
  HEALTH_CRITICAL      // <40% (needs immediate attention)
};
```

### Bot Registry (On MCU/Hub)

```cpp
class BotRegistry {
private:
  BotProfile bots[32];            // Support up to 32 bots
  uint8_t botCount;
  
public:
  // Bot lifecycle management
  void registerBot(BotProfile* bot);
  void updateBotStatus(uint8_t* mac, BotProfile* update);
  void deactivateBot(uint8_t* mac, DeactivationReason reason);
  
  // Query functions
  BotProfile* getBotByMAC(uint8_t* mac);
  BotProfile* getBotByName(const char* name);
  BotProfile** getBotsOfType(BotType type, uint8_t* count);
  BotProfile** getHealthyBots(uint8_t* count);
  BotProfile** getDegradedBots(uint8_t* count);
  
  // Analytics
  float getAverageReputation();
  float getSwarmFitness();
  BotType getMostNeededBotType();
  void generateSwarmReport(SwarmReport* report);
};
```

---

## Relationship Management

### Peer Trust Network

```cpp
struct BotRelationship {
  uint8_t botA_MAC[6];
  uint8_t botB_MAC[6];
  
  // ═══════════════════════════════════════
  // TRUST METRICS
  // ═══════════════════════════════════════
  float trustScore;               // 0.0-1.0
  uint32_t interactionCount;      // Total interactions
  uint32_t successfulInteractions;// Went well
  uint32_t failedInteractions;    // Went poorly
  
  // ═══════════════════════════════════════
  // DATA VERIFICATION
  // ═══════════════════════════════════════
  uint32_t messagesReceived;
  uint32_t messagesVerified;      // Confirmed by A
  uint32_t messagesContradicted;  // Conflicted with A's sensors
  
  float dataAgreementRate;        // messagesVerified / messagesReceived
  
  // ═══════════════════════════════════════
  // COLLABORATION HISTORY
  // ═══════════════════════════════════════
  uint32_t missionsCompleted;     // Tasks done together
  uint32_t missionsFailed;        // Tasks failed together
  float collaborationScore;       // Success rate when working together
  
  // ═══════════════════════════════════════
  // TEMPORAL DATA
  // ═══════════════════════════════════════
  uint32_t firstInteraction;      // Timestamp
  uint32_t lastInteraction;       // Timestamp
  uint32_t relationshipAge;       // Duration in seconds
};

class RelationshipGraph {
private:
  BotRelationship edges[512];     // N*(N-1)/2 relationships for N bots
  uint16_t edgeCount;
  
public:
  // Trust management
  void updateTrust(uint8_t* macA, uint8_t* macB, TrustEvent event);
  float getTrustScore(uint8_t* macA, uint8_t* macB);
  
  // Data verification
  void reportDataMatch(uint8_t* reporter, uint8_t* original);
  void reportDataMismatch(uint8_t* reporter, uint8_t* original, 
                          DataMismatchSeverity severity);
  
  // Collaboration tracking
  void recordCollaboration(uint8_t* macA, uint8_t* macB, bool success);
  float getCollaborationScore(uint8_t* macA, uint8_t* macB);
  
  // Network analysis
  BotProfile* getMostTrustedBot();
  BotProfile* getLeastTrustedBot();
  BotProfile** getPreferredPartners(uint8_t* mac, uint8_t* count);
  void detectIsolatedBots(BotProfile** isolated, uint8_t* count);
};

enum TrustEvent {
  TRUST_DATA_ACCURATE,        // +trust
  TRUST_DATA_INACCURATE,      // -trust
  TRUST_MISSION_SUCCESS,      // +trust
  TRUST_MISSION_FAILURE,      // -trust
  TRUST_RESPONSE_TIMELY,      // +trust
  TRUST_RESPONSE_DELAYED,     // -trust
  TRUST_HELP_PROVIDED,        // +trust
  TRUST_HELP_REFUSED          // -trust (if unjustified)
};
```

### Trust Calculation

```cpp
float calculateTrustScore(BotRelationship* rel) {
  // Component 1: Data accuracy (40%)
  float dataScore = rel->dataAgreementRate;
  
  // Component 2: Collaboration success (30%)
  float collabScore = 0.0;
  if (rel->missionsCompleted + rel->missionsFailed > 0) {
    collabScore = (float)rel->missionsCompleted / 
                  (rel->missionsCompleted + rel->missionsFailed);
  }
  
  // Component 3: Interaction consistency (20%)
  float consistencyScore = 0.0;
  if (rel->interactionCount > 0) {
    consistencyScore = (float)rel->successfulInteractions / 
                       rel->interactionCount;
  }
  
  // Component 4: Temporal factor (10%)
  // Recent interactions matter more
  uint32_t daysSinceLastInteraction = 
    (currentTime - rel->lastInteraction) / 86400;
  float recencyScore = 1.0 / (1.0 + daysSinceLastInteraction * 0.1);
  
  // Weighted combination
  float trustScore = 
    (dataScore * 0.40) +
    (collabScore * 0.30) +
    (consistencyScore * 0.20) +
    (recencyScore * 0.10);
  
  return trustScore;
}
```

---

## Data Quality Assessment

### Real-Time Data Verification

```cpp
struct DataVerificationEvent {
  uint8_t reportingBot[6];        // Who reported this data?
  uint8_t originalBot[6];         // Who generated original data?
  
  DataType dataType;              // OBSTACLE, POSITION, STATUS, etc.
  
  // Original data
  float originalValue[4];         // e.g., [x, y, distance, confidence]
  uint32_t originalTimestamp;
  
  // Verification data
  float verifiedValue[4];         // What reporter observed
  uint32_t verificationTimestamp;
  uint32_t verificationDelay;     // Time between original and verification
  
  // Discrepancy analysis
  float discrepancy;              // How different? (0.0-1.0)
  DiscrepancySeverity severity;   // MINOR, MODERATE, MAJOR, CRITICAL
  
  bool verified;                  // true = data confirmed, false = contradicted
};

enum DiscrepancySeverity {
  DISCREPANCY_NONE,       // <5% difference
  DISCREPANCY_MINOR,      // 5-15% difference
  DISCREPANCY_MODERATE,   // 15-30% difference
  DISCREPANCY_MAJOR,      // 30-50% difference
  DISCREPANCY_CRITICAL    // >50% difference
};

class DataQualityTracker {
public:
  void verifyObstacleReport(uint8_t* reporter, uint8_t* original,
                           ObstacleData* originalData,
                           ObstacleData* verifiedData);
  
  void verifyPositionReport(uint8_t* reporter, uint8_t* original,
                           Position* originalPos,
                           Position* verifiedPos);
  
  void updateBotDataAccuracy(uint8_t* mac);
  
  float calculateDiscrepancy(DataType type, 
                            float* original, 
                            float* verified);
  
  void flagSuspiciousBot(uint8_t* mac, SuspicionReason reason);
  
private:
  DataVerificationEvent history[1000];  // Ring buffer
  uint16_t historyIndex;
};
```

### Example: Obstacle Report Verification

```cpp
// SPEEDY reports obstacle at (5, 10)
void speedyReportsObstacle() {
  ObstacleMessage msg;
  msg.senderMAC = SPEEDY_MAC;
  msg.x = 5.0;
  msg.y = 10.0;
  msg.distance = 0.15;  // 15cm away
  msg.confidence = 0.95;
  msg.timestamp = millis();
  
  broadcastObstacle(&msg);
}

// WHEELIE receives and verifies
void wheelieVerifiesObstacle(ObstacleMessage* msg) {
  // Check if WHEELIE can verify (near location?)
  if (distanceToPoint(msg->x, msg->y) < 1.0) {  // Within 1m
    
    // Read own sensor
    int distance = readDistance();
    float myX, myY;
    getPosition(&myX, &myY);
    
    // Calculate where obstacle should be
    float expectedDistance = distanceToPoint(msg->x, msg->y);
    
    // Compare
    float discrepancy = abs(distance - expectedDistance);
    
    if (discrepancy < 0.05) {  // <5cm difference
      // GOOD DATA!
      relationshipGraph.reportDataMatch(myMAC, msg->senderMAC);
      dataQualityTracker.verifyObstacleReport(myMAC, msg->senderMAC,
                                              msg, &myReading);
      
      Serial.println("✅ SPEEDY's data verified - trustworthy!");
      
    } else {
      // BAD DATA!
      relationshipGraph.reportDataMismatch(myMAC, msg->senderMAC,
                                           DISCREPANCY_MAJOR);
      
      Serial.println("❌ SPEEDY's data contradicted - unreliable!");
      
      // Lower trust
      BotProfile* speedy = botRegistry.getBotByMAC(msg->senderMAC);
      speedy->dataAccuracy -= 0.05;  // Penalty
      speedy->reputationScore = calculateReputation(speedy);
      
      // If repeatedly bad, flag for inspection
      if (speedy->dataAccuracy < 0.60) {
        flagForInspection(speedy, REASON_POOR_DATA_QUALITY);
      }
    }
  }
}
```

---

## Weakness Detection

### Automated Health Monitoring

```cpp
class BotHealthMonitor {
public:
  void analyzeBotPerformance(BotProfile* bot);
  void detectDegradation(BotProfile* bot);
  void identifyFailingComponents(BotProfile* bot);
  void predictFailure(BotProfile* bot, FailurePrediction* prediction);
  
private:
  void checkSensorDrift(BotProfile* bot);
  void checkMotorPerformance(BotProfile* bot);
  void checkBatteryHealth(BotProfile* bot);
  void checkCommunicationQuality(BotProfile* bot);
};

struct FailurePrediction {
  ComponentType component;
  float failureProbability;     // 0.0-1.0 (probability in next 24h)
  uint32_t estimatedTimeToFailure;  // Seconds
  FailureMode likelyMode;       // How it will fail
  FailureSeverity severity;     // Impact on bot function
};

void BotHealthMonitor::detectDegradation(BotProfile* bot) {
  // ═══════════════════════════════════════
  // PERFORMANCE DEGRADATION DETECTION
  // ═══════════════════════════════════════
  
  // Check 1: Fitness decline over time
  if (bot->generation > 50) {  // Need history
    float recentFitness = getAverageFitness(bot, LAST_10_GENERATIONS);
    float historicalFitness = getAverageFitness(bot, GENERATIONS_20_TO_30);
    
    if (recentFitness < historicalFitness * 0.85) {  // 15% drop
      flagDegradation(bot, DEGRADATION_FITNESS_DECLINE);
      Serial.println("⚠️ Bot performance declining!");
    }
  }
  
  // Check 2: Sensor accuracy drift
  if (bot->dataAccuracy < bot->historicalDataAccuracy * 0.90) {
    flagDegradation(bot, DEGRADATION_SENSOR_DRIFT);
    Serial.println("⚠️ Sensor accuracy degrading!");
    
    // Identify which sensor
    checkSensorDrift(bot);
  }
  
  // Check 3: Motor degradation
  float currentSpeed = bot->currentGenome.motorSpeed;
  float actualSpeed = measureActualSpeed(bot);  // From IMU or timing
  
  if (actualSpeed < currentSpeed * 0.85) {  // Motor wearing out
    flagDegradation(bot, DEGRADATION_MOTOR_WEAR);
    Serial.println("⚠️ Motor performance degrading!");
  }
  
  // Check 4: Battery capacity loss
  if (bot->batteryRuntime < bot->historicalBatteryRuntime * 0.80) {
    flagDegradation(bot, DEGRADATION_BATTERY_AGING);
    Serial.println("⚠️ Battery capacity declining!");
  }
  
  // Check 5: Communication reliability
  if (bot->communicationReliability < 0.85) {
    flagDegradation(bot, DEGRADATION_COMM_ISSUES);
    Serial.println("⚠️ Communication quality degrading!");
  }
  
  // Check 6: Mission success rate decline
  float recentSuccessRate = getMissionSuccessRate(bot, LAST_20_MISSIONS);
  float historicalSuccessRate = bot->missionSuccessRate;
  
  if (recentSuccessRate < historicalSuccessRate * 0.80) {
    flagDegradation(bot, DEGRADATION_MISSION_FAILURES);
    Serial.println("⚠️ Mission success rate declining!");
  }
}
```

### Weakness Report Generation

```cpp
struct WeaknessReport {
  BotProfile* bot;
  uint32_t reportTimestamp;
  
  // Identified weaknesses
  uint8_t weaknessCount;
  Weakness weaknesses[16];
  
  // Overall assessment
  BotHealth overallHealth;
  float degradationRate;          // How fast is it getting worse?
  uint32_t estimatedUsefulLife;   // Hours until critical
  
  // Recommendations
  bool recommendUpgrade;
  bool recommendMaintenance;
  bool recommendRetirement;
  bool recommendReplacement;
  
  // Specific actions
  uint8_t actionCount;
  RecommendedAction actions[8];
};

struct Weakness {
  ComponentType component;
  WeaknessType type;
  float severity;                 // 0.0-1.0
  const char* description;
  
  // Impact
  float performanceImpact;        // How much does this hurt?
  float reliabilityImpact;
  float safetyImpact;
  
  // Remedy
  RemedyType recommendedRemedy;
  float remedyCost;               // Estimated effort/cost
  float remodyBenefit;            // Expected improvement
};

enum WeaknessType {
  WEAKNESS_SENSOR_DRIFT,
  WEAKNESS_SENSOR_NOISE,
  WEAKNESS_SENSOR_BLIND_SPOT,
  WEAKNESS_MOTOR_WEAR,
  WEAKNESS_MOTOR_IMBALANCE,
  WEAKNESS_BATTERY_AGING,
  WEAKNESS_COMMUNICATION_POOR,
  WEAKNESS_STRATEGY_SUBOPTIMAL,
  WEAKNESS_STRUCTURAL_DAMAGE,
  WEAKNESS_SOFTWARE_BUG
};

enum RemedyType {
  REMEDY_CALIBRATE,      // Recalibrate sensors
  REMEDY_REPLACE_PART,   // Replace worn component
  REMEDY_SOFTWARE_UPDATE,// Update firmware
  REMEDY_PARAMETER_TUNE, // Adjust genome parameters
  REMEDY_ADD_SENSOR,     // Upgrade with new sensor
  REMEDY_RETIRE,         // Take out of service
  REMEDY_FULL_REBUILD    // Major overhaul needed
};
```

---

## Upgrade Recommendations

### Intelligent Upgrade Advisor

```cpp
class UpgradeAdvisor {
public:
  void analyzeBot(BotProfile* bot, UpgradeRecommendations* recs);
  void analyzSwarm(SwarmUpgradeReport* report);
  
private:
  void recommendSensorUpgrades(BotProfile* bot, UpgradeRecommendations* recs);
  void recommendHardwareUpgrades(BotProfile* bot, UpgradeRecommendations* recs);
  void recommendSoftwareUpgrades(BotProfile* bot, UpgradeRecommendations* recs);
  void recommendRoleChange(BotProfile* bot, UpgradeRecommendations* recs);
};

struct UpgradeRecommendations {
  BotProfile* bot;
  
  // Sensor upgrades
  uint8_t sensorUpgradeCount;
  SensorUpgrade sensorUpgrades[4];
  
  // Hardware upgrades
  uint8_t hardwareUpgradeCount;
  HardwareUpgrade hardwareUpgrades[4];
  
  // Software upgrades
  bool needsFirmwareUpdate;
  const char* firmwareVersion;
  
  // Role changes
  bool recommendRoleChange;
  BotType currentRole;
  BotType recommendedRole;
  const char* roleChangeReason;
  
  // Priority
  UpgradePriority priority;
  float expectedFitnessIncrease;
  float estimatedCost;
  float roi;  // Return on investment
};

struct SensorUpgrade {
  SensorType currentSensor;
  SensorType recommendedSensor;
  const char* reason;
  float performanceGain;
  float cost;
};

struct HardwareUpgrade {
  ComponentType component;
  const char* currentSpec;
  const char* recommendedSpec;
  const char* reason;
  float performanceGain;
  float cost;
};

enum UpgradePriority {
  PRIORITY_CRITICAL,     // Do immediately (safety/reliability)
  PRIORITY_HIGH,         // Do soon (significant performance gain)
  PRIORITY_MEDIUM,       // Do when convenient (moderate gain)
  PRIORITY_LOW,          // Nice to have (small gain)
  PRIORITY_OPTIONAL      // Only if excess resources
};
```

### Example: Upgrade Analysis

```cpp
void UpgradeAdvisor::analyzeBot(BotProfile* bot, 
                                UpgradeRecommendations* recs) {
  Serial.println("═══════════════════════════════════");
  Serial.print("ANALYZING BOT: ");
  Serial.println(bot->botName);
  Serial.println("═══════════════════════════════════");
  
  // SENSOR ANALYSIS
  if (bot->botType == BOT_SCOUT && bot->dataAccuracy < 0.85) {
    // WHEELIE/SPEEDY with poor sensor accuracy
    
    if (hasSensor(bot, SENSOR_VL53L0X)) {
      // VL53L0X might be degraded
      SensorUpgrade upgrade;
      upgrade.currentSensor = SENSOR_VL53L0X;
      upgrade.recommendedSensor = SENSOR_VL53L1X;  // Newer model
      upgrade.reason = "Current ToF sensor showing drift. VL53L1X has "
                      "better accuracy and longer range.";
      upgrade.performanceGain = 0.15;  // 15% improvement expected
      upgrade.cost = 12.0;  // $12
      
      recs->sensorUpgrades[recs->sensorUpgradeCount++] = upgrade;
      
      Serial.println("📊 SENSOR UPGRADE RECOMMENDED:");
      Serial.println("  Current: VL53L0X");
      Serial.println("  Upgrade to: VL53L1X");
      Serial.println("  Reason: Sensor drift detected");
      Serial.println("  Expected gain: +15% accuracy");
    }
    
    // Maybe needs additional sensor for redundancy
    if (!hasSensor(bot, SENSOR_ULTRASONIC)) {
      SensorUpgrade upgrade;
      upgrade.currentSensor = SENSOR_NONE;
      upgrade.recommendedSensor = SENSOR_ULTRASONIC;
      upgrade.reason = "Add HC-SR04 for sensor fusion and redundancy. "
                      "Wide field of view complements laser sensor.";
      upgrade.performanceGain = 0.20;
      upgrade.cost = 2.0;
      
      recs->sensorUpgrades[recs->sensorUpgradeCount++] = upgrade;
      
      Serial.println("📊 ADDITIONAL SENSOR RECOMMENDED:");
      Serial.println("  Add: HC-SR04 Ultrasonic");
      Serial.println("  Reason: Sensor fusion + redundancy");
      Serial.println("  Expected gain: +20% reliability");
    }
  }
  
  // MOTOR ANALYSIS
  if (measureActualSpeed(bot) < bot->currentGenome.motorSpeed * 0.85) {
    HardwareUpgrade upgrade;
    upgrade.component = COMPONENT_MOTOR;
    upgrade.currentSpec = "TT Gear Motor 1:48";
    upgrade.recommendedSpec = "TT Gear Motor 1:48 (new) or upgrade to higher quality";
    upgrade.reason = "Motors showing wear. Performance degraded by 15%+.";
    upgrade.performanceGain = 0.18;
    upgrade.cost = 10.0;
    
    recs->hardwareUpgrades[recs->hardwareUpgradeCount++] = upgrade;
    
    Serial.println("⚙️ HARDWARE UPGRADE RECOMMENDED:");
    Serial.println("  Component: Motors");
    Serial.println("  Reason: Wear detected");
    Serial.println("  Action: Replace motors");
    Serial.println("  Expected gain: +18% speed");
  }
  
  // ROLE ANALYSIS
  if (bot->botType == BOT_SCOUT && 
      bot->fitness < 0.60 && 
      bot->generation > 100) {
    // Evolved for 100+ generations but still low fitness
    // Maybe wrong role?
    
    if (bot->currentGenome.motorSpeed < 150) {
      // Evolved to be slow - maybe better as manipulator?
      recs->recommendRoleChange = true;
      recs->currentRole = BOT_SCOUT;
      recs->recommendedRole = BOT_MANIPULATOR;
      recs->roleChangeReason = "Bot evolved for low-speed precision. "
                              "Would perform better in manipulator role.";
      
      Serial.println("🔄 ROLE CHANGE RECOMMENDED:");
      Serial.println("  Current: Scout");
      Serial.println("  Recommended: Manipulator");
      Serial.println("  Reason: Evolved personality mismatch");
    }
  }
  
  // PRIORITY ASSESSMENT
  if (bot->health == HEALTH_CRITICAL) {
    recs->priority = PRIORITY_CRITICAL;
  } else if (bot->dataAccuracy < 0.70 || bot->missionSuccessRate < 0.60) {
    recs->priority = PRIORITY_HIGH;
  } else if (bot->fitness < bot->swarmAverageFitness * 0.85) {
    recs->priority = PRIORITY_MEDIUM;
  } else {
    recs->priority = PRIORITY_LOW;
  }
  
  // ROI CALCULATION
  float totalCost = 0;
  float totalGain = 0;
  
  for (int i = 0; i < recs->sensorUpgradeCount; i++) {
    totalCost += recs->sensorUpgrades[i].cost;
    totalGain += recs->sensorUpgrades[i].performanceGain;
  }
  
  for (int i = 0; i < recs->hardwareUpgradeCount; i++) {
    totalCost += recs->hardwareUpgrades[i].cost;
    totalGain += recs->hardwareUpgrades[i].performanceGain;
  }
  
  recs->estimatedCost = totalCost;
  recs->expectedFitnessIncrease = totalGain;
  recs->roi = (totalGain * bot->fitness) / (totalCost + 1);
  
  Serial.println("═══════════════════════════════════");
  Serial.print("TOTAL ESTIMATED COST: $");
  Serial.println(totalCost);
  Serial.print("EXPECTED FITNESS GAIN: +");
  Serial.print(totalGain * 100);
  Serial.println("%");
  Serial.print("ROI: ");
  Serial.println(recs->roi);
  Serial.println("═══════════════════════════════════\n");
}
```

---

## New Bot Creation

### Swarm Composition Analysis

```cpp
class SwarmCompositionAnalyzer {
public:
  void analyzeSwarmNeeds(SwarmNeeds* needs);
  void recommendNewBot(NewBotRecommendation* rec);
  
private:
  void identifyCapabilityGaps(SwarmNeeds* needs);
  void analyzeMissionFailures(SwarmNeeds* needs);
  void assessWorkloadBalance(SwarmNeeds* needs);
  void detectBottlenecks(SwarmNeeds* needs);
};

struct SwarmNeeds {
  // Current swarm composition
  uint8_t scoutCount;
  uint8_t manipulatorCount;
  uint8_t heavyCount;
  uint8_t aerialCount;
  
  // Capability gaps
  bool needsFastScout;           // No high-speed bot
  bool needsPrecisionManipulator;// No gripper bot
  bool needsHeavyLifter;         // Can't move heavy objects
  bool needsAerialRecon;         // No 3D vision
  bool needsSpecialist;          // Custom requirement
  
  // Performance analysis
  float averageSwarmFitness;
  float lowestBotFitness;
  float missionSuccessRate;
  
  // Mission analysis
  uint32_t missionsAttempted;
  uint32_t missionsSucceeded;
  uint32_t missionsFailedCapability;  // Failed due to lacking capability
  uint32_t missionsFailedPerformance; // Failed due to poor performance
  
  // Bottleneck analysis
  BotType bottleneckRole;        // Which role is overworked?
  float bottleneckSeverity;      // How bad is it?
  
  // Recommendation priority
  RecommendationPriority priority;
  const char* urgencyReason;
};

struct NewBotRecommendation {
  BotType recommendedType;
  const char* botName;
  const char* purpose;
  
  // Specifications
  SensorType requiredSensors[8];
  uint8_t sensorCount;
  ActuatorType requiredActuators[4];
  uint8_t actuatorCount;
  
  // Performance targets
  float targetSpeed;
  float targetPrecision;
  float targetPayload;
  float targetEndurance;
  
  // Expected impact
  float expectedSwarmFitnessIncrease;
  float expectedMissionSuccessIncrease;
  const char* primaryBenefit;
  
  // Build guidance
  float estimatedCost;
  uint32_t estimatedBuildTime;  // Hours
  DifficultyLevel buildDifficulty;
  const char* buildGuideURL;
};
```

### Example: Identifying Need for New Bot

```cpp
void SwarmCompositionAnalyzer::analyzeSwarmNeeds(SwarmNeeds* needs) {
  Serial.println("\n");
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║   SWARM COMPOSITION ANALYSIS           ║");
  Serial.println("╚════════════════════════════════════════╝");
  
  // Current composition
  needs->scoutCount = 2;         // WHEELIE, SPEEDY
  needs->manipulatorCount = 0;   // (GRABBER on hold)
  needs->heavyCount = 0;
  needs->aerialCount = 0;
  
  Serial.println("\n📊 CURRENT SWARM COMPOSITION:");
  Serial.print("  Scouts: ");
  Serial.println(needs->scoutCount);
  Serial.print("  Manipulators: ");
  Serial.println(needs->manipulatorCount);
  Serial.print("  Heavy: ");
  Serial.println(needs->heavyCount);
  Serial.print("  Aerial: ");
  Serial.println(needs->aerialCount);
  
  // Analyze recent missions
  analyzeMissionFailures(needs);
  
  // Check for capability gaps
  identifyCapabilityGaps(needs);
  
  // Example findings:
  if (needs->missionsFailedCapability > 5 && 
      needs->manipulatorCount == 0) {
    
    needs->needsPrecisionManipulator = true;
    needs->priority = PRIORITY_HIGH;
    needs->urgencyReason = "Multiple missions failed due to inability "
                          "to manipulate objects. Need GRABBER bot.";
    
    Serial.println("\n⚠️ CRITICAL CAPABILITY GAP DETECTED!");
    Serial.println("  Missing: Manipulation capability");
    Serial.println("  Impact: 5+ mission failures");
    Serial.println("  Recommendation: Build GRABBER bot (manipulator)");
  }
  
  // Check workload balance
  assessWorkloadBalance(needs);
  
  if (needs->bottleneckRole == BOT_SCOUT && 
      needs->bottleneckSeverity > 0.70) {
    
    needs->needsFastScout = true;
    needs->priority = PRIORITY_MEDIUM;
    needs->urgencyReason = "Scout bots are bottleneck. 70% of time spent "
                          "scouting, delaying other tasks.";
    
    Serial.println("\n⚠️ WORKLOAD BOTTLENECK DETECTED!");
    Serial.println("  Bottleneck role: Scout");
    Serial.println("  Severity: 70%");
    Serial.println("  Recommendation: Add additional scout bot");
  }
  
  // Check for performance gaps
  if (needs->averageSwarmFitness < 0.65 && needs->scoutCount < 3) {
    needs->needsFastScout = true;
    needs->priority = PRIORITY_MEDIUM;
    needs->urgencyReason = "Low swarm fitness. Additional scout would "
                          "improve exploration coverage.";
  }
  
  Serial.println("\n═══════════════════════════════════════");
  Serial.println("ANALYSIS COMPLETE");
  Serial.println("═══════════════════════════════════════\n");
}

void SwarmCompositionAnalyzer::recommendNewBot(NewBotRecommendation* rec) {
  SwarmNeeds needs;
  analyzeSwarmNeeds(&needs);
  
  if (needs.needsPrecisionManipulator) {
    // CRITICAL: Need GRABBER
    rec->recommendedType = BOT_MANIPULATOR;
    rec->botName = "GRABBER";
    rec->purpose = "Object manipulation, precision tasks, assembly";
    
    // Sensors
    rec->requiredSensors[0] = SENSOR_CURRENT;      // Grip detection
    rec->requiredSensors[1] = SENSOR_VL53L0X;      // Distance
    rec->requiredSensors[2] = SENSOR_IMU;          // Stability
    rec->sensorCount = 3;
    
    // Actuators
    rec->requiredActuators[0] = ACTUATOR_SERVO;    // Gripper
    rec->requiredActuators[1] = ACTUATOR_SERVO;    // Arm
    rec->requiredActuators[2] = ACTUATOR_MOTOR;    // Wheels
    rec->actuatorCount = 3;
    
    // Targets
    rec->targetSpeed = 0.5;           // Slow, precise
    rec->targetPrecision = 0.95;      // Very precise
    rec->targetPayload = 0.25;        // 250g
    rec->targetEndurance = 2.0;       // 2 hours
    
    // Impact
    rec->expectedSwarmFitnessIncrease = 0.25;
    rec->expectedMissionSuccessIncrease = 0.40;
    rec->primaryBenefit = "Enables manipulation tasks, object assembly, "
                         "tool use. Opens new mission types.";
    
    // Build info
    rec->estimatedCost = 65.0;
    rec->estimatedBuildTime = 8;      // 8 hours
    rec->buildDifficulty = DIFFICULTY_MEDIUM;
    rec->buildGuideURL = "docs.projectjumbo.org/build-grabber";
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║   NEW BOT RECOMMENDATION               ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println();
    Serial.println("🤖 RECOMMENDED BOT: GRABBER");
    Serial.println("   Type: Manipulator");
    Serial.println("   Purpose: Object manipulation & assembly");
    Serial.println();
    Serial.println("📦 REQUIRED HARDWARE:");
    Serial.println("   • Current sensor (grip detection)");
    Serial.println("   • VL53L0X (distance)");
    Serial.println("   • IMU (stability)");
    Serial.println("   • 2× Servos (gripper + arm)");
    Serial.println("   • 2× Motors (locomotion)");
    Serial.println();
    Serial.println("🎯 PERFORMANCE TARGETS:");
    Serial.print("   Speed: ");
    Serial.print(rec->targetSpeed);
    Serial.println(" m/s (slow, precise)");
    Serial.print("   Precision: ");
    Serial.print(rec->targetPrecision * 100);
    Serial.println("% (very high)");
    Serial.print("   Payload: ");
    Serial.print(rec->targetPayload * 1000);
    Serial.println("g");
    Serial.println();
    Serial.println("📈 EXPECTED IMPACT:");
    Serial.print("   Swarm fitness: +");
    Serial.print(rec->expectedSwarmFitnessIncrease * 100);
    Serial.println("%");
    Serial.print("   Mission success: +");
    Serial.print(rec->expectedMissionSuccessIncrease * 100);
    Serial.println("%");
    Serial.print("   Primary benefit: ");
    Serial.println(rec->primaryBenefit);
    Serial.println();
    Serial.println("💰 BUILD INFORMATION:");
    Serial.print("   Estimated cost: $");
    Serial.println(rec->estimatedCost);
    Serial.print("   Build time: ");
    Serial.print(rec->estimatedBuildTime);
    Serial.println(" hours");
    Serial.print("   Difficulty: ");
    Serial.println(getDifficultyString(rec->buildDifficulty));
    Serial.print("   Guide: ");
    Serial.println(rec->buildGuideURL);
    Serial.println();
    Serial.println("═══════════════════════════════════════");
  }
  
  // Additional recommendations...
}
```

---

## Implementation

### MCU/Hub Integration

```cpp
// On Raspberry Pi Hub / PC MCU
class SwarmIntelligenceManager {
private:
  BotRegistry botRegistry;
  RelationshipGraph relationshipGraph;
  DataQualityTracker dataQualityTracker;
  BotHealthMonitor healthMonitor;
  UpgradeAdvisor upgradeAdvisor;
  SwarmCompositionAnalyzer compositionAnalyzer;
  
public:
  void initialize();
  void update();  // Called every second
  
  // Bot management
  void onBotConnect(uint8_t* mac, BotProfile* profile);
  void onBotDisconnect(uint8_t* mac);
  void onBotStatusUpdate(uint8_t* mac, StatusUpdate* update);
  
  // Data verification
  void onDataReport(uint8_t* mac, DataReport* data);
  void verifyDataCrossCheck(DataReport* data);
  
  // Relationship management
  void onCollaboration(uint8_t* macA, uint8_t* macB, bool success);
  void updateRelationships();
  
  // Health monitoring
  void monitorBotHealth();
  void generateHealthReports();
  
  // Recommendations
  void analyzeUpgradeNeeds();
  void analyzeCompositionNeeds();
  void generateRecommendations();
  
  // Reporting
  void printSwarmStatus();
  void printBotDetails(uint8_t* mac);
  void printRelationshipGraph();
  void printRecommendations();
};

void SwarmIntelligenceManager::update() {
  static uint32_t lastUpdate = 0;
  static uint32_t updateCounter = 0;
  
  if (millis() - lastUpdate < 1000) return;  // 1Hz update
  lastUpdate = millis();
  updateCounter++;
  
  // Every second: Update relationships
  updateRelationships();
  
  // Every 10 seconds: Check data quality
  if (updateCounter % 10 == 0) {
    for (int i = 0; i < botRegistry.botCount; i++) {
      dataQualityTracker.updateBotDataAccuracy(
        botRegistry.bots[i].botMAC
      );
    }
  }
  
  // Every minute: Monitor health
  if (updateCounter % 60 == 0) {
    monitorBotHealth();
  }
  
  // Every 10 minutes: Analyze upgrades
  if (updateCounter % 600 == 0) {
    analyzeUpgradeNeeds();
  }
  
  // Every hour: Analyze composition
  if (updateCounter % 3600 == 0) {
    analyzeCompositionNeeds();
    generateRecommendations();
  }
  
  // Every hour: Print status
  if (updateCounter % 3600 == 0) {
    printSwarmStatus();
  }
}
```

---

## Example Scenarios

### Scenario 1: Detecting Unreliable Bot

```txt
Hour 0: SPEEDY joins swarm
  - Reputation: 0.50 (neutral, new bot)
  - Trust with peers: None yet

Hour 1: SPEEDY reports 15 obstacles
  - WHEELIE verifies: 12 accurate, 3 inaccurate
  - Data accuracy: 80%
  - Reputation: 0.65 (decent)

Hour 2: SPEEDY reports 20 obstacles
  - WHEELIE verifies: 10 accurate, 10 inaccurate
  - Data accuracy drops to: 55%
  - Reputation drops to: 0.48
  - ⚠️ System flags SPEEDY for inspection

Hour 3: Human inspects SPEEDY
  - Discovers VL53L0X sensor has loose wire!
  - Repairs connection
  - Sensor recalibrated

Hour 4: SPEEDY reports 18 obstacles
  - WHEELIE verifies: 17 accurate, 1 inaccurate
  - Data accuracy recovers to: 94%
  - Reputation increases to: 0.72
  - ✅ System marks SPEEDY as reliable again

Result: Bad data detected → Bot flagged → Human fixed → 
        Quality restored → Trust rebuilt
```

### Scenario 2: Identifying Need for New Bot

```txt
Week 1: Swarm has 2 scouts (WHEELIE, SPEEDY)
  - 20 missions attempted
  - 15 completed (75% success)
  - 5 failed due to: "Cannot manipulate target object"

Week 2: More failures
  - 25 missions attempted
  - 16 completed (64% success)
  - 9 failed: 8 due to manipulation, 1 other

System Analysis:
  ⚠️ Capability gap detected!
  ⚠️ 13 missions failed: lack of manipulation
  ⚠️ Mission success declining
  
  📊 RECOMMENDATION:
  Build GRABBER bot (manipulator role)
  - Estimated impact: +40% mission success
  - Priority: HIGH
  - Cost: $65
  - Time: 8 hours

Week 3: Human builds GRABBER
  - Joins swarm
  - Initial reputation: 0.50

Week 4: GRABBER operational
  - 30 missions attempted
  - 27 completed (90% success!)
  - Only 3 failures (unrelated to manipulation)
  
Result: System identified gap → Recommended solution → 
        Human built bot → Problem solved
```

### Scenario 3: Detecting Motor Degradation

```txt
Month 1: WHEELIE performing well
  - Fitness: 0.78
  - Motor speed genome: 235
  - Actual speed: 232 (98.7% efficiency)
  - Data accuracy: 92%

Month 2: Slight decline
  - Fitness: 0.74
  - Motor speed genome: 235
  - Actual speed: 220 (93.6% efficiency)
  - System notes 5% performance drop

Month 3: Continued decline
  - Fitness: 0.68
  - Motor speed genome: 235
  - Actual speed: 198 (84.3% efficiency)
  - ⚠️ System flags motor degradation!

System Analysis:
  Component: Motors
  Issue: 15% performance loss
  Cause: Bearing wear, gear degradation
  
  📊 RECOMMENDATION:
  Replace both TT motors
  - Cost: $10
  - Time: 30 minutes
  - Expected recovery: +15% fitness

Human replaces motors

Month 4: Recovered
  - Fitness: 0.77
  - Motor speed genome: 235
  - Actual speed: 231 (98.3% efficiency)
  - ✅ Performance restored!

Result: Gradual degradation detected → Root cause identified → 
        Maintenance performed → Performance recovered
```

---

## Summary

**The Swarm Ecosystem Manager provides:**

1. ✅ **Bot Identity & Tracking** - Every bot has a profile, history, reputation
2. ✅ **Relationship Management** - Bots track peer reliability through data verification
3. ✅ **Data Quality Assessment** - Bad data = lowered trust = isolation
4. ✅ **Weakness Detection** - Automatic monitoring spots degradation early
5. ✅ **Upgrade Recommendations** - System suggests specific improvements
6. ✅ **New Bot Creation** - Identifies capability gaps, recommends new bots

**Result: A self-aware, self-improving ecosystem that knows:**

- Which bots are reliable
- Which bots need help
- What the swarm is missing
- How to get better

**It's not just robots. It's an intelligent interpretive ecosystem.** 🌐🤖

---

### "In nature, ecosystems don't just contain life—they evaluate it, adapt to it, and evolve with it. So should our robot swarms."

### "This is the guiding philosophy behind the Swarm Ecosystem Manager."
