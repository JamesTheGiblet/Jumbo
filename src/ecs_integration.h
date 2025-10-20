/*
🧬 Project Jumbo: Evolutionary Code System (ECS) v2.0 - ESP32 Integration
ESP32 C++ integration layer for autonomous code evolution

This header provides the interface between ESP32 bot code and the 
Python-based Evolutionary Code System, enabling real-time parameter
evolution, performance monitoring, and intelligent mutation application.

Key Features:
- Real-time parameter evolution via ESP-NOW
- Performance metric collection and reporting
- Automatic error detection and reporting
- Memory-efficient parameter storage in EEPROM
- Seamless integration with existing bot code
*/

#ifndef ECS_INTEGRATION_H
#define ECS_INTEGRATION_H

#include <Arduino.h>
#include <EEPROM.h>
#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// ECS Configuration Constants
#define ECS_VERSION "2.0"
#define ECS_EEPROM_SIZE 512
#define ECS_PARAM_COUNT 16
#define ECS_ERROR_LOG_SIZE 32
#define ECS_PERFORMANCE_BUFFER_SIZE 64
#define ECS_MESSAGE_BUFFER_SIZE 256

// Message Types for ECS Communication
enum ECSMessageType {
    ECS_PARAM_UPDATE = 0x10,        // Parameter evolution from ECS
    ECS_PERFORMANCE_REPORT = 0x11,  // Bot performance metrics
    ECS_ERROR_REPORT = 0x12,        // Error/exception reporting
    ECS_FITNESS_REQUEST = 0x13,     // Request fitness evaluation
    ECS_MUTATION_APPLY = 0x14,      // Apply specific mutation
    ECS_STATUS_QUERY = 0x15,        // System status check
    ECS_RESET_PARAMS = 0x16,        // Reset to baseline parameters
    ECS_BACKUP_GENOME = 0x17        // Backup current genome
};

// Error Severity Levels
enum ErrorSeverity {
    ERROR_INFO = 0,
    ERROR_WARNING = 1,
    ERROR_CRITICAL = 2,
    ERROR_FATAL = 3
};

// Performance Metric Types
enum MetricType {
    METRIC_TASK_COMPLETION = 0,
    METRIC_ENERGY_EFFICIENCY = 1,
    METRIC_COMMUNICATION_SUCCESS = 2,
    METRIC_COLLISION_AVOIDANCE = 3,
    METRIC_COOPERATION_SCORE = 4,
    METRIC_ADAPTATION_SPEED = 5,
    METRIC_SENSOR_ACCURACY = 6,
    METRIC_MOVEMENT_EFFICIENCY = 7
};

// Evolvable Parameter Structure
struct EvolvableParameter {
    char name[16];          // Parameter name
    int32_t value;          // Current value
    int32_t min_value;      // Minimum allowed value
    int32_t max_value;      // Maximum allowed value
    int32_t default_value;  // Baseline/default value
    uint16_t mutation_count; // Number of mutations applied
    float fitness_impact;   // Historical fitness impact
    uint32_t last_updated;  // Timestamp of last update
};

// Performance Metric Structure
struct PerformanceMetric {
    MetricType type;
    float value;
    uint32_t timestamp;
    uint16_t sample_count;
    float running_average;
};

// Error Log Entry
struct ErrorLogEntry {
    ErrorSeverity severity;
    uint32_t timestamp;
    uint16_t error_code;
    char description[32];
    char function_name[16];
};

// ECS Integration Class
class ECSIntegration {
private:
    // Parameter management
    EvolvableParameter parameters[ECS_PARAM_COUNT];
    uint8_t param_count;
    
    // Performance tracking
    PerformanceMetric metrics[8];  // One for each MetricType
    uint16_t performance_sample_count;
    
    // Error logging
    ErrorLogEntry error_log[ECS_ERROR_LOG_SIZE];
    uint8_t error_log_index;
    uint8_t error_count;
    
    // Communication
    uint8_t ecs_coordinator_mac[6];
    bool ecs_connected;
    uint32_t last_heartbeat;
    uint16_t message_sequence;
    
    // Evolution tracking
    uint32_t generation;
    float current_fitness;
    uint32_t mutation_attempts;
    uint32_t successful_mutations;
    
    // Internal methods
    void loadParametersFromEEPROM();
    void saveParametersToEEPROM();
    void updateRunningAverages();
    void pruneErrorLog();
    bool validateParameter(const char* name, int32_t value);
    void sendESPNowMessage(ECSMessageType type, const char* payload);
    
public:
    ECSIntegration();
    ~ECSIntegration();
    
    // Initialization
    bool initialize(const uint8_t coordinator_mac[6]);
    void setCoordinatorMAC(const uint8_t mac[6]);
    
    // Parameter management
    bool registerParameter(const char* name, int32_t initial_value, 
                          int32_t min_val, int32_t max_val);
    int32_t getParameter(const char* name);
    bool setParameter(const char* name, int32_t value);
    void resetParametersToDefault();
    uint8_t getParameterCount() const { return param_count; }
    
    // Performance monitoring
    void reportMetric(MetricType type, float value);
    float getMetricAverage(MetricType type);
    void calculateFitnessScore();
    float getCurrentFitness() const { return current_fitness; }
    
    // Error handling
    void reportError(ErrorSeverity severity, uint16_t error_code, 
                    const char* description, const char* function_name = nullptr);
    void reportException(const char* exception_type, const char* function_name);
    uint8_t getErrorCount(ErrorSeverity min_severity = ERROR_WARNING);
    
    // Evolution interface
    void requestEvolution(const char* trigger_reason = nullptr);
    void applyMutation(const char* parameter_name, int32_t new_value);
    void reportEvolutionResult(bool success, float fitness_delta);
    
    // Communication
    void sendPerformanceReport();
    void sendErrorReport();
    void sendHeartbeat();
    bool isConnectedToECS() const { return ecs_connected; }
    
    // ESP-NOW callback handling
    void handleESPNowMessage(const uint8_t* mac, const uint8_t* data, int len);
    
    // System status
    void printSystemStatus();
    void getStatusJSON(char* buffer, size_t buffer_size);
    
    // Evolution statistics
    uint32_t getGeneration() const { return generation; }
    float getSuccessRate() const;
    uint32_t getTotalMutations() const { return mutation_attempts; }
    
    // Periodic update (call from main loop)
    void update();
};

// Global ECS instance (to be used by bots)
extern ECSIntegration ecs;

// Convenience macros for parameter access
#define ECS_GET_PARAM(name) ecs.getParameter(name)
#define ECS_SET_PARAM(name, value) ecs.setParameter(name, value)
#define ECS_REPORT_METRIC(type, value) ecs.reportMetric(type, value)
#define ECS_REPORT_ERROR(severity, code, desc) ecs.reportError(severity, code, desc, __FUNCTION__)

// Performance measurement helpers
#define ECS_START_TIMING() unsigned long _ecs_start_time = millis()
#define ECS_END_TIMING(metric_type) do { \
    unsigned long _duration = millis() - _ecs_start_time; \
    float _efficiency = 1000.0 / max(1UL, _duration); \
    ecs.reportMetric(metric_type, _efficiency); \
} while(0)

// Error handling wrapper
#define ECS_TRY_CATCH(code, error_desc) do { \
    try { \
        code; \
    } catch (...) { \
        ecs.reportError(ERROR_CRITICAL, __LINE__, error_desc, __FUNCTION__); \
    } \
} while(0)

// Automatic parameter evolution triggers
class AutoEvolutionTrigger {
private:
    float fitness_threshold;
    uint32_t stagnation_timeout;
    uint32_t last_improvement;
    float last_fitness;
    
public:
    AutoEvolutionTrigger(float threshold = -0.1, uint32_t timeout_ms = 300000) 
        : fitness_threshold(threshold), stagnation_timeout(timeout_ms), 
          last_improvement(millis()), last_fitness(0.0) {}
    
    bool shouldTriggerEvolution() {
        float current_fitness = ecs.getCurrentFitness();
        uint32_t now = millis();
        
        // Trigger if fitness decline
        if (current_fitness < last_fitness + fitness_threshold) {
            last_improvement = now;
            last_fitness = current_fitness;
            return true;
        }
        
        // Trigger if stagnation
        if (now - last_improvement > stagnation_timeout) {
            last_improvement = now;
            return true;
        }
        
        // Update tracking
        if (current_fitness > last_fitness) {
            last_improvement = now;
        }
        last_fitness = current_fitness;
        
        return false;
    }
};

// Task-specific evolution contexts
namespace ECSContext {
    // Movement optimization context
    struct MovementContext {
        static void registerParameters() {
            ecs.registerParameter("motorSpeed", 200, 50, 255);
            ecs.registerParameter("turnSpeed", 150, 50, 200);
            ecs.registerParameter("backupDuration", 600, 100, 2000);
            ecs.registerParameter("turnDuration", 350, 100, 1000);
        }
        
        static void reportCollision() {
            ecs.reportError(ERROR_WARNING, 1001, "Collision detected", "movement");
            ecs.reportMetric(METRIC_COLLISION_AVOIDANCE, 0.0);
        }
        
        static void reportSuccessfulNavigation(unsigned long duration) {
            float efficiency = 10000.0 / max(1UL, duration);
            ecs.reportMetric(METRIC_MOVEMENT_EFFICIENCY, efficiency);
        }
    };
    
    // Sensor optimization context
    struct SensorContext {
        static void registerParameters() {
            ecs.registerParameter("obstacleThreshold", 200, 50, 500);
            ecs.registerParameter("clearThreshold", 300, 100, 600);
            ecs.registerParameter("sensorSensitivity", 100, 50, 150);
            ecs.registerParameter("noiseFilter", 10, 0, 50);
        }
        
        static void reportSensorReading(int distance, bool is_accurate) {
            if (is_accurate) {
                ecs.reportMetric(METRIC_SENSOR_ACCURACY, 1.0);
            } else {
                ecs.reportMetric(METRIC_SENSOR_ACCURACY, 0.0);
                ecs.reportError(ERROR_WARNING, 2001, "Sensor reading error", "sensor");
            }
        }
    };
    
    // Communication optimization context
    struct CommunicationContext {
        static void registerParameters() {
            ecs.registerParameter("signalStrength", 100, 50, 200);
            ecs.registerParameter("retryCount", 3, 1, 10);
            ecs.registerParameter("timeoutMs", 1000, 100, 5000);
            ecs.registerParameter("bufferSize", 64, 32, 256);
        }
        
        static void reportMessageSuccess() {
            ecs.reportMetric(METRIC_COMMUNICATION_SUCCESS, 1.0);
        }
        
        static void reportMessageFailure() {
            ecs.reportMetric(METRIC_COMMUNICATION_SUCCESS, 0.0);
            ecs.reportError(ERROR_WARNING, 3001, "Message transmission failed", "communication");
        }
    };
    
    // Swarm coordination context
    struct SwarmContext {
        static void registerParameters() {
            ecs.registerParameter("leadershipWeight", 50, 0, 100);
            ecs.registerParameter("followDistance", 100, 50, 200);
            ecs.registerParameter("consensusTimeout", 5000, 1000, 15000);
            ecs.registerParameter("taskPriority", 1, 0, 10);
        }
        
        static void reportCoordinationSuccess() {
            ecs.reportMetric(METRIC_COOPERATION_SCORE, 1.0);
        }
        
        static void reportCoordinationFailure() {
            ecs.reportMetric(METRIC_COOPERATION_SCORE, 0.0);
            ecs.reportError(ERROR_WARNING, 4001, "Coordination failure", "swarm");
        }
    };
}

#endif // ECS_INTEGRATION_H