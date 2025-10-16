/*
 * 🌊 Swarm Intelligence Implementation
 * ====================================
 * Core swarm behaviors for WHEELIE and SPEEDIE coordination
 * Implements practical emergent intelligence patterns
 * 
 * Key Features:
 * - Simple leader election (fitness-based)
 * - Basic task coordination 
 * - Exploration area sharing
 * - Emergency response protocols
 * - Adaptive role assignment
 * 
 * Author: Project Jumbo Team
 * Version: 1.0.0
 */

#include "swarm_intelligence.h"
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════
// 🌊 GLOBAL SWARM STATE VARIABLES
// ═══════════════════════════════════════════════════════════

// Leadership state
LeadershipBid leadershipCandidates[MAX_LEADERSHIP_CANDIDATES];
int candidateCount = 0;
uint8_t currentLeader[6] = {0};
bool isLeader = false;
unsigned long lastLeadershipCheck = 0;

// Task management
SwarmTask activeTasks[MAX_SWARM_TASKS];
int activeTaskCount = 0;
uint16_t nextTaskId = 1;

// Exploration zones
ExplorationZone explorationZones[MAX_EXPLORATION_ZONES];
int activeZoneCount = 0;

// Formation state
SwarmFormation currentFormation;
FormationPosition myPosition;

// Emergent behavior state
EmergentState emergentState;

// ═══════════════════════════════════════════════════════════
// 🏆 LEADER ELECTION IMPLEMENTATION
// ═══════════════════════════════════════════════════════════

bool initiateLeaderElection(LeadershipCriteria criteria) {
  Serial.println("🗳️ Initiating leader election...");
  
  // Clear previous candidates
  candidateCount = 0;
  memset(leadershipCandidates, 0, sizeof(leadershipCandidates));
  
  // Submit our own candidacy
  LeadershipBid myBid;
  WiFi.macAddress((uint8_t*)myBid.candidateMac);
  
  // Get bot type and current stats (these would be externally accessible)
  extern BotType myBotType;
  extern EvolvingGenome currentGenome;
  extern int strategyCount;
  
  myBid.botType = myBotType;
  myBid.generation = currentGenome.generation;
  myBid.fitnessScore = currentGenome.fitnessScore;
  myBid.batteryLevel = 100; // Placeholder - could read actual battery
  myBid.strategiesLearned = strategyCount;
  myBid.uptime = millis();
  myBid.votes = 0;
  myBid.bidTimestamp = millis();
  
  submitLeadershipBid(&myBid);
  
  // Broadcast leadership election message
  // This would trigger other bots to submit their bids
  
  lastLeadershipCheck = millis();
  return true;
}

void submitLeadershipBid(LeadershipBid* bid) {
  if (candidateCount >= MAX_LEADERSHIP_CANDIDATES) {
    Serial.println("⚠️ Too many leadership candidates");
    return;
  }
  
  // Check if this candidate already exists (update instead of add)
  for (int i = 0; i < candidateCount; i++) {
    if (memcmp(leadershipCandidates[i].candidateMac, bid->candidateMac, 6) == 0) {
      leadershipCandidates[i] = *bid;
      Serial.printf("📝 Updated leadership bid from %s\n", 
                    macToString(bid->candidateMac).c_str());
      return;
    }
  }
  
  // Add new candidate
  leadershipCandidates[candidateCount] = *bid;
  candidateCount++;
  
  Serial.printf("📝 Leadership bid from %s (Gen:%d, Fit:%.3f)\n",
                macToString(bid->candidateMac).c_str(),
                bid->generation, bid->fitnessScore);
}

uint8_t* electLeader(LeadershipBid candidates[], int candidateCount) {
  if (candidateCount == 0) return nullptr;
  
  // Simple fitness-based election
  int bestCandidate = 0;
  float bestScore = candidates[0].fitnessScore;
  
  for (int i = 1; i < candidateCount; i++) {
    float score = candidates[i].fitnessScore;
    
    // Tie-breaker: higher generation wins
    if (score == bestScore) {
      if (candidates[i].generation > candidates[bestCandidate].generation) {
        bestCandidate = i;
      }
    } else if (score > bestScore) {
      bestScore = score;
      bestCandidate = i;
    }
  }
  
  // Check if we are the winner
  uint8_t myMac[6];
  WiFi.macAddress(myMac);
  
  if (memcmp(candidates[bestCandidate].candidateMac, myMac, 6) == 0) {
    isLeader = true;
    Serial.println("👑 I am elected as swarm leader!");
  } else {
    isLeader = false;
    Serial.printf("👑 Leader elected: %s (Fitness: %.3f)\n",
                  macToString(candidates[bestCandidate].candidateMac).c_str(),
                  candidates[bestCandidate].fitnessScore);
  }
  
  memcpy(currentLeader, candidates[bestCandidate].candidateMac, 6);
  return currentLeader;
}

// ═══════════════════════════════════════════════════════════
// 🎯 TASK COORDINATION IMPLEMENTATION
// ═══════════════════════════════════════════════════════════

uint16_t createSwarmTask(TaskCategory category, TaskType type, uint8_t priority) {
  if (activeTaskCount >= MAX_SWARM_TASKS) {
    Serial.println("⚠️ Task queue full");
    return 0;
  }
  
  // Find empty task slot
  int taskIndex = -1;
  for (int i = 0; i < MAX_SWARM_TASKS; i++) {
    if (activeTasks[i].taskId == 0) {
      taskIndex = i;
      break;
    }
  }
  
  if (taskIndex == -1) return 0;
  
  SwarmTask* task = &activeTasks[taskIndex];
  task->taskId = nextTaskId++;
  task->category = category;
  task->taskType = type;
  task->priority = priority;
  task->status = TASK_STATUS_PENDING;
  task->createdTime = millis();
  task->deadlineTime = millis() + TASK_TIMEOUT;
  task->progressPercent = 0;
  task->retryCount = 0;
  task->requiresConfirmation = true;
  
  // Set requester as ourselves
  WiFi.macAddress((uint8_t*)task->requesterBot);
  
  activeTaskCount++;
  
  Serial.printf("📋 Created task %d: Category=%d, Type=%d, Priority=%d\n",
                task->taskId, category, type, priority);
  
  return task->taskId;
}

bool assignTask(uint16_t taskId, uint8_t* botMac) {
  SwarmTask* task = findTask(taskId);
  if (!task) return false;
  
  if (task->status != TASK_STATUS_PENDING) {
    Serial.printf("⚠️ Task %d not in pending state\n", taskId);
    return false;
  }
  
  memcpy(task->assignedBot, botMac, 6);
  task->status = TASK_STATUS_ASSIGNED;
  task->startTime = millis();
  
  Serial.printf("📋 Task %d assigned to %s\n", taskId, macToString(botMac).c_str());
  
  // Send task assignment message to the bot
  // This would use the ESP-NOW messaging system
  
  return true;
}

void reportTaskProgress(uint16_t taskId, uint8_t progressPercent) {
  SwarmTask* task = findTask(taskId);
  if (!task) return;
  
  task->progressPercent = progressPercent;
  task->status = TASK_STATUS_ACTIVE;
  
  if (progressPercent >= 100) {
    completeTask(taskId, true);
  }
}

void completeTask(uint16_t taskId, bool successful) {
  SwarmTask* task = findTask(taskId);
  if (!task) return;
  
  task->status = successful ? TASK_STATUS_COMPLETED : TASK_STATUS_FAILED;
  task->completionTime = millis();
  task->progressPercent = successful ? 100 : task->progressPercent;
  
  Serial.printf("📋 Task %d %s in %ldms\n", 
                taskId, 
                successful ? "completed" : "failed",
                task->completionTime - task->startTime);
  
  // Clean up completed tasks after a delay
  // For now, just mark as completed
}

SwarmTask* findTask(uint16_t taskId) {
  for (int i = 0; i < MAX_SWARM_TASKS; i++) {
    if (activeTasks[i].taskId == taskId) {
      return &activeTasks[i];
    }
  }
  return nullptr;
}

// ═══════════════════════════════════════════════════════════
// 🗺️ EXPLORATION COORDINATION
// ═══════════════════════════════════════════════════════════

bool assignExplorationZone(uint8_t* botMac, ExplorationZone* zone) {
  if (activeZoneCount >= MAX_EXPLORATION_ZONES) return false;
  
  // Find empty zone slot
  int zoneIndex = -1;
  for (int i = 0; i < MAX_EXPLORATION_ZONES; i++) {
    if (!explorationZones[i].isActive) {
      zoneIndex = i;
      break;
    }
  }
  
  if (zoneIndex == -1) return false;
  
  explorationZones[zoneIndex] = *zone;
  memcpy(explorationZones[zoneIndex].assignedBot, botMac, 6);
  explorationZones[zoneIndex].isActive = true;
  explorationZones[zoneIndex].startTime = millis();
  
  activeZoneCount++;
  
  Serial.printf("🗺️ Zone assigned to %s: (%.1f,%.1f) %dx%d\n",
                macToString(botMac).c_str(),
                zone->centerX, zone->centerY,
                (int)zone->width, (int)zone->height);
  
  return true;
}

void reportExplorationProgress(uint16_t zoneId, uint8_t progressPercent) {
  // Find zone by ID (simplified - using array index as ID)
  if (zoneId < MAX_EXPLORATION_ZONES && explorationZones[zoneId].isActive) {
    explorationZones[zoneId].completionPercent = progressPercent;
    
    Serial.printf("🗺️ Zone %d progress: %d%%\n", zoneId, progressPercent);
    
    if (progressPercent >= 100) {
      explorationZones[zoneId].isActive = false;
      activeZoneCount--;
      Serial.printf("✅ Zone %d exploration completed\n", zoneId);
    }
  }
}

ExplorationStrategy selectOptimalStrategy(ExplorationZone* zone, BotType botType) {
  // Select strategy based on bot capabilities and zone characteristics
  
  if (botType == BOT_WHEELIE) {
    // WHEELIE with precision sensors good for detailed exploration
    if (zone->width * zone->height < 25.0) {
      return EXPLORE_SPIRAL_SEARCH; // Small areas - spiral search
    } else {
      return EXPLORE_GRID_COVERAGE; // Large areas - systematic grid
    }
  } else if (botType == BOT_SPEEDIE) {
    // SPEEDIE good for rapid coverage and perimeter work
    if (zone->priority >= 7) {
      return EXPLORE_RANDOM_WALK; // High priority - fast coverage
    } else {
      return EXPLORE_BOUNDARY_TRACE; // Normal priority - perimeter first
    }
  }
  
  return EXPLORE_RANDOM_WALK; // Default fallback
}

// ═══════════════════════════════════════════════════════════
// 🚨 EMERGENCY RESPONSE PROTOCOLS
// ═══════════════════════════════════════════════════════════

void triggerEmergencyResponse(uint8_t emergencyType, float x, float y) {
  Serial.printf("🚨 EMERGENCY: Type=%d at (%.1f, %.1f)\n", emergencyType, x, y);
  
  // Create high-priority emergency task
  uint16_t emergencyTaskId = createSwarmTask(TASK_CAT_EMERGENCY, TASK_RETURN_BASE, 10);
  
  // Broadcast emergency stop to all bots
  // This would use ESP-NOW emergency message
  
  // Switch to emergency formation
  setFormation(FORMATION_DISPERSED, 2.0); // Spread out for safety
  
  // Change swarm behavior to emergency mode
  emergentState.currentBehavior = EMERGENT_COOPERATIVE;
  emergentState.intensity = 1.0;
  emergentState.behaviorStartTime = millis();
}

// ═══════════════════════════════════════════════════════════
// 🔄 FORMATION CONTROL
// ═══════════════════════════════════════════════════════════

void setFormation(FormationType type, float scale) {
  currentFormation.type = type;
  currentFormation.scale = scale;
  currentFormation.isActive = true;
  currentFormation.lastUpdate = millis();
  
  Serial.printf("🔄 Formation set: Type=%d, Scale=%.1f\n", type, scale);
  
  // Calculate positions based on formation type
  switch (type) {
    case FORMATION_LINE:
      setupLineFormation(scale);
      break;
    case FORMATION_CIRCLE:
      setupCircularFormation(scale);
      break;
    case FORMATION_DISPERSED:
      setupDispersedFormation(scale);
      break;
    default:
      setupDefaultFormation(scale);
      break;
  }
}

void setupLineFormation(float scale) {
  // Simple line formation - bots arranged in a line
  currentFormation.positions[0].relativeX = -scale;
  currentFormation.positions[0].relativeY = 0;
  currentFormation.positions[1].relativeX = scale;
  currentFormation.positions[1].relativeY = 0;
  
  for (int i = 0; i < 2; i++) {
    currentFormation.positions[i].heading = 0; // Face forward
    currentFormation.positions[i].priority = i + 1;
    currentFormation.positions[i].isOccupied = false;
  }
}

void setupCircularFormation(float scale) {
  // Circular formation - bots arranged in a circle
  float angle = 0;
  float angleStep = 2 * PI / 2; // For 2 bots
  
  for (int i = 0; i < 2; i++) {
    currentFormation.positions[i].relativeX = scale * cos(angle);
    currentFormation.positions[i].relativeY = scale * sin(angle);
    currentFormation.positions[i].heading = angle + PI/2; // Face tangent
    currentFormation.positions[i].priority = i + 1;
    currentFormation.positions[i].isOccupied = false;
    angle += angleStep;
  }
}

void setupDispersedFormation(float scale) {
  // Dispersed formation - maximize distance between bots
  currentFormation.positions[0].relativeX = -scale * 1.5;
  currentFormation.positions[0].relativeY = -scale * 1.5;
  currentFormation.positions[1].relativeX = scale * 1.5;
  currentFormation.positions[1].relativeY = scale * 1.5;
  
  for (int i = 0; i < 2; i++) {
    currentFormation.positions[i].heading = 0;
    currentFormation.positions[i].priority = i + 1;
    currentFormation.positions[i].isOccupied = false;
  }
}

void setupDefaultFormation(float scale) {
  // Default formation - simple side-by-side
  currentFormation.positions[0].relativeX = -scale * 0.5;
  currentFormation.positions[0].relativeY = 0;
  currentFormation.positions[1].relativeX = scale * 0.5;
  currentFormation.positions[1].relativeY = 0;
  
  for (int i = 0; i < 2; i++) {
    currentFormation.positions[i].heading = 0;
    currentFormation.positions[i].priority = i + 1;
    currentFormation.positions[i].isOccupied = false;
  }
}

// ═══════════════════════════════════════════════════════════
// 🧠 EMERGENT BEHAVIOR DETECTION
// ═══════════════════════════════════════════════════════════

EmergentBehavior detectEmergentBehavior() {
  // Simple behavior detection based on current state
  unsigned long currentTime = millis();
  
  // Check if we've been in formation for a while
  if (currentFormation.isActive && 
      (currentTime - currentFormation.lastUpdate > 30000)) {
    // Been in formation for 30+ seconds - stable formation behavior
    emergentState.currentBehavior = EMERGENT_FLOCKING;
    emergentState.intensity = 0.8;
    emergentState.isStable = true;
    return EMERGENT_FLOCKING;
  }
  
  // Check for competitive behavior (multiple tasks, high priority)
  int highPriorityTasks = 0;
  for (int i = 0; i < MAX_SWARM_TASKS; i++) {
    if (activeTasks[i].taskId > 0 && activeTasks[i].priority >= 7) {
      highPriorityTasks++;
    }
  }
  
  if (highPriorityTasks > 1) {
    emergentState.currentBehavior = EMERGENT_COMPETITIVE;
    emergentState.intensity = (float)highPriorityTasks / 5.0; // Scale by task count
    return EMERGENT_COMPETITIVE;
  }
  
  // Default to cooperative behavior
  emergentState.currentBehavior = EMERGENT_COOPERATIVE;
  emergentState.intensity = 0.5;
  return EMERGENT_COOPERATIVE;
}

float measureSwarmCoherence() {
  // Simple coherence measure based on synchronized activity
  float coherence = 0.5; // Base coherence
  
  // Increase coherence if in active formation
  if (currentFormation.isActive) {
    coherence += 0.3;
  }
  
  // Increase coherence if tasks are being completed
  int activeTasks = 0;
  for (int i = 0; i < MAX_SWARM_TASKS; i++) {
    if (activeTasks[i].taskId > 0 && activeTasks[i].status == TASK_STATUS_ACTIVE) {
      activeTasks++;
    }
  }
  
  if (activeTasks > 0) {
    coherence += 0.2;
  }
  
  // Cap at 1.0
  return min(coherence, 1.0);
}

// ═══════════════════════════════════════════════════════════
// 🔄 SWARM UPDATE FUNCTIONS (Call from main loop)
// ═══════════════════════════════════════════════════════════

void updateSwarmIntelligence() {
  unsigned long currentTime = millis();
  
  // Check for leadership timeout
  if (currentTime - lastLeadershipCheck > 30000) { // 30 seconds
    if (candidateCount > 0) {
      electLeader(leadershipCandidates, candidateCount);
    }
    lastLeadershipCheck = currentTime;
  }
  
  // Clean up expired tasks
  for (int i = 0; i < MAX_SWARM_TASKS; i++) {
    SwarmTask* task = &activeTasks[i];
    if (task->taskId > 0 && 
        task->status != TASK_STATUS_COMPLETED &&
        task->status != TASK_STATUS_FAILED &&
        currentTime > task->deadlineTime) {
      
      Serial.printf("⏰ Task %d timed out\n", task->taskId);
      task->status = TASK_STATUS_FAILED;
    }
  }
  
  // Update emergent behavior detection
  EmergentBehavior detectedBehavior = detectEmergentBehavior();
  if (detectedBehavior != emergentState.currentBehavior) {
    Serial.printf("🌊 Emergent behavior change: %d -> %d\n", 
                  emergentState.currentBehavior, detectedBehavior);
    emergentState.lastBehaviorChange = currentTime;
  }
  
  // Update coherence measurement
  emergentState.coherence = measureSwarmCoherence();
}