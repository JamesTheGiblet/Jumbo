/*
 * 🌊 Swarm Intelligence Framework
 * ===============================
 * Advanced emergent behaviors for Project Jumbo bot swarm
 * Implements collective intelligence patterns for WHEELIE + SPEEDIE coordination
 * 
 * Emergent Behaviors:
 * - Leader election and role assignment
 * - Collaborative exploration strategies  
 * - Task distribution and execution
 * - Collective decision making
 * - Adaptive formation control
 * - Consensus protocols
 * 
 * Author: Project Jumbo Team
 * Version: 1.0.0
 */

#ifndef SWARM_INTELLIGENCE_H
#define SWARM_INTELLIGENCE_H

#include "swarm_espnow.h"

// ═══════════════════════════════════════════════════════════
// 🏆 LEADER ELECTION ALGORITHMS
// ═══════════════════════════════════════════════════════════

enum LeadershipCriteria {
  ELECT_BY_FITNESS = 0x01,        // Highest fitness score leads
  ELECT_BY_GENERATION = 0x02,     // Most evolved bot leads
  ELECT_BY_BATTERY = 0x03,        // Highest battery leads
  ELECT_BY_SENSOR_QUALITY = 0x04, // Best sensor coverage leads
  ELECT_BY_EXPERIENCE = 0x05,     // Most strategies learned leads
  ELECT_BY_RANDOM = 0x06,         // Random selection for fairness
  ELECT_BY_CONSENSUS = 0x07       // Democratic voting system
};

struct LeadershipBid {
  uint8_t candidateMac[6];        // Candidate's MAC address
  BotType botType;                // Type of bot
  uint16_t generation;            // Evolution generation
  float fitnessScore;             // Current fitness
  uint8_t batteryLevel;           // Battery percentage
  uint16_t strategiesLearned;     // Number of strategies
  uint32_t uptime;               // Time since boot
  uint8_t votes;                  // Votes received
  uint32_t bidTimestamp;          // When bid was made
};

#define MAX_LEADERSHIP_CANDIDATES 8
#define LEADERSHIP_TIMEOUT 10000    // 10 second election cycle
#define MIN_VOTES_REQUIRED 2        // Minimum votes to become leader

// ═══════════════════════════════════════════════════════════
// 🗺️ COLLABORATIVE EXPLORATION STRATEGIES
// ═══════════════════════════════════════════════════════════

enum ExplorationStrategy {
  EXPLORE_RANDOM_WALK = 0x01,     // Independent random exploration
  EXPLORE_SPIRAL_SEARCH = 0x02,   // Expanding spiral pattern
  EXPLORE_GRID_COVERAGE = 0x03,   // Systematic grid coverage
  EXPLORE_BOUNDARY_TRACE = 0x04,  // Follow perimeter boundaries
  EXPLORE_GRADIENT_ASCENT = 0x05, // Follow signal gradients
  EXPLORE_SWARM_DISPERSION = 0x06, // Maximize swarm distribution
  EXPLORE_TARGET_CONVERGENCE = 0x07 // Converge on target location
};

struct ExplorationZone {
  float centerX, centerY;         // Zone center coordinates
  float width, height;            // Zone dimensions
  uint8_t assignedBot[6];         // MAC of assigned bot
  ExplorationStrategy strategy;   // Exploration method
  uint8_t priority;               // Zone priority (1-10)
  uint32_t timeAllocated;         // Time allocated (ms)
  uint32_t startTime;             // Start timestamp
  uint8_t completionPercent;      // Progress (0-100%)
  bool isActive;                  // Zone active flag
};

#define MAX_EXPLORATION_ZONES 16

// ═══════════════════════════════════════════════════════════
// 🎯 TASK DISTRIBUTION SYSTEM
// ═══════════════════════════════════════════════════════════

enum TaskCategory {
  TASK_CAT_EXPLORATION = 0x01,    // Area exploration tasks
  TASK_CAT_SURVEILLANCE = 0x02,   // Monitoring and patrol
  TASK_CAT_RESCUE = 0x03,         // Search and rescue ops
  TASK_CAT_MAINTENANCE = 0x04,    // System maintenance
  TASK_CAT_LEARNING = 0x05,       // Collaborative learning
  TASK_CAT_EMERGENCY = 0x06       // Emergency response
};

enum TaskStatus {
  TASK_STATUS_PENDING = 0x01,     // Waiting for assignment
  TASK_STATUS_ASSIGNED = 0x02,    // Assigned to bot
  TASK_STATUS_ACTIVE = 0x03,      // Currently executing
  TASK_STATUS_COMPLETED = 0x04,   // Successfully completed
  TASK_STATUS_FAILED = 0x05,      // Failed or aborted
  TASK_STATUS_CANCELLED = 0x06    // Cancelled by coordinator
};

struct SwarmTask {
  uint16_t taskId;                // Unique task identifier
  TaskCategory category;          // Task category
  TaskType taskType;              // Specific task type
  uint8_t priority;               // Priority level (1-10)
  uint8_t assignedBot[6];         // Assigned bot MAC
  uint8_t requesterBot[6];        // Bot that requested task
  TaskStatus status;              // Current status
  uint32_t createdTime;           // Creation timestamp
  uint32_t deadlineTime;          // Deadline timestamp
  uint32_t startTime;             // Start execution time
  uint32_t completionTime;        // Completion timestamp
  float parameters[8];            // Task-specific parameters
  uint8_t progressPercent;        // Progress (0-100%)
  uint8_t retryCount;             // Number of retries
  bool requiresConfirmation;      // Needs completion confirmation
};

#define MAX_SWARM_TASKS 32
#define TASK_TIMEOUT 30000          // 30 second task timeout
#define MAX_TASK_RETRIES 3

// ═══════════════════════════════════════════════════════════
// 🗳️ CONSENSUS AND DECISION MAKING
// ═══════════════════════════════════════════════════════════

enum ConsensusType {
  CONSENSUS_SIMPLE_MAJORITY = 0x01,   // >50% agreement
  CONSENSUS_SUPERMAJORITY = 0x02,     // >66% agreement  
  CONSENSUS_UNANIMOUS = 0x03,         // 100% agreement
  CONSENSUS_WEIGHTED_VOTE = 0x04,     // Fitness-weighted voting
  CONSENSUS_EXPERT_DECISION = 0x05    // Specialist bot decides
};

enum DecisionTopic {
  DECIDE_EXPLORATION_AREA = 0x01,     // Where to explore next
  DECIDE_FORMATION_CHANGE = 0x02,     // Change swarm formation
  DECIDE_LEADER_SELECTION = 0x03,     // Select new leader
  DECIDE_TASK_PRIORITY = 0x04,        // Prioritize tasks
  DECIDE_RESOURCE_ALLOCATION = 0x05,  // Allocate resources
  DECIDE_EMERGENCY_RESPONSE = 0x06,   // Emergency protocols
  DECIDE_LEARNING_STRATEGY = 0x07     // Learning approach
};

struct ConsensusVote {
  uint16_t proposalId;            // Proposal identifier
  DecisionTopic topic;            // What we're deciding
  uint8_t voterMac[6];            // Voter's MAC address
  uint8_t choice;                 // Vote choice (0-255)
  float confidence;               // Vote confidence (0.0-1.0)
  uint32_t timestamp;             // Vote timestamp
  bool isValid;                   // Vote validity flag
};

struct ConsensusProposal {
  uint16_t proposalId;            // Unique proposal ID
  DecisionTopic topic;            // Decision topic
  uint8_t proposerMac[6];         // Who proposed this
  ConsensusType consensusType;    // Type of consensus needed
  uint32_t createdTime;           // When proposal was made
  uint32_t votingDeadline;        // Voting deadline
  uint8_t totalVoters;            // Expected number of voters
  uint8_t votesReceived;          // Votes received so far
  ConsensusVote votes[MAX_SWARM_PEERS]; // All votes
  uint8_t winningChoice;          // Current winning choice
  float winningConfidence;        // Confidence in winning choice
  bool isResolved;                // Consensus reached
  bool isActive;                  // Proposal active
};

#define MAX_CONSENSUS_PROPOSALS 8
#define VOTING_TIMEOUT 15000        // 15 second voting window
#define MIN_CONSENSUS_PARTICIPANTS 2

// ═══════════════════════════════════════════════════════════
// 🔄 FORMATION CONTROL PATTERNS
// ═══════════════════════════════════════════════════════════

enum FormationType {
  FORMATION_DISPERSED = 0x01,         // Maximum spread coverage
  FORMATION_LINE = 0x02,              // Linear formation
  FORMATION_WEDGE = 0x03,             // V-shaped formation
  FORMATION_CIRCLE = 0x04,            // Circular formation
  FORMATION_GRID = 0x05,              // Grid pattern
  FORMATION_FOLLOW_LEADER = 0x06,     // Single file behind leader
  FORMATION_PARALLEL = 0x07,          // Side-by-side movement
  FORMATION_ADAPTIVE = 0x08           // Context-adaptive formation
};

struct FormationPosition {
  float relativeX;                // X offset from formation center
  float relativeY;                // Y offset from formation center
  float heading;                  // Desired heading
  uint8_t assignedBot[6];         // Bot assigned to this position
  uint8_t priority;               // Position priority
  bool isOccupied;                // Position currently occupied
};

struct SwarmFormation {
  FormationType type;             // Formation type
  float centerX, centerY;         // Formation center
  float scale;                    // Formation scale factor
  float heading;                  // Formation heading
  FormationPosition positions[MAX_SWARM_PEERS]; // Position assignments
  uint8_t activeBots;             // Number of bots in formation
  bool isActive;                  // Formation active
  uint32_t lastUpdate;            // Last formation update
};

// ═══════════════════════════════════════════════════════════
// 🎭 EMERGENT BEHAVIOR PATTERNS
// ═══════════════════════════════════════════════════════════

enum EmergentBehavior {
  EMERGENT_FLOCKING = 0x01,           // Boids-like flocking
  EMERGENT_HERDING = 0x02,            // Shepherding behavior
  EMERGENT_CHAIN_REACTION = 0x03,     // Cascading responses
  EMERGENT_OSCILLATION = 0x04,        // Synchronized oscillation
  EMERGENT_SPIRAL_DANCE = 0x05,       // Spiral movement patterns
  EMERGENT_COMPETITIVE = 0x06,        // Competition-based behavior
  EMERGENT_COOPERATIVE = 0x07,        // Pure cooperation
  EMERGENT_ADAPTIVE_ROLE = 0x08       // Dynamic role switching
};

struct EmergentState {
  EmergentBehavior currentBehavior;   // Active emergent behavior
  float intensity;                    // Behavior intensity (0.0-1.0)
  uint32_t behaviorStartTime;         // When behavior started
  uint32_t lastBehaviorChange;        // Last behavior change
  uint8_t participatingBots;          // Bots showing this behavior
  float coherence;                    // Behavioral coherence (0.0-1.0)
  bool isStable;                      // Behavior stability flag
};

// ═══════════════════════════════════════════════════════════
// 🧬 COLLECTIVE LEARNING PROTOCOLS
// ═══════════════════════════════════════════════════════════

enum LearningMode {
  LEARN_INDEPENDENT = 0x01,           // Individual learning only
  LEARN_IMITATION = 0x02,             // Copy successful peers
  LEARN_COLLABORATION = 0x03,         // Joint problem solving
  LEARN_COMPETITION = 0x04,           // Competitive learning
  LEARN_SPECIALIZATION = 0x05,        // Role-based specialization
  LEARN_CROSS_BREEDING = 0x06,        // Genetic algorithm approach
  LEARN_ENSEMBLE = 0x07               // Ensemble decision making
};

struct LearningShare {
  uint8_t teacherBot[6];              // Bot sharing knowledge
  uint8_t learnerBot[6];              // Bot receiving knowledge
  uint16_t strategyId;                // Strategy identifier
  float successRate;                  // Strategy success rate
  uint32_t timesUsed;                 // Usage count
  uint8_t contextData[16];            // Context-specific data
  uint32_t shareTime;                 // When knowledge was shared
  bool wasSuccessful;                 // Learning was successful
};

#define MAX_LEARNING_SHARES 64

// ═══════════════════════════════════════════════════════════
// 🔧 SWARM INTELLIGENCE API
// ═══════════════════════════════════════════════════════════

// Leadership functions
bool initiateLeaderElection(LeadershipCriteria criteria);
void submitLeadershipBid(LeadershipBid* bid);
uint8_t* electLeader(LeadershipBid candidates[], int candidateCount);
void transferLeadership(uint8_t* newLeaderMac);

// Exploration coordination
bool assignExplorationZone(uint8_t* botMac, ExplorationZone* zone);
void reportExplorationProgress(uint16_t zoneId, uint8_t progressPercent);
ExplorationStrategy selectOptimalStrategy(ExplorationZone* zone, BotType botType);

// Task management
uint16_t createSwarmTask(TaskCategory category, TaskType type, uint8_t priority);
bool assignTask(uint16_t taskId, uint8_t* botMac);
void reportTaskProgress(uint16_t taskId, uint8_t progressPercent);
void completeTask(uint16_t taskId, bool successful);

// Consensus protocols
uint16_t proposeDecision(DecisionTopic topic, ConsensusType type);
void castVote(uint16_t proposalId, uint8_t choice, float confidence);
bool checkConsensusReached(uint16_t proposalId);
uint8_t getConsensusResult(uint16_t proposalId);

// Formation control
void setFormation(FormationType type, float scale);
FormationPosition* getAssignedPosition(uint8_t* botMac);
void updateFormationPosition(float x, float y, float heading);
bool maintainFormation();

// Emergent behavior detection
EmergentBehavior detectEmergentBehavior();
void encourageEmergence(EmergentBehavior behavior);
float measureSwarmCoherence();

// Collective learning
void shareStrategy(uint16_t strategyId, float successRate);
void requestKnowledge(uint8_t* expertBot, uint16_t topicId);
void crossBreedGenomes(uint8_t* parentBot1, uint8_t* parentBot2);

#endif // SWARM_INTELLIGENCE_H