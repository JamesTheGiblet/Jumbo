#include "swarm_ecosystem_manager.h"
#include <EEPROM.h>
#include <WiFi.h>

// ═══════════════════════════════════════════════════════════
// 🌐 SWARM ECOSYSTEM MANAGER IMPLEMENTATION
// ═══════════════════════════════════════════════════════════

// Global ecosystem manager instance
SwarmEcosystemManager* ecosystemManager = nullptr;

SwarmEcosystemManager::SwarmEcosystemManager() {
  botCount = 0;
  relationshipCount = 0;
  verificationLogIndex = 0;
  lastReputationUpdate = 0;
  lastEcosystemAnalysis = 0;
}

void SwarmEcosystemManager::initialize() {
  Serial.println("🌐 Initializing Swarm Ecosystem Manager...");
  
  // Clear all profiles and relationships
  memset(botProfiles, 0, sizeof(botProfiles));
  memset(relationships, 0, sizeof(relationships));
  memset(verificationLog, 0, sizeof(verificationLog));
  
  // Initialize timing
  lastReputationUpdate = millis();
  lastEcosystemAnalysis = millis();
  
  Serial.printf("🌐 Ecosystem Manager ready (Max %d bots, %d relationships)\n", 
                MAX_BOT_PROFILES, MAX_RELATIONSHIPS);
}

// ═══════════════════════════════════════════════════════════
// 🤖 BOT PROFILE MANAGEMENT
// ═══════════════════════════════════════════════════════════

void SwarmEcosystemManager::registerBot(uint8_t* mac, BotType type, const char* name) {
  // Check if bot already exists
  BotProfile* existing = getBotProfile(mac);
  if (existing != nullptr) {
    Serial.printf("🔄 Bot %s already registered, updating...\n", name);
    existing->lastSeenTimestamp = millis();
    return;
  }
  
  if (botCount >= MAX_BOT_PROFILES) {
    Serial.println("⚠️ Bot registry full - cannot register new bot");
    return;
  }
  
  BotProfile* profile = &botProfiles[botCount];
  
  // Initialize identity
  memcpy(profile->botMAC, mac, 6);
  strncpy(profile->botName, name, sizeof(profile->botName) - 1);
  profile->botType = type;
  profile->serialNumber = botCount + 1000; // Simple serial numbering
  profile->activationTimestamp = millis();
  
  // Initialize metrics to reasonable defaults
  profile->generation = 0;
  profile->fitness = 0.0f;
  profile->totalRuntime = 0;
  profile->totalMissions = 0;
  
  // Initialize reliability metrics
  profile->dataAccuracy = 0.8f;           // Start with moderate trust
  profile->missionSuccessRate = 0.8f;
  profile->communicationReliability = 1.0f;
  profile->availabilityScore = 1.0f;
  
  profile->totalDataSent = 0;
  profile->dataVerifiedGood = 0;
  profile->dataVerifiedBad = 0;
  
  // Initialize reputation
  profile->reputationScore = 50.0f;       // Start neutral
  profile->lastReputationUpdate = millis();
  
  // Initialize health
  profile->health = HEALTH_GOOD;          // Assume new bots are healthy
  for (int i = 0; i < 4; i++) {
    profile->sensorHealth[i] = COMPONENT_OPERATIONAL;
  }
  profile->estimatedRemainingLife = 1000.0f; // 1000 hours estimated life
  
  // Initialize ecosystem tracking
  profile->lastSeenTimestamp = millis();
  profile->consecutiveFailures = 0;
  profile->needsInspection = false;
  profile->isBlacklisted = false;
  
  botCount++;
  
  Serial.printf("✅ Registered bot %s (MAC: %s, Type: %d)\n", 
                name, WiFi.macAddress().c_str(), type);
}

void SwarmEcosystemManager::updateBotStatus(uint8_t* mac, uint32_t generation, float fitness) {
  BotProfile* profile = getBotProfile(mac);
  if (profile == nullptr) return;
  
  profile->generation = generation;
  profile->fitness = fitness;
  profile->lastSeenTimestamp = millis();
  profile->consecutiveFailures = 0; // Reset failure count on successful communication
  
  // Update runtime estimation
  profile->totalRuntime = (millis() - profile->activationTimestamp) / 3600000; // Convert to hours
}

void SwarmEcosystemManager::updateBotHealth(uint8_t* mac, BotHealth health) {
  BotProfile* profile = getBotProfile(mac);
  if (profile == nullptr) return;
  
  profile->health = health;
  profile->lastSeenTimestamp = millis();
  
  // Flag for inspection if health is poor
  if (health <= HEALTH_FAILING) {
    profile->needsInspection = true;
    Serial.printf("⚠️ Bot %s health degraded to %d - needs inspection\n", 
                  profile->botName, health);
  }
}

BotProfile* SwarmEcosystemManager::getBotProfile(uint8_t* mac) {
  for (uint8_t i = 0; i < botCount; i++) {
    if (memcmp(botProfiles[i].botMAC, mac, 6) == 0) {
      return &botProfiles[i];
    }
  }
  return nullptr;
}

void SwarmEcosystemManager::deactivateBot(uint8_t* mac) {
  BotProfile* profile = getBotProfile(mac);
  if (profile == nullptr) return;
  
  Serial.printf("📴 Deactivating bot %s\n", profile->botName);
  
  // Mark all relationships with this bot as inactive
  for (uint8_t i = 0; i < relationshipCount; i++) {
    if (memcmp(relationships[i].botA_MAC, mac, 6) == 0 || 
        memcmp(relationships[i].botB_MAC, mac, 6) == 0) {
      relationships[i].isActive = false;
    }
  }
  
  // Remove from bot registry by shifting array
  for (uint8_t i = 0; i < botCount; i++) {
    if (memcmp(botProfiles[i].botMAC, mac, 6) == 0) {
      // Shift remaining profiles down
      for (uint8_t j = i; j < botCount - 1; j++) {
        botProfiles[j] = botProfiles[j + 1];
      }
      botCount--;
      break;
    }
  }
}

// ═══════════════════════════════════════════════════════════
// 🤝 TRUST & RELATIONSHIP MANAGEMENT
// ═══════════════════════════════════════════════════════════

void SwarmEcosystemManager::recordInteraction(uint8_t* botA, uint8_t* botB, 
                                            InteractionType type, InteractionResult result) {
  // Find or create relationship
  BotRelationship* relationship = nullptr;
  
  for (uint8_t i = 0; i < relationshipCount; i++) {
    if ((memcmp(relationships[i].botA_MAC, botA, 6) == 0 && 
         memcmp(relationships[i].botB_MAC, botB, 6) == 0) ||
        (memcmp(relationships[i].botA_MAC, botB, 6) == 0 && 
         memcmp(relationships[i].botB_MAC, botA, 6) == 0)) {
      relationship = &relationships[i];
      break;
    }
  }
  
  // Create new relationship if not found
  if (relationship == nullptr && relationshipCount < MAX_RELATIONSHIPS) {
    relationship = &relationships[relationshipCount];
    memcpy(relationship->botA_MAC, botA, 6);
    memcpy(relationship->botB_MAC, botB, 6);
    relationship->trustScore = 0.5f; // Start neutral
    relationship->interactionCount = 0;
    relationship->successfulInteractions = 0;
    relationship->failedInteractions = 0;
    relationship->interactionIndex = 0;
    relationship->isActive = true;
    relationshipCount++;
  }
  
  if (relationship == nullptr) return; // Couldn't create relationship
  
  // Record the interaction
  relationship->recentInteractions[relationship->interactionIndex] = type;
  relationship->recentResults[relationship->interactionIndex] = result;
  relationship->recentTimestamps[relationship->interactionIndex] = millis();
  relationship->interactionIndex = (relationship->interactionIndex + 1) % 10;
  
  relationship->interactionCount++;
  if (result == RESULT_SUCCESS || result == RESULT_PARTIAL) {
    relationship->successfulInteractions++;
  } else {
    relationship->failedInteractions++;
  }
  
  // Update trust score based on interaction result
  float trustDelta = 0.0f;
  switch (result) {
    case RESULT_SUCCESS:
      trustDelta = 0.05f;  // Small positive adjustment
      break;
    case RESULT_PARTIAL:
      trustDelta = 0.01f;  // Tiny positive adjustment
      break;
    case RESULT_FAILURE:
      trustDelta = -0.03f; // Small negative adjustment
      break;
    case RESULT_CONTRADICTED:
      trustDelta = -0.1f;  // Large negative adjustment for bad data
      break;
  }
  
  relationship->trustScore = constrain(relationship->trustScore + trustDelta, 0.0f, 1.0f);
  relationship->lastInteraction = millis();
  
  Serial.printf("🤝 Recorded interaction between bots (Type: %d, Result: %d, New trust: %.3f)\n", 
                type, result, relationship->trustScore);
}

void SwarmEcosystemManager::recordDataVerification(uint8_t* sender, uint8_t* verifier, 
                                                 uint32_t dataHash, bool isCorrect, float confidence) {
  // Add to verification log
  DataVerificationEntry* entry = &verificationLog[verificationLogIndex];
  memcpy(entry->senderMAC, sender, 6);
  memcpy(entry->verifierMAC, verifier, 6);
  entry->dataHash = dataHash;
  entry->isVerification = isCorrect;
  entry->timestamp = millis();
  entry->dataConfidence = confidence;
  
  verificationLogIndex = (verificationLogIndex + 1) % 100;
  
  // Update bot profile data accuracy
  BotProfile* senderProfile = getBotProfile(sender);
  if (senderProfile != nullptr) {
    senderProfile->totalDataSent++;
    if (isCorrect) {
      senderProfile->dataVerifiedGood++;
    } else {
      senderProfile->dataVerifiedBad++;
    }
    
    // Recalculate data accuracy
    updateDataAccuracy(sender);
  }
  
  // Record interaction between sender and verifier
  InteractionResult result = isCorrect ? RESULT_SUCCESS : RESULT_CONTRADICTED;
  recordInteraction(sender, verifier, INTERACTION_DATA_SHARE, result);
}

float SwarmEcosystemManager::getTrustScore(uint8_t* botA, uint8_t* botB) {
  for (uint8_t i = 0; i < relationshipCount; i++) {
    if ((memcmp(relationships[i].botA_MAC, botA, 6) == 0 && 
         memcmp(relationships[i].botB_MAC, botB, 6) == 0) ||
        (memcmp(relationships[i].botA_MAC, botB, 6) == 0 && 
         memcmp(relationships[i].botB_MAC, botA, 6) == 0)) {
      return relationships[i].trustScore;
    }
  }
  return 0.5f; // Default neutral trust for unknown relationships
}

bool SwarmEcosystemManager::shouldTrustBot(uint8_t* mac, float minTrustThreshold) {
  BotProfile* profile = getBotProfile(mac);
  if (profile == nullptr) return false;
  
  if (profile->isBlacklisted) return false;
  
  // Check overall reputation
  if (profile->reputationScore < (minTrustThreshold * 100.0f)) return false;
  
  // Check data accuracy
  if (profile->dataAccuracy < minTrustThreshold) return false;
  
  // Check health status
  if (profile->health <= HEALTH_FAILING) return false;
  
  return true;
}

// ═══════════════════════════════════════════════════════════
// 📊 REPUTATION CALCULATION
// ═══════════════════════════════════════════════════════════

void SwarmEcosystemManager::updateAllReputations() {
  Serial.println("📊 Updating all bot reputations...");
  
  for (uint8_t i = 0; i < botCount; i++) {
    calculateReputation(botProfiles[i].botMAC);
  }
  
  lastReputationUpdate = millis();
}

float SwarmEcosystemManager::calculateReputation(uint8_t* mac) {
  BotProfile* profile = getBotProfile(mac);
  if (profile == nullptr) return 0.0f;
  
  // Reputation components (weighted)
  float dataAccuracyWeight = 0.3f;
  float missionSuccessWeight = 0.25f;
  float communicationWeight = 0.2f;
  float availabilityWeight = 0.15f;
  float healthWeight = 0.1f;
  
  // Calculate health factor
  float healthFactor = (float)profile->health / 5.0f; // Convert to 0.0-1.0
  
  // Composite reputation score
  float reputation = (profile->dataAccuracy * dataAccuracyWeight) +
                    (profile->missionSuccessRate * missionSuccessWeight) +
                    (profile->communicationReliability * communicationWeight) +
                    (profile->availabilityScore * availabilityWeight) +
                    (healthFactor * healthWeight);
  
  // Apply penalties for failures
  if (profile->consecutiveFailures > 3) {
    reputation *= 0.8f; // 20% penalty for repeated failures
  }
  if (profile->consecutiveFailures > 10) {
    reputation *= 0.5f; // Additional 50% penalty for chronic issues
  }
  
  // Convert to 0-100 scale
  reputation *= 100.0f;
  
  // Apply blacklist penalty
  if (profile->isBlacklisted) {
    reputation = min(reputation, 25.0f); // Max 25% reputation when blacklisted
  }
  
  profile->reputationScore = constrain(reputation, 0.0f, 100.0f);
  profile->lastReputationUpdate = millis();
  
  return profile->reputationScore;
}

void SwarmEcosystemManager::updateDataAccuracy(uint8_t* mac) {
  BotProfile* profile = getBotProfile(mac);
  if (profile == nullptr) return;
  
  if (profile->totalDataSent == 0) {
    profile->dataAccuracy = 0.8f; // Default for new bots
    return;
  }
  
  // Calculate accuracy based on verification results
  float accuracy = (float)profile->dataVerifiedGood / (float)profile->totalDataSent;
  
  // Apply smoothing to prevent rapid changes
  profile->dataAccuracy = (profile->dataAccuracy * 0.7f) + (accuracy * 0.3f);
  profile->dataAccuracy = constrain(profile->dataAccuracy, 0.0f, 1.0f);
}

// ═══════════════════════════════════════════════════════════
// 🔍 ECOSYSTEM ANALYSIS
// ═══════════════════════════════════════════════════════════

void SwarmEcosystemManager::performEcosystemAnalysis() {
  Serial.println("🔍 Performing ecosystem analysis...");
  
  identifyWeakBots();
  identifyCapabilityGaps();
  generateUpgradeRecommendations();
  
  lastEcosystemAnalysis = millis();
}

void SwarmEcosystemManager::identifyWeakBots() {
  Serial.println("🔍 Identifying weak bots...");
  
  for (uint8_t i = 0; i < botCount; i++) {
    BotProfile* profile = &botProfiles[i];
    
    bool isWeak = false;
    String weaknessReasons = "";
    
    // Check reputation
    if (profile->reputationScore < 30.0f) {
      isWeak = true;
      weaknessReasons += "Low reputation ";
    }
    
    // Check data accuracy
    if (profile->dataAccuracy < 0.6f) {
      isWeak = true;
      weaknessReasons += "Poor data accuracy ";
    }
    
    // Check health
    if (profile->health <= HEALTH_DEGRADED) {
      isWeak = true;
      weaknessReasons += "Health issues ";
    }
    
    // Check consecutive failures
    if (profile->consecutiveFailures > 5) {
      isWeak = true;
      weaknessReasons += "Communication failures ";
    }
    
    if (isWeak) {
      profile->needsInspection = true;
      Serial.printf("⚠️ Weak bot identified: %s - %s\n", 
                    profile->botName, weaknessReasons.c_str());
    }
  }
}

void SwarmEcosystemManager::identifyCapabilityGaps() {
  // Count bots by type
  uint8_t scoutCount = 0;
  uint8_t manipulatorCount = 0;
  uint8_t heavyCount = 0;
  uint8_t aerialCount = 0;
  
  for (uint8_t i = 0; i < botCount; i++) {
    switch (botProfiles[i].botType) {
      case BOT_SCOUT: scoutCount++; break;
      case BOT_MANIPULATOR: manipulatorCount++; break;
      case BOT_HEAVY: heavyCount++; break;
      case BOT_AERIAL: aerialCount++; break;
      default: break;
    }
  }
  
  Serial.printf("🔍 Bot type distribution: Scouts=%d, Manipulators=%d, Heavy=%d, Aerial=%d\n",
                scoutCount, manipulatorCount, heavyCount, aerialCount);
  
  // Identify gaps (basic logic)
  if (scoutCount == 0) {
    Serial.println("🚨 CRITICAL GAP: No scout bots available!");
  }
  if (manipulatorCount == 0 && botCount > 2) {
    Serial.println("⚠️ GAP: No manipulator bots for complex tasks");
  }
  if (heavyCount == 0 && botCount > 4) {
    Serial.println("💡 SUGGESTION: Heavy bot would improve capability diversity");
  }
}

void SwarmEcosystemManager::generateUpgradeRecommendations() {
  Serial.println("🔧 Generating upgrade recommendations...");
  
  for (uint8_t i = 0; i < botCount; i++) {
    BotProfile* profile = &botProfiles[i];
    
    // Check if bot needs upgrades
    bool needsUpgrade = false;
    
    if (profile->health <= HEALTH_DEGRADED) {
      Serial.printf("🔧 %s: Recommend hardware maintenance/replacement\n", profile->botName);
      needsUpgrade = true;
    }
    
    if (profile->dataAccuracy < 0.7f) {
      Serial.printf("🔧 %s: Recommend sensor calibration/upgrade\n", profile->botName);
      needsUpgrade = true;
    }
    
    if (profile->fitness < 0.5f && profile->generation > 20) {
      Serial.printf("🔧 %s: Recommend genome reset/fresh start\n", profile->botName);
      needsUpgrade = true;
    }
    
    profile->needsUpgrade = needsUpgrade;
  }
}

// ═══════════════════════════════════════════════════════════
// 📈 SWARM HEALTH MONITORING
// ═══════════════════════════════════════════════════════════

float SwarmEcosystemManager::getOverallSwarmHealth() {
  if (botCount == 0) return 0.0f;
  
  float totalHealth = 0.0f;
  for (uint8_t i = 0; i < botCount; i++) {
    totalHealth += (float)botProfiles[i].health;
  }
  
  return (totalHealth / (botCount * 5.0f)) * 100.0f; // Convert to percentage
}

uint8_t SwarmEcosystemManager::getHealthyBotCount() {
  uint8_t count = 0;
  for (uint8_t i = 0; i < botCount; i++) {
    if (botProfiles[i].health >= HEALTH_GOOD) {
      count++;
    }
  }
  return count;
}

uint8_t SwarmEcosystemManager::getDegradedBotCount() {
  uint8_t count = 0;
  for (uint8_t i = 0; i < botCount; i++) {
    if (botProfiles[i].health <= HEALTH_DEGRADED) {
      count++;
    }
  }
  return count;
}

// ═══════════════════════════════════════════════════════════
// 🎯 TASK ASSIGNMENT INTELLIGENCE
// ═══════════════════════════════════════════════════════════

uint8_t* SwarmEcosystemManager::selectBestBotForTask(TaskType task, uint8_t* candidateMACs, uint8_t candidateCount) {
  if (candidateCount == 0) return nullptr;
  
  float bestScore = -1.0f;
  uint8_t* bestBot = nullptr;
  
  for (uint8_t i = 0; i < candidateCount; i++) {
    uint8_t* mac = &candidateMACs[i * 6];
    BotProfile* profile = getBotProfile(mac);
    if (profile == nullptr) continue;
    
    // Skip blacklisted or severely degraded bots
    if (profile->isBlacklisted || profile->health <= HEALTH_FAILING) continue;
    
    // Calculate suitability score
    float score = profile->reputationScore * 0.4f +
                  profile->dataAccuracy * 30.0f +
                  profile->missionSuccessRate * 20.0f +
                  (float)profile->health * 2.0f;
    
    if (score > bestScore) {
      bestScore = score;
      bestBot = mac;
    }
  }
  
  return bestBot;
}

bool SwarmEcosystemManager::shouldExcludeFromCriticalTasks(uint8_t* mac) {
  BotProfile* profile = getBotProfile(mac);
  if (profile == nullptr) return true;
  
  return profile->isBlacklisted || 
         profile->reputationScore < 40.0f ||
         profile->health <= HEALTH_DEGRADED ||
         profile->consecutiveFailures > 3;
}

void SwarmEcosystemManager::blacklistBot(uint8_t* mac, const char* reason) {
  BotProfile* profile = getBotProfile(mac);
  if (profile == nullptr) return;
  
  profile->isBlacklisted = true;
  profile->needsInspection = true;
  
  Serial.printf("🚫 Bot %s BLACKLISTED: %s\n", profile->botName, reason);
}

void SwarmEcosystemManager::rehabilitateBot(uint8_t* mac) {
  BotProfile* profile = getBotProfile(mac);
  if (profile == nullptr) return;
  
  profile->isBlacklisted = false;
  profile->needsInspection = false;
  profile->consecutiveFailures = 0;
  
  // Reset to moderate reputation to give second chance
  profile->reputationScore = 50.0f;
  
  Serial.printf("✅ Bot %s rehabilitated - second chance granted\n", profile->botName);
}

// ═══════════════════════════════════════════════════════════
// 🔄 PERIODIC UPDATE CYCLE
// ═══════════════════════════════════════════════════════════

void SwarmEcosystemManager::update() {
  unsigned long now = millis();
  
  // Update reputations every 10 minutes
  if (now - lastReputationUpdate > REPUTATION_UPDATE_INTERVAL) {
    updateAllReputations();
  }
  
  // Perform ecosystem analysis every hour
  if (now - lastEcosystemAnalysis > ECOSYSTEM_ANALYSIS_INTERVAL) {
    performEcosystemAnalysis();
  }
  
  // Check for timeout bots
  for (uint8_t i = 0; i < botCount; i++) {
    if (now - botProfiles[i].lastSeenTimestamp > 30000) { // 30 second timeout
      botProfiles[i].consecutiveFailures++;
      
      if (botProfiles[i].consecutiveFailures > 10) {
        Serial.printf("📴 Bot %s appears offline (timeout)\n", botProfiles[i].botName);
        botProfiles[i].availabilityScore *= 0.9f; // Reduce availability score
      }
    }
  }
}

// ═══════════════════════════════════════════════════════════
// 🐛 DEBUG & REPORTING
// ═══════════════════════════════════════════════════════════

void SwarmEcosystemManager::printSwarmReport() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║       🌐 SWARM ECOSYSTEM REPORT       ║");
  Serial.println("╚════════════════════════════════════════╝");
  
  Serial.printf("📊 Total Bots: %d/%d\n", botCount, MAX_BOT_PROFILES);
  Serial.printf("🔗 Active Relationships: %d/%d\n", relationshipCount, MAX_RELATIONSHIPS);
  Serial.printf("💚 Overall Swarm Health: %.1f%%\n", getOverallSwarmHealth());
  Serial.printf("✅ Healthy Bots: %d\n", getHealthyBotCount());
  Serial.printf("⚠️ Degraded Bots: %d\n", getDegradedBotCount());
  
  Serial.println("\n🤖 Bot Profiles:");
  for (uint8_t i = 0; i < botCount; i++) {
    BotProfile* bot = &botProfiles[i];
    Serial.printf("  %s: Rep=%.1f, Health=%d, Trust=%.3f%s%s\n",
                  bot->botName, bot->reputationScore, bot->health, bot->dataAccuracy,
                  bot->needsInspection ? " [INSPECT]" : "",
                  bot->isBlacklisted ? " [BLACKLISTED]" : "");
  }
  
  Serial.println("═══════════════════════════════════════\n");
}

void SwarmEcosystemManager::printBotProfile(uint8_t* mac) {
  BotProfile* profile = getBotProfile(mac);
  if (profile == nullptr) {
    Serial.println("Bot not found");
    return;
  }
  
  Serial.printf("\n🤖 Bot Profile: %s\n", profile->botName);
  Serial.printf("  Type: %d, Generation: %lu, Fitness: %.3f\n", 
                profile->botType, profile->generation, profile->fitness);
  Serial.printf("  Reputation: %.1f/100, Health: %d/5\n", 
                profile->reputationScore, profile->health);
  Serial.printf("  Data Accuracy: %.3f, Success Rate: %.3f\n", 
                profile->dataAccuracy, profile->missionSuccessRate);
  Serial.printf("  Status: %s%s%s\n",
                profile->needsInspection ? "NEEDS_INSPECTION " : "",
                profile->isBlacklisted ? "BLACKLISTED " : "",
                profile->consecutiveFailures > 0 ? "COMM_ISSUES" : "OK");
}

void SwarmEcosystemManager::printTrustNetwork() {
  Serial.println("\n🤝 Trust Network:");
  for (uint8_t i = 0; i < relationshipCount; i++) {
    BotRelationship* rel = &relationships[i];
    if (!rel->isActive) continue;
    
    Serial.printf("  Trust=%.3f, Interactions=%lu (%lu success, %lu failed)\n",
                  rel->trustScore, rel->interactionCount, 
                  rel->successfulInteractions, rel->failedInteractions);
  }
}

// ═══════════════════════════════════════════════════════════
// 🔧 INTEGRATION HELPER FUNCTIONS
// ═══════════════════════════════════════════════════════════

void initializeEcosystemManager() {
  if (ecosystemManager == nullptr) {
    ecosystemManager = new SwarmEcosystemManager();
    ecosystemManager->initialize();
    
    // Register self
    uint8_t myMac[6];
    WiFi.macAddress(myMac);
    ecosystemManager->registerBot(myMac, BOT_SCOUT, "SPEEDIE"); // Adjust as needed
  }
}

void handleEcosystemMessage(const uint8_t* senderMac, SwarmMessage* message) {
  if (ecosystemManager == nullptr) return;
  
  // Handle ecosystem-specific message types here
  // This would be integrated with your existing handleSwarmMessage function
}

bool verifyDataWithEcosystem(uint8_t* senderMac, uint32_t dataHash, float* trustMultiplier) {
  if (ecosystemManager == nullptr) {
    *trustMultiplier = 1.0f;
    return true;
  }
  
  BotProfile* profile = ecosystemManager->getBotProfile(senderMac);
  if (profile == nullptr) {
    *trustMultiplier = 0.5f; // Unknown bot - moderate trust
    return true;
  }
  
  // Apply trust multiplier based on reputation
  *trustMultiplier = profile->reputationScore / 100.0f;
  
  return ecosystemManager->shouldTrustBot(senderMac);
}

void reportInteractionToEcosystem(uint8_t* peerMac, InteractionType type, InteractionResult result) {
  if (ecosystemManager == nullptr) return;
  
  uint8_t myMac[6];
  WiFi.macAddress(myMac);
  
  ecosystemManager->recordInteraction(myMac, peerMac, type, result);
}