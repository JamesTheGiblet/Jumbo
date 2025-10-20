# 📡 Project Jumbo: API Reference

## Overview

This document provides comprehensive API documentation for all communication interfaces in the Project Jumbo ecosystem, including ESP-NOW protocols, WebSocket message formats, and PC MCU API endpoints.

---

## 🤖 Bot Message Types

### 1. Status Messages

#### Bot Status Report

```json
{
  "type": "status_report",
  "timestamp": 1729440615123,
  "bot_id": "WHEELIE_001",
  "generation": 42,
  "fitness_score": 0.847,
  "capabilities": ["movement", "sensing", "communication"],
  "status": "active"
}
```

#### Heartbeat Message

```json
{
  "type": "heartbeat",
  "timestamp": 1729440615123,
  "bot_id": "SPEEDIE_002",
  "sequence": 12345,
  "battery_voltage": 3.7,
  "uptime": 86400
}
```

### 2. Sensor Data Messages

#### Distance Sensor Reading

```json
{
  "type": "sensor_data",
  "timestamp": 1729440615123,
  "bot_id": "WHEELIE_001",
  "sensor_type": "distance",
  "data": {
    "distance_cm": 25.4,
    "confidence": 0.95,
    "sensor_id": "vl53l0x_front"
  }
}
```

#### Motion Detection

```json
{
  "type": "sensor_data",
  "timestamp": 1729440615123,
  "bot_id": "WHEELIE_001",
  "sensor_type": "motion",
  "data": {
    "motion_detected": true,
    "detection_strength": 0.78,
    "sensor_id": "rcwl_0516"
  }
}
```

#### IMU Data (SPEEDIE)

```json
{
  "type": "sensor_data",
  "timestamp": 1729440615123,
  "bot_id": "SPEEDIE_002",
  "sensor_type": "imu",
  "data": {
    "acceleration": {
      "x": 0.12,
      "y": -0.05,
      "z": 9.78
    },
    "gyroscope": {
      "x": 0.002,
      "y": -0.001,
      "z": 0.045
    },
    "heading": 142.8
  }
}
```

### 3. Evolutionary Data Messages

#### Genetic Parameters

```json
{
  "type": "genetic_data",
  "timestamp": 1729440615123,
  "bot_id": "WHEELIE_001",
  "generation": 42,
  "parameters": {
    "motor_speed": 235,
    "sensor_threshold": 150,
    "curiosity_factor": 0.73,
    "exploration_radius": 200,
    "cooperation_bias": 0.65
  },
  "fitness_components": {
    "exploration": 0.82,
    "obstacle_avoidance": 0.91,
    "cooperation": 0.78,
    "energy_efficiency": 0.85
  }
}
```

#### Mutation Report

```json
{
  "type": "mutation_event",
  "timestamp": 1729440615123,
  "bot_id": "SPEEDIE_002",
  "generation": 43,
  "mutations": [
    {
      "parameter": "motor_speed",
      "old_value": 220,
      "new_value": 235,
      "mutation_type": "gaussian",
      "magnitude": 0.068
    }
  ]
}
```

### 4. Emotional State Messages

#### Emotional Status Update

```json
{
  "type": "emotional_state",
  "timestamp": 1729440615123,
  "bot_id": "WHEELIE_001",
  "emotional_state": {
    "curiosity": 0.8,
    "confidence": 0.7,
    "cooperation": 0.9,
    "energy_level": 0.85,
    "frustration": 0.2,
    "satisfaction": 0.75
  },
  "context": "exploring_new_area",
  "trigger_event": "obstacle_overcome"
}
```

### 5. Communication & Coordination Messages

#### Inter-Bot Signal

```json
{
  "type": "bot_signal",
  "timestamp": 1729440615123,
  "sender": "SPEEDIE_002",
  "receiver": "WHEELIE_001",
  "signal_data": {
    "signal_type": "coordination",
    "frequency": 2000,
    "duration": 150,
    "pattern": "short-long-short",
    "context": "obstacle_detected",
    "urgency": 0.8
  }
}
```

#### Position Report

```json
{
  "type": "position_update",
  "timestamp": 1729440615123,
  "bot_id": "WHEELIE_001",
  "position": {
    "x": 125.6,
    "y": 87.3,
    "heading": 142.8,
    "confidence": 0.92
  },
  "velocity": {
    "linear": 15.2,
    "angular": 0.05
  }
}
```

#### Audio Beacon Localization

```json
{
  "type": "audio_localization",
  "timestamp": 1729440615123,
  "bot_id": "SPEEDIE_002",
  "beacon_data": {
    "beacon_id": "SPEEDIE_001",
    "distance": 47.3,
    "angle": 73.5,
    "time_of_flight": 0.137,
    "signal_strength": 0.85,
    "frequency": 2000
  }
}
```

---

## 🌐 ESP-NOW Protocol Specification

### 1. Physical Layer

#### Network Configuration

```cpp
// Network settings
#define ESPNOW_CHANNEL 1
#define MAX_PACKET_SIZE 250
#define MAX_PEERS 20
#define RETRY_COUNT 3
#define ACK_TIMEOUT 100  // milliseconds
```

#### Device Registration

```cpp
typedef struct {
    uint8_t mac_address[6];
    char bot_id[16];
    uint8_t bot_type;  // 1=WHEELIE, 2=SPEEDIE, 3=MICRO_BOT
    uint32_t capabilities;
    uint16_t firmware_version;
} device_info_t;
```

### 2. Message Header Format

```cpp
typedef struct {
    uint8_t message_type;     // Message type identifier
    uint8_t sender_id;        // Sender device ID
    uint8_t receiver_id;      // Receiver device ID (0xFF = broadcast)
    uint8_t sequence_number;  // Message sequence for ordering
    uint16_t payload_length;  // Length of payload data
    uint8_t checksum;         // Simple checksum for integrity
    uint8_t reserved;         // Reserved for future use
} espnow_header_t;
```

### 3. Message Types Enumeration

```cpp
typedef enum {
    // System messages (0x00-0x0F)
    MSG_HEARTBEAT = 0x00,
    MSG_DISCOVERY = 0x01,
    MSG_REGISTRATION = 0x02,
    MSG_ACK = 0x03,
    MSG_ERROR = 0x04,
    
    // Sensor data (0x10-0x1F)
    MSG_SENSOR_DISTANCE = 0x10,
    MSG_SENSOR_MOTION = 0x11,
    MSG_SENSOR_IMU = 0x12,
    MSG_SENSOR_AUDIO = 0x13,
    MSG_SENSOR_BATTERY = 0x14,
    
    // Navigation (0x20-0x2F)
    MSG_POSITION = 0x20,
    MSG_VELOCITY = 0x21,
    MSG_HEADING = 0x22,
    MSG_WAYPOINT = 0x23,
    
    // Coordination (0x30-0x3F)
    MSG_SIGNAL = 0x30,
    MSG_FORMATION = 0x31,
    MSG_MISSION = 0x32,
    MSG_STATUS_REQUEST = 0x33,
    
    // Evolution (0x40-0x4F)
    MSG_GENETIC_DATA = 0x40,
    MSG_FITNESS_SCORE = 0x41,
    MSG_MUTATION = 0x42,
    MSG_SELECTION = 0x43,
    
    // Audio localization (0x50-0x5F)
    MSG_AUDIO_BEACON = 0x50,
    MSG_LOCALIZATION_REQUEST = 0x51,
    MSG_LOCALIZATION_RESPONSE = 0x52,
    
    // Emergency (0xF0-0xFF)
    MSG_EMERGENCY = 0xF0,
    MSG_SHUTDOWN = 0xFE,
    MSG_RESET = 0xFF
} message_type_t;
```

### 4. Function Signatures

#### Core ESP-NOW Functions

```cpp
// Initialize ESP-NOW communication
esp_err_t espnow_init(void);

// Register peer device
esp_err_t espnow_add_peer(const uint8_t *mac_addr, uint8_t channel);

// Send message to specific peer
esp_err_t espnow_send_message(const uint8_t *mac_addr, 
                             message_type_t msg_type,
                             const void *data, 
                             size_t data_len);

// Broadcast message to all peers
esp_err_t espnow_broadcast_message(message_type_t msg_type,
                                  const void *data,
                                  size_t data_len);

// Set receive callback
void espnow_set_recv_callback(void (*callback)(const uint8_t *mac_addr,
                                              const uint8_t *data,
                                              int data_len));
```

#### Message Construction Functions

```cpp
// Create heartbeat message
size_t create_heartbeat_message(uint8_t *buffer, 
                               uint32_t sequence,
                               float battery_voltage,
                               uint32_t uptime);

// Create sensor data message
size_t create_sensor_message(uint8_t *buffer,
                           uint8_t sensor_type,
                           const void *sensor_data,
                           size_t data_size);

// Create position update message
size_t create_position_message(uint8_t *buffer,
                             float x, float y, float heading,
                             float confidence);

// Create coordination signal
size_t create_signal_message(uint8_t *buffer,
                           uint16_t frequency,
                           uint16_t duration,
                           const char *pattern,
                           const char *context);
```

#### Audio Localization Functions

```cpp
// Generate audio beacon
void audio_beacon_start(uint16_t frequency, uint16_t duration);
void audio_beacon_stop(void);

// Time-of-flight measurement
float measure_audio_distance(uint16_t frequency, float timeout_ms);

// Triangulation calculation
bool calculate_position(const audio_measurement_t *measurements,
                       size_t measurement_count,
                       float *x, float *y);
```

---

## 🖥️ PC MCU API Endpoints

### 1. WebSocket Connection API

#### Connection Establishment

```javascript
// Connect to MICRO BOT bridge
const ws = new WebSocket('ws://192.168.1.207:8766');

// Connection events
ws.onopen = function(event) {
    console.log('Connected to MICRO BOT bridge');
};

ws.onmessage = function(event) {
    const message = JSON.parse(event.data);
    handleBridgeMessage(message);
};

ws.onerror = function(error) {
    console.error('WebSocket error:', error);
};
```

#### Authentication Message

```json
{
  "type": "auth_request",
  "timestamp": 1729440615123,
  "client_type": "pc_mcu",
  "client_version": "1.0.0",
  "capabilities": ["bot_monitoring", "mission_control", "data_analysis"]
}
```

### 2. Bot Management API

#### Get Bot List

```json
// Request
{
  "type": "get_bot_list",
  "timestamp": 1729440615123,
  "request_id": "req_001"
}

// Response
{
  "type": "bot_list_response",
  "timestamp": 1729440615124,
  "request_id": "req_001",
  "bots": [
    {
      "bot_id": "WHEELIE_001",
      "status": "online",
      "last_seen": 1729440615120,
      "generation": 42,
      "fitness_score": 0.847
    },
    {
      "bot_id": "SPEEDIE_002", 
      "status": "online",
      "last_seen": 1729440615118,
      "generation": 38,
      "fitness_score": 0.792
    }
  ]
}
```

#### Get Bot Details

```json
// Request
{
  "type": "get_bot_details",
  "timestamp": 1729440615123,
  "request_id": "req_002",
  "bot_id": "WHEELIE_001"
}

// Response
{
  "type": "bot_details_response",
  "timestamp": 1729440615125,
  "request_id": "req_002",
  "bot_data": {
    "bot_id": "WHEELIE_001",
    "generation": 42,
    "fitness_score": 0.847,
    "emotional_state": {
      "curiosity": 0.8,
      "confidence": 0.7,
      "cooperation": 0.9
    },
    "parameters": {
      "motor_speed": 235,
      "sensor_threshold": 150
    },
    "position": {
      "x": 125.6,
      "y": 87.3,
      "heading": 142.8
    },
    "sensor_data": {
      "distance": 25.4,
      "motion_detected": false,
      "battery_voltage": 3.7
    }
  }
}
```

### 3. Mission Control API

#### Create Mission

```json
// Request
{
  "type": "create_mission",
  "timestamp": 1729440615123,
  "request_id": "req_003",
  "mission_data": {
    "mission_type": "exploration",
    "target_bots": ["WHEELIE_001", "SPEEDIE_002"],
    "parameters": {
      "area_bounds": {
        "x_min": 0, "x_max": 500,
        "y_min": 0, "y_max": 500
      },
      "duration": 300,
      "coordination_required": true
    },
    "priority": "normal"
  }
}

// Response
{
  "type": "mission_created_response",
  "timestamp": 1729440615126,
  "request_id": "req_003",
  "mission_id": "mission_001",
  "status": "created",
  "assigned_bots": ["WHEELIE_001", "SPEEDIE_002"]
}
```

#### Mission Status Update

```json
{
  "type": "mission_status_update",
  "timestamp": 1729440615127,
  "mission_id": "mission_001",
  "status": "in_progress",
  "progress": 0.35,
  "bot_status": {
    "WHEELIE_001": "exploring",
    "SPEEDIE_002": "coordinating"
  },
  "estimated_completion": 1729440735
}
```

### 4. Data Analysis API

#### Get Performance Analytics

```json
// Request
{
  "type": "get_analytics",
  "timestamp": 1729440615123,
  "request_id": "req_004",
  "analytics_type": "fitness_trends",
  "time_range": {
    "start": 1729354215,
    "end": 1729440615
  },
  "bot_ids": ["WHEELIE_001", "SPEEDIE_002"]
}

// Response
{
  "type": "analytics_response",
  "timestamp": 1729440615128,
  "request_id": "req_004",
  "analytics_data": {
    "fitness_trends": [
      {
        "timestamp": 1729354215,
        "WHEELIE_001": 0.723,
        "SPEEDIE_002": 0.681
      },
      {
        "timestamp": 1729397415,
        "WHEELIE_001": 0.789,
        "SPEEDIE_002": 0.745
      },
      {
        "timestamp": 1729440615,
        "WHEELIE_001": 0.847,
        "SPEEDIE_002": 0.792
      }
    ]
  }
}
```

### 5. System Control API

#### Emergency Stop

```json
{
  "type": "emergency_stop",
  "timestamp": 1729440615123,
  "reason": "user_requested",
  "target_bots": "all"  // or specific bot IDs
}
```

#### System Status Request

```json
// Request
{
  "type": "get_system_status",
  "timestamp": 1729440615123,
  "request_id": "req_005"
}

// Response
{
  "type": "system_status_response",
  "timestamp": 1729440615129,
  "request_id": "req_005",
  "system_status": {
    "bridge_uptime": 86400,
    "connected_bots": 2,
    "active_missions": 1,
    "message_throughput": 15.7,
    "error_count": 0,
    "memory_usage": 0.34,
    "cpu_usage": 0.12
  }
}
```

---

## 🔧 Python Function Signatures

### 1. PC MCU Core Functions

#### Connection Management

```python
class PCMCUController:
    def __init__(self, bridge_ip: str = "192.168.1.207", bridge_port: int = 8766):
        """Initialize PC MCU controller"""
        
    async def connect(self) -> bool:
        """Establish WebSocket connection to MICRO BOT bridge"""
        
    async def disconnect(self) -> None:
        """Close connection gracefully"""
        
    def is_connected(self) -> bool:
        """Check connection status"""
```

#### Bot Management

```python
async def get_bot_list(self) -> List[BotInfo]:
    """Retrieve list of connected bots"""
    
async def get_bot_details(self, bot_id: str) -> BotData:
    """Get detailed information for specific bot"""
    
async def send_bot_command(self, bot_id: str, command: dict) -> bool:
    """Send command to specific bot"""
    
def update_bot_data(self, bot_id: str, data: dict) -> None:
    """Update local bot data cache"""
```

#### Mission Control

```python
class MissionPlanner:
    def create_mission(self, mission_type: str, bot_ids: List[str], 
                      parameters: dict) -> str:
        """Create new mission and return mission ID"""
        
    def get_mission_status(self, mission_id: str) -> MissionStatus:
        """Get current mission status"""
        
    def cancel_mission(self, mission_id: str) -> bool:
        """Cancel active mission"""
        
    def get_active_missions(self) -> List[MissionInfo]:
        """Get list of all active missions"""
```

#### Analytics Functions

```python
class AnalyticsEngine:
    def calculate_fitness_trends(self, bot_ids: List[str], 
                               time_range: TimeRange) -> DataFrame:
        """Calculate fitness score trends over time"""
        
    def analyze_communication_patterns(self, 
                                     communication_logs: List[dict]) -> dict:
        """Analyze emergent communication patterns"""
        
    def generate_performance_report(self, bot_id: str) -> PerformanceReport:
        """Generate comprehensive performance report"""
        
    def export_data(self, format: str = "csv") -> str:
        """Export analytics data to file"""
```

### 2. MICRO BOT Bridge Functions

#### Bridge Server Core

```python
class ESPNowBridge:
    def __init__(self, bot_port: int = 8765, pc_port: int = 8766):
        """Initialize bridge server"""
        
    async def start_server(self) -> None:
        """Start WebSocket servers for bots and PC"""
        
    async def handle_bot_connection(self, websocket, path: str) -> None:
        """Handle ESP32 bot connections"""
        
    async def handle_pc_connection(self, websocket, path: str) -> None:
        """Handle PC MCU connections"""
        
    def route_message(self, message: dict, source: str) -> None:
        """Route messages between bots and PC"""
```

#### Message Processing

```python
def validate_message(self, message: dict) -> bool:
    """Validate message format and content"""
    
def process_bot_message(self, bot_id: str, message: dict) -> None:
    """Process message from ESP32 bot"""
    
def process_pc_command(self, command: dict) -> dict:
    """Process command from PC MCU"""
    
def broadcast_to_bots(self, message: dict, exclude: List[str] = None) -> None:
    """Broadcast message to all connected bots"""
```

---

## 🚨 Error Codes & Handling

### 1. System Error Codes

```python
class ErrorCodes:
    # Connection errors (1000-1099)
    CONNECTION_FAILED = 1000
    CONNECTION_TIMEOUT = 1001
    CONNECTION_LOST = 1002
    AUTHENTICATION_FAILED = 1003
    
    # Message errors (1100-1199) 
    MESSAGE_FORMAT_INVALID = 1100
    MESSAGE_TOO_LARGE = 1101
    MESSAGE_CHECKSUM_FAILED = 1102
    UNKNOWN_MESSAGE_TYPE = 1103
    
    # Bot errors (1200-1299)
    BOT_NOT_FOUND = 1200
    BOT_OFFLINE = 1201
    BOT_BUSY = 1202
    BOT_LOW_BATTERY = 1203
    
    # Mission errors (1300-1399)
    MISSION_NOT_FOUND = 1300
    MISSION_ALREADY_ACTIVE = 1301
    INSUFFICIENT_BOTS = 1302
    INVALID_PARAMETERS = 1303
    
    # System errors (1400-1499)
    BRIDGE_OVERLOAD = 1400
    MEMORY_EXHAUSTED = 1401
    STORAGE_FULL = 1402
    HARDWARE_FAILURE = 1403
```

### 2. Error Response Format

```json
{
  "type": "error_response",
  "timestamp": 1729440615123,
  "request_id": "req_001", 
  "error": {
    "code": 1200,
    "message": "Bot not found",
    "details": "Bot ID 'WHEELIE_003' is not registered in the system",
    "severity": "warning",
    "retry_possible": false
  }
}
```

---

## 📋 API Usage Examples

### 1. Complete Bot Monitoring Example

```python
import asyncio
import websockets
import json

async def monitor_bots():
    uri = "ws://192.168.1.207:8766"
    
    async with websockets.connect(uri) as websocket:
        # Send authentication
        auth_msg = {
            "type": "auth_request",
            "timestamp": int(time.time() * 1000),
            "client_type": "pc_mcu",
            "client_version": "1.0.0"
        }
        await websocket.send(json.dumps(auth_msg))
        
        # Request bot list
        bot_list_msg = {
            "type": "get_bot_list",
            "timestamp": int(time.time() * 1000),
            "request_id": "req_001"
        }
        await websocket.send(json.dumps(bot_list_msg))
        
        # Listen for responses
        async for message in websocket:
            data = json.loads(message)
            
            if data["type"] == "bot_list_response":
                print(f"Connected bots: {len(data['bots'])}")
                for bot in data["bots"]:
                    print(f"- {bot['bot_id']}: {bot['status']}")
                    
            elif data["type"] == "bot_message":
                print(f"Message from {data['bot_id']}: {data['data']}")

# Run the monitor
asyncio.run(monitor_bots())
```

### 2. Mission Creation Example

```python
async def create_exploration_mission(bot_ids):
    uri = "ws://192.168.1.207:8766"
    
    async with websockets.connect(uri) as websocket:
        mission_msg = {
            "type": "create_mission",
            "timestamp": int(time.time() * 1000),
            "request_id": f"req_{int(time.time())}",
            "mission_data": {
                "mission_type": "exploration",
                "target_bots": bot_ids,
                "parameters": {
                    "area_bounds": {
                        "x_min": 0, "x_max": 500,
                        "y_min": 0, "y_max": 500
                    },
                    "duration": 300,
                    "coordination_required": True
                },
                "priority": "normal"
            }
        }
        
        await websocket.send(json.dumps(mission_msg))
        
        # Wait for response
        response = await websocket.recv()
        result = json.loads(response)
        
        if result["type"] == "mission_created_response":
            print(f"Mission {result['mission_id']} created successfully")
            return result["mission_id"]
        else:
            print(f"Mission creation failed: {result}")
            return None
```

---

**This API reference provides comprehensive documentation for all communication interfaces in the Project Jumbo ecosystem. 📡🤖✨**
