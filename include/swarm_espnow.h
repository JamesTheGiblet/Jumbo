/*
 * 📡 ESP-NOW Swarm Communication Protocol
 * ========================================
 * Peer-to-peer communication framework for Project Jumbo bots
 * Enables direct WHEELIE ↔ SPEEDIE communication without WiFi infrastructure
 * 
 * Protocol Features:
 * - Low latency P2P messaging (1-20ms typical)
 * - Auto-discovery and pairing
 * - Message prioritization and routing
 * - Swarm intelligence coordination
 * - Emergent behavior protocols
 * 
 * Author: Project Jumbo Team
 * Version: 1.0.0
 */

#ifndef SWARM_ESPNOW_H
#define SWARM_ESPNOW_H

#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════
// 🤖 BOT IDENTIFICATION & ROLES
// ═══════════════════════════════════════════════════════════
enum BotType {
  BOT_WHEELIE = 0x01,   // Precision scout with VL53L0X + motion sensor
  BOT_SPEEDIE = 0x02,   // Speed scout with HC-SR04 + MPU-6050
  BOT_UNKNOWN = 0xFF
};

enum SwarmRole {
  ROLE_SCOUT = 0x01,        // Exploration and mapping
  ROLE_GUARDIAN = 0x02,     // Perimeter monitoring
  ROLE_COORDINATOR = 0x03,  // Swarm decision making
  ROLE_FOLLOWER = 0x04,     // Following leader instructions
  ROLE_LEADER = 0x05        // Leading swarm operations
};

// ═══════════════════════════════════════════════════════════
// 📨 MESSAGE TYPES & PROTOCOLS
// ═══════════════════════════════════════════════════════════
enum MessageType {
  // Discovery & Connection
  MSG_DISCOVERY = 0x01,      // "I'm here" announcement
  MSG_PAIRING_REQUEST = 0x02, // Request to pair
  MSG_PAIRING_RESPONSE = 0x03, // Response to pairing
  MSG_HEARTBEAT = 0x04,      // Keep-alive signal
  
  // Status & Data Sharing
  MSG_STATUS_UPDATE = 0x10,  // General status info
  MSG_SENSOR_DATA = 0x11,    // Sensor readings
  MSG_POSITION_UPDATE = 0x12, // Location information
  MSG_EMOTIONAL_STATE = 0x13, // Emotional state sharing
  
  // Coordination & Commands
  MSG_TASK_ASSIGNMENT = 0x20, // Assign task to peer
  MSG_TASK_COMPLETION = 0x21, // Report task completion
  MSG_FORMATION_REQUEST = 0x22, // Request formation change
  MSG_EMERGENCY_STOP = 0x23,  // Emergency halt command
  
  // Swarm Intelligence
  MSG_LEADER_ELECTION = 0x30, // Leader election protocol
  MSG_CONSENSUS_VOTE = 0x31,  // Collective decision making
  MSG_AREA_CLAIM = 0x32,     // Claim exploration area
  MSG_AREA_SHARE = 0x33,     // Share exploration data
  
  // Learning & Evolution
  MSG_GENOME_SHARE = 0x40,   // Share evolutionary parameters
  MSG_STRATEGY_SHARE = 0x41, // Share successful strategies
  MSG_LEARN_REQUEST = 0x42,  // Request learning data
  MSG_PERFORMANCE_DATA = 0x43, // Share fitness metrics
  
  // Localization & Navigation
  MSG_LOCALIZATION_REQUEST = 0x50, // Request distance measurement
  MSG_LOCALIZATION_RESPONSE = 0x51, // Distance measurement response
  MSG_BEACON_PING = 0x52,    // Audio beacon for ranging
  MSG_POSITION_SHARE = 0x53  // Share precise position data
};

enum MessagePriority {
  PRIORITY_LOW = 1,      // Status updates, sensor data
  PRIORITY_NORMAL = 2,   // Task assignments, coordination
  PRIORITY_HIGH = 3,     // Emergency commands, critical data
  PRIORITY_URGENT = 4    // Safety stops, collision warnings
};

// ═══════════════════════════════════════════════════════════
// 📦 MESSAGE STRUCTURES
// ═══════════════════════════════════════════════════════════

// Base message header (all messages start with this)
struct MessageHeader {
  uint8_t messageType;      // MessageType enum
  uint8_t priority;         // MessagePriority enum
  uint8_t senderType;       // BotType enum
  uint8_t sequenceNumber;   // Message sequence (0-255)
  uint32_t timestamp;       // Milliseconds since boot
  uint8_t checksum;         // Simple checksum for integrity
  uint8_t reserved[2];      // Future expansion
} __attribute__((packed));

// Discovery message payload
struct DiscoveryPayload {
  uint8_t botType;          // BotType enum
  uint8_t currentRole;      // SwarmRole enum
  uint16_t generation;      // Current evolution generation
  float fitnessScore;       // Current fitness (0.0-1.0)
  uint32_t uptime;          // Milliseconds since boot
  uint8_t capabilities[4];  // Capability flags
} __attribute__((packed));

// Status update payload
struct StatusPayload {
  uint8_t currentRole;      // SwarmRole enum
  uint8_t batteryLevel;     // Battery percentage (0-100)
  uint16_t generation;      // Evolution generation
  float fitnessScore;       // Current fitness
  uint8_t emotionalState[4]; // [frustration, confidence, curiosity, cooperation]
  uint16_t messagesSent;    // Total messages sent
  uint16_t messagesReceived; // Total messages received
} __attribute__((packed));

// Sensor data payload (variable based on bot type)
struct SensorPayload {
  uint8_t sensorType;       // Sensor identifier
  uint32_t timestamp;       // Reading timestamp
  float value1;             // Primary sensor value
  float value2;             // Secondary sensor value
  float value3;             // Tertiary sensor value
  uint8_t confidence;       // Reading confidence (0-100)
  uint8_t reserved[3];      // Future expansion
} __attribute__((packed));

// Position update payload
struct PositionPayload {
  float x;                  // X coordinate (relative)
  float y;                  // Y coordinate (relative)
  float heading;            // Direction in degrees (0-360)
  float velocity;           // Current speed
  uint8_t confidence;       // Position confidence (0-100)
  uint8_t reserved[3];      // Future expansion
} __attribute__((packed));

// Task assignment payload
struct TaskPayload {
  uint8_t taskType;         // Task type identifier
  uint8_t taskPriority;     // Task priority (1-10)
  uint16_t taskId;          // Unique task identifier
  uint32_t duration;        // Expected duration (ms)
  float parameters[4];      // Task-specific parameters
  uint8_t requiresResponse; // Response required flag
  uint8_t reserved[3];      // Future expansion
} __attribute__((packed));

// Genome sharing payload
struct GenomePayload {
  uint16_t generation;      // Source generation
  float fitnessScore;       // Source fitness
  uint8_t parameters[32];   // Encoded genome parameters
  uint8_t mutationRate;     // Current mutation rate
  uint8_t reserved[3];      // Future expansion
} __attribute__((packed));

// Localization payload for audio beacon ranging
struct LocalizationPayload {
  uint8_t requestType;      // 0=Request, 1=Response, 2=BeaconPing, 3=PositionShare
  uint32_t beaconTimestamp; // Timestamp when beacon was sent (microseconds)
  uint32_t responseTime;    // Response time for ranging calculation
  float measuredDistance;   // Calculated distance (meters)
  float senderX;            // Sender's X position
  float senderY;            // Sender's Y position
  float senderHeading;      // Sender's heading (radians)
  uint16_t beaconFrequency; // Audio beacon frequency (Hz)
  uint8_t signalStrength;   // Audio signal strength (0-255)
  uint8_t reserved[3];      // Future expansion
} __attribute__((packed));

// Complete message structure (max ESP-NOW payload is 250 bytes)
struct SwarmMessage {
  MessageHeader header;
  union {
    DiscoveryPayload discovery;
    StatusPayload status;
    SensorPayload sensor;
    PositionPayload position;
    TaskPayload task;
    GenomePayload genome;
    LocalizationPayload localization; // Audio beacon ranging data
    uint8_t rawData[200];   // Raw data buffer
  } payload;
} __attribute__((packed));

// ═══════════════════════════════════════════════════════════
// 🌐 SWARM NETWORK MANAGEMENT
// ═══════════════════════════════════════════════════════════

struct SwarmPeer {
  uint8_t macAddress[6];    // Peer MAC address
  BotType botType;          // Type of bot
  SwarmRole currentRole;    // Current swarm role
  uint32_t lastSeen;        // Last message timestamp
  uint16_t generation;      // Peer's generation
  float fitnessScore;       // Peer's fitness
  uint16_t messagesSent;    // Messages sent to this peer
  uint16_t messagesReceived; // Messages received from peer
  uint8_t signalStrength;   // RSSI indicator
  bool isActive;            // Peer active flag
};

#define MAX_SWARM_PEERS 8    // Maximum peers in swarm
#define DISCOVERY_INTERVAL 5000  // Discovery broadcast interval (ms)
#define HEARTBEAT_INTERVAL 2000  // Heartbeat interval (ms)
#define PEER_TIMEOUT 10000   // Peer timeout (ms)
#define MAX_RETRIES 3        // Message retry attempts

// ═══════════════════════════════════════════════════════════
// 🧠 SWARM INTELLIGENCE PROTOCOLS
// ═══════════════════════════════════════════════════════════

enum SwarmBehavior {
  BEHAVIOR_EXPLORE_INDEPENDENT = 0x01,  // Independent exploration
  BEHAVIOR_EXPLORE_COORDINATED = 0x02,  // Coordinated area coverage
  BEHAVIOR_FOLLOW_LEADER = 0x03,        // Follow designated leader
  BEHAVIOR_PATROL_FORMATION = 0x04,     // Maintain patrol formation
  BEHAVIOR_CONVERGE_TARGET = 0x05,      // Converge on target
  BEHAVIOR_DISPERSE_COVERAGE = 0x06,    // Spread for max coverage
  BEHAVIOR_EMERGENCY_REGROUP = 0x07     // Emergency regrouping
};

enum TaskType {
  TASK_EXPLORE_AREA = 0x01,      // Explore specific area
  TASK_GUARD_PERIMETER = 0x02,   // Guard perimeter points
  TASK_SEARCH_OBJECT = 0x03,     // Search for specific object
  TASK_FOLLOW_PATH = 0x04,       // Follow predetermined path
  TASK_MONITOR_POSITION = 0x05,  // Monitor specific position
  TASK_RETURN_BASE = 0x06,       // Return to base location
  TASK_ASSIST_PEER = 0x07        // Assist another bot
};

enum DecisionCriteria {
  DECIDE_BY_FITNESS = 0x01,      // Highest fitness leads
  DECIDE_BY_GENERATION = 0x02,   // Highest generation leads
  DECIDE_BY_SENSOR_QUALITY = 0x03, // Best sensors lead
  DECIDE_BY_BATTERY = 0x04,      // Highest battery leads
  DECIDE_BY_PROXIMITY = 0x05,    // Closest to objective leads
  DECIDE_BY_VOTE = 0x06          // Democratic voting
};

// ═══════════════════════════════════════════════════════════
// 🔧 UTILITY FUNCTIONS & MACROS
// ═══════════════════════════════════════════════════════════

#define BROADCAST_MAC {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

// Message validation
inline bool isValidMessage(const SwarmMessage* msg) {
  if (!msg) return false;
  if (msg->header.messageType == 0 || msg->header.messageType > 0x50) return false;
  if (msg->header.priority == 0 || msg->header.priority > 4) return false;
  return true;
}

// Calculate simple checksum
inline uint8_t calculateChecksum(const uint8_t* data, size_t length) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < length; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

// MAC address comparison
inline bool macEquals(const uint8_t* mac1, const uint8_t* mac2) {
  return memcmp(mac1, mac2, 6) == 0;
}

// Convert MAC to string for debugging
inline String macToString(const uint8_t* mac) {
  char buffer[18];
  snprintf(buffer, sizeof(buffer), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buffer);
}

// ═══════════════════════════════════════════════════════════
// 📊 COMMUNICATION STATISTICS
// ═══════════════════════════════════════════════════════════

struct CommStats {
  uint32_t messagesSent;        // Total messages sent
  uint32_t messagesReceived;    // Total messages received
  uint32_t messagesDropped;     // Messages dropped/failed
  uint32_t discoveryCount;      // Discovery messages sent
  uint32_t heartbeatCount;      // Heartbeats sent
  uint32_t lastMessageTime;     // Last message timestamp
  uint8_t activeePeers;         // Currently active peers
  uint8_t maxPeers;             // Maximum peers seen
  float averageRssi;            // Average signal strength
  uint32_t commErrors;          // Communication errors
};

#endif // SWARM_ESPNOW_H