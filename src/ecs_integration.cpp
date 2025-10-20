/*
🧬 Project Jumbo: Evolutionary Code System (ECS) v2.0 - ESP32 Implementation
Implementation of the ECS integration layer for ESP32-based bots
*/

#include "ecs_integration.h"

// Global ECS instance
ECSIntegration ecs;

// EEPROM layout constants
#define ECS_EEPROM_MAGIC 0xEC52  // "ECS2" magic number
#define ECS_EEPROM_MAGIC_ADDR 0
#define ECS_EEPROM_VERSION_ADDR 2
#define ECS_EEPROM_PARAM_COUNT_ADDR 4
#define ECS_EEPROM_PARAMS_START 8

ECSIntegration::ECSIntegration() {
    param_count = 0;
    performance_sample_count = 0;
    error_log_index = 0;
    error_count = 0;
    ecs_connected = false;
    last_heartbeat = 0;
    message_sequence = 0;
    generation = 0;
    current_fitness = 0.0;
    mutation_attempts = 0;
    successful_mutations = 0;
    
    // Initialize parameter array
    memset(parameters, 0, sizeof(parameters));
    
    // Initialize metrics
    for (int i = 0; i < 8; i++) {
        metrics[i].type = (MetricType)i;
        metrics[i].value = 0.0;
        metrics[i].timestamp = 0;
        metrics[i].sample_count = 0;
        metrics[i].running_average = 0.0;
    }
    
    // Initialize error log
    memset(error_log, 0, sizeof(error_log));
    
    // Clear coordinator MAC
    memset(ecs_coordinator_mac, 0, sizeof(ecs_coordinator_mac));
}

ECSIntegration::~ECSIntegration() {
    saveParametersToEEPROM();
}

bool ECSIntegration::initialize(const uint8_t coordinator_mac[6]) {
    Serial.println("🧬 Initializing ECS Integration v2.0");
    
    // Initialize EEPROM
    if (!EEPROM.begin(ECS_EEPROM_SIZE)) {
        Serial.println("❌ Failed to initialize EEPROM");
        return false;
    }
    
    // Set coordinator MAC
    if (coordinator_mac) {
        memcpy(ecs_coordinator_mac, coordinator_mac, 6);
        Serial.printf("🎯 ECS Coordinator MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                     coordinator_mac[0], coordinator_mac[1], coordinator_mac[2],
                     coordinator_mac[3], coordinator_mac[4], coordinator_mac[5]);
    }
    
    // Load parameters from EEPROM
    loadParametersFromEEPROM();
    
    // Initialize ESP-NOW if WiFi is available
    if (WiFi.getMode() != WIFI_MODE_NULL) {
        Serial.println("✅ ECS Integration initialized successfully");
        ecs_connected = true;
    } else {
        Serial.println("⚠️  ECS initialized in offline mode (no WiFi)");
    }
    
    return true;
}

void ECSIntegration::setCoordinatorMAC(const uint8_t mac[6]) {
    memcpy(ecs_coordinator_mac, mac, 6);
    ecs_connected = true;
}

bool ECSIntegration::registerParameter(const char* name, int32_t initial_value, 
                                     int32_t min_val, int32_t max_val) {
    if (param_count >= ECS_PARAM_COUNT) {
        reportError(ERROR_CRITICAL, 9001, "Parameter limit exceeded", __FUNCTION__);
        return false;
    }
    
    // Check for duplicate names
    for (uint8_t i = 0; i < param_count; i++) {
        if (strcmp(parameters[i].name, name) == 0) {
            reportError(ERROR_WARNING, 9002, "Duplicate parameter", __FUNCTION__);
            return false;
        }
    }
    
    // Create new parameter
    EvolvableParameter& param = parameters[param_count];
    strncpy(param.name, name, sizeof(param.name) - 1);
    param.name[sizeof(param.name) - 1] = '\0';
    
    param.value = initial_value;
    param.min_value = min_val;
    param.max_value = max_val;
    param.default_value = initial_value;
    param.mutation_count = 0;
    param.fitness_impact = 0.0;
    param.last_updated = millis();
    
    param_count++;
    
    Serial.printf("📝 Registered parameter: %s = %d [%d..%d]\n", 
                  name, initial_value, min_val, max_val);
    
    return true;
}

int32_t ECSIntegration::getParameter(const char* name) {
    for (uint8_t i = 0; i < param_count; i++) {
        if (strcmp(parameters[i].name, name) == 0) {
            return parameters[i].value;
        }
    }
    
    reportError(ERROR_WARNING, 9003, "Parameter not found", __FUNCTION__);
    return 0;
}

bool ECSIntegration::setParameter(const char* name, int32_t value) {
    for (uint8_t i = 0; i < param_count; i++) {
        if (strcmp(parameters[i].name, name) == 0) {
            EvolvableParameter& param = parameters[i];
            
            // Validate bounds
            if (value < param.min_value || value > param.max_value) {
                reportError(ERROR_WARNING, 9004, "Parameter out of bounds", __FUNCTION__);
                return false;
            }
            
            param.value = value;
            param.mutation_count++;
            param.last_updated = millis();
            
            Serial.printf("🔧 Parameter updated: %s = %d\n", name, value);
            return true;
        }
    }
    
    reportError(ERROR_WARNING, 9005, "Cannot set unknown parameter", __FUNCTION__);
    return false;
}

void ECSIntegration::resetParametersToDefault() {
    for (uint8_t i = 0; i < param_count; i++) {
        parameters[i].value = parameters[i].default_value;
        parameters[i].last_updated = millis();
    }
    
    Serial.println("🔄 Parameters reset to default values");
    saveParametersToEEPROM();
}

void ECSIntegration::reportMetric(MetricType type, float value) {
    if (type >= 8) return;  // Invalid metric type
    
    PerformanceMetric& metric = metrics[type];
    
    // Update running average
    if (metric.sample_count == 0) {
        metric.running_average = value;
    } else {
        // Exponential moving average with alpha = 0.1
        metric.running_average = 0.9 * metric.running_average + 0.1 * value;
    }
    
    metric.value = value;
    metric.timestamp = millis();
    metric.sample_count++;
    
    performance_sample_count++;
    
    // Recalculate fitness after new metric
    calculateFitnessScore();
}

float ECSIntegration::getMetricAverage(MetricType type) {
    if (type >= 8) return 0.0;
    return metrics[type].running_average;
}

void ECSIntegration::calculateFitnessScore() {
    // Weight different metrics based on importance
    const float weights[] = {
        0.25,  // METRIC_TASK_COMPLETION
        0.20,  // METRIC_ENERGY_EFFICIENCY  
        0.15,  // METRIC_COMMUNICATION_SUCCESS
        0.15,  // METRIC_COLLISION_AVOIDANCE
        0.10,  // METRIC_COOPERATION_SCORE
        0.05,  // METRIC_ADAPTATION_SPEED
        0.05,  // METRIC_SENSOR_ACCURACY
        0.05   // METRIC_MOVEMENT_EFFICIENCY
    };
    
    float new_fitness = 0.0;
    float total_weight = 0.0;
    
    for (int i = 0; i < 8; i++) {
        if (metrics[i].sample_count > 0) {
            new_fitness += metrics[i].running_average * weights[i];
            total_weight += weights[i];
        }
    }
    
    if (total_weight > 0) {
        current_fitness = new_fitness / total_weight;
    }
}

void ECSIntegration::reportError(ErrorSeverity severity, uint16_t error_code, 
                               const char* description, const char* function_name) {
    // Add to error log (circular buffer)
    ErrorLogEntry& entry = error_log[error_log_index];
    entry.severity = severity;
    entry.timestamp = millis();
    entry.error_code = error_code;
    
    strncpy(entry.description, description, sizeof(entry.description) - 1);
    entry.description[sizeof(entry.description) - 1] = '\0';
    
    if (function_name) {
        strncpy(entry.function_name, function_name, sizeof(entry.function_name) - 1);
        entry.function_name[sizeof(entry.function_name) - 1] = '\0';
    } else {
        entry.function_name[0] = '\0';
    }
    
    error_log_index = (error_log_index + 1) % ECS_ERROR_LOG_SIZE;
    error_count++;
    
    // Print to serial
    const char* severity_str[] = {"INFO", "WARN", "CRIT", "FATAL"};
    Serial.printf("🚨 [%s] %s: %s (code: %d)\n", 
                  severity_str[severity], 
                  function_name ? function_name : "Unknown",
                  description, error_code);
    
    // Trigger evolution on critical errors
    if (severity >= ERROR_CRITICAL) {
        requestEvolution("Critical error detected");
    }
}

void ECSIntegration::reportException(const char* exception_type, const char* function_name) {
    char desc[32];
    snprintf(desc, sizeof(desc), "%s exception", exception_type ? exception_type : "Unknown");
    reportError(ERROR_CRITICAL, 9999, desc, function_name);
}

uint8_t ECSIntegration::getErrorCount(ErrorSeverity min_severity) {
    uint8_t count = 0;
    uint8_t log_size = (error_count < ECS_ERROR_LOG_SIZE) ? error_count : ECS_ERROR_LOG_SIZE;
    
    for (uint8_t i = 0; i < log_size; i++) {
        if (error_log[i].severity >= min_severity) {
            count++;
        }
    }
    
    return count;
}

void ECSIntegration::requestEvolution(const char* trigger_reason) {
    if (!ecs_connected) {
        Serial.println("⚠️  Cannot request evolution: ECS not connected");
        return;
    }
    
    DynamicJsonDocument doc(256);
    doc["type"] = "evolution_request";
    doc["generation"] = generation;
    doc["fitness"] = current_fitness;
    doc["mutation_attempts"] = mutation_attempts;
    doc["error_count"] = getErrorCount(ERROR_WARNING);
    
    if (trigger_reason) {
        doc["trigger"] = trigger_reason;
    }
    
    char buffer[256];
    serializeJson(doc, buffer, sizeof(buffer));
    
    sendESPNowMessage(ECS_FITNESS_REQUEST, buffer);
    
    Serial.printf("🧬 Evolution requested: %s\n", 
                  trigger_reason ? trigger_reason : "Manual trigger");
}

void ECSIntegration::applyMutation(const char* parameter_name, int32_t new_value) {
    if (setParameter(parameter_name, new_value)) {
        mutation_attempts++;
        Serial.printf("🔬 Mutation applied: %s → %d\n", parameter_name, new_value);
    }
}

void ECSIntegration::reportEvolutionResult(bool success, float fitness_delta) {
    if (success) {
        successful_mutations++;
    }
    
    DynamicJsonDocument doc(128);
    doc["type"] = "evolution_result";
    doc["success"] = success;
    doc["fitness_delta"] = fitness_delta;
    doc["generation"] = generation;
    
    char buffer[128];
    serializeJson(doc, buffer, sizeof(buffer));
    
    sendESPNowMessage(ECS_PERFORMANCE_REPORT, buffer);
    
    if (success) {
        generation++;
        Serial.printf("✅ Evolution successful! Generation %d, fitness Δ: %.3f\n", 
                      generation, fitness_delta);
    } else {
        Serial.printf("❌ Evolution failed, fitness Δ: %.3f\n", fitness_delta);
    }
}

void ECSIntegration::sendPerformanceReport() {
    if (!ecs_connected) return;
    
    DynamicJsonDocument doc(512);
    doc["type"] = "performance_report";
    doc["bot_id"] = WiFi.macAddress();
    doc["generation"] = generation;
    doc["fitness"] = current_fitness;
    doc["sample_count"] = performance_sample_count;
    
    JsonArray metrics_array = doc.createNestedArray("metrics");
    for (int i = 0; i < 8; i++) {
        JsonObject metric = metrics_array.createNestedObject();
        metric["type"] = i;
        metric["value"] = metrics[i].running_average;
        metric["samples"] = metrics[i].sample_count;
    }
    
    JsonArray params_array = doc.createNestedArray("parameters");
    for (uint8_t i = 0; i < param_count; i++) {
        JsonObject param = params_array.createNestedObject();
        param["name"] = parameters[i].name;
        param["value"] = parameters[i].value;
        param["mutations"] = parameters[i].mutation_count;
    }
    
    char buffer[512];
    serializeJson(doc, buffer, sizeof(buffer));
    
    sendESPNowMessage(ECS_PERFORMANCE_REPORT, buffer);
}

void ECSIntegration::sendErrorReport() {
    if (!ecs_connected) return;
    
    DynamicJsonDocument doc(256);
    doc["type"] = "error_report";
    doc["bot_id"] = WiFi.macAddress();
    doc["error_count"] = (error_count < ECS_ERROR_LOG_SIZE) ? error_count : ECS_ERROR_LOG_SIZE;
    
    JsonArray errors_array = doc.createNestedArray("errors");
    uint8_t log_size = (error_count < ECS_ERROR_LOG_SIZE) ? error_count : ECS_ERROR_LOG_SIZE;
    
    for (uint8_t i = 0; i < log_size; i++) {
        const ErrorLogEntry& entry = error_log[i];
        JsonObject error = errors_array.createNestedObject();
        error["severity"] = entry.severity;
        error["code"] = entry.error_code;
        error["description"] = entry.description;
        error["function"] = entry.function_name;
        error["timestamp"] = entry.timestamp;
    }
    
    char buffer[256];
    serializeJson(doc, buffer, sizeof(buffer));
    
    sendESPNowMessage(ECS_ERROR_REPORT, buffer);
}

void ECSIntegration::sendHeartbeat() {
    if (!ecs_connected) return;
    
    DynamicJsonDocument doc(128);
    doc["type"] = "heartbeat";
    doc["bot_id"] = WiFi.macAddress();
    doc["uptime"] = millis();
    doc["generation"] = generation;
    doc["fitness"] = current_fitness;
    doc["free_heap"] = ESP.getFreeHeap();
    
    char buffer[128];
    serializeJson(doc, buffer, sizeof(buffer));
    
    sendESPNowMessage(ECS_STATUS_QUERY, buffer);
    
    last_heartbeat = millis();
}

void ECSIntegration::handleESPNowMessage(const uint8_t* mac, const uint8_t* data, int len) {
    if (len > ECS_MESSAGE_BUFFER_SIZE) return;
    
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, (const char*)data, len);
    
    if (error) {
        reportError(ERROR_WARNING, 8001, "JSON parse error", __FUNCTION__);
        return;
    }
    
    const char* msg_type = doc["type"];
    if (!msg_type) return;
    
    if (strcmp(msg_type, "parameter_update") == 0) {
        const char* param_name = doc["parameter"];
        int32_t new_value = doc["value"];
        
        if (param_name) {
            applyMutation(param_name, new_value);
        }
    }
    else if (strcmp(msg_type, "evolution_trigger") == 0) {
        const char* reason = doc["reason"];
        requestEvolution(reason);
    }
    else if (strcmp(msg_type, "reset_parameters") == 0) {
        resetParametersToDefault();
    }
    else if (strcmp(msg_type, "status_request") == 0) {
        sendPerformanceReport();
        sendErrorReport();
    }
}

void ECSIntegration::printSystemStatus() {
    Serial.println("🧬 ECS System Status:");
    Serial.printf("Generation: %d\n", generation);
    Serial.printf("Current Fitness: %.3f\n", current_fitness);
    Serial.printf("Mutations: %d/%d (%.1f%% success)\n", 
                  successful_mutations, mutation_attempts, getSuccessRate() * 100);
    Serial.printf("Parameters: %d registered\n", param_count);
    Serial.printf("Errors: %d total\n", getErrorCount());
    Serial.printf("Performance Samples: %d\n", performance_sample_count);
    Serial.printf("ECS Connected: %s\n", ecs_connected ? "Yes" : "No");
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
}

void ECSIntegration::getStatusJSON(char* buffer, size_t buffer_size) {
    DynamicJsonDocument doc(256);
    doc["generation"] = generation;
    doc["fitness"] = current_fitness;
    doc["mutations"] = mutation_attempts;
    doc["successful_mutations"] = successful_mutations;
    doc["success_rate"] = getSuccessRate();
    doc["parameter_count"] = param_count;
    doc["error_count"] = getErrorCount();
    doc["performance_samples"] = performance_sample_count;
    doc["connected"] = ecs_connected;
    doc["free_heap"] = ESP.getFreeHeap();
    doc["uptime"] = millis();
    
    serializeJson(doc, buffer, buffer_size);
}

float ECSIntegration::getSuccessRate() const {
    return mutation_attempts > 0 ? (float)successful_mutations / mutation_attempts : 0.0;
}

void ECSIntegration::update() {
    static unsigned long last_heartbeat_time = 0;
    static unsigned long last_auto_save = 0;
    
    unsigned long now = millis();
    
    // Send periodic heartbeat (every 30 seconds)
    if (ecs_connected && (now - last_heartbeat_time > 30000)) {
        sendHeartbeat();
        last_heartbeat_time = now;
    }
    
    // Auto-save parameters (every 5 minutes)
    if (now - last_auto_save > 300000) {
        saveParametersToEEPROM();
        last_auto_save = now;
    }
    
    // Update running averages
    updateRunningAverages();
    
    // Prune old error log entries
    pruneErrorLog();
}

void ECSIntegration::loadParametersFromEEPROM() {
    uint16_t magic = EEPROM.readUShort(ECS_EEPROM_MAGIC_ADDR);
    
    if (magic != ECS_EEPROM_MAGIC) {
        Serial.println("🔧 No valid ECS data in EEPROM, using defaults");
        return;
    }
    
    uint8_t version = EEPROM.readUChar(ECS_EEPROM_VERSION_ADDR);
    uint8_t saved_param_count = EEPROM.readUChar(ECS_EEPROM_PARAM_COUNT_ADDR);
    
    Serial.printf("📖 Loading ECS data: version %d, %d parameters\n", version, saved_param_count);
    
    int addr = ECS_EEPROM_PARAMS_START;
    
    for (uint8_t i = 0; i < saved_param_count && i < param_count && addr < ECS_EEPROM_SIZE - sizeof(EvolvableParameter); i++) {
        EvolvableParameter saved_param;
        EEPROM.readBytes(addr, (uint8_t*)&saved_param, sizeof(EvolvableParameter));
        addr += sizeof(EvolvableParameter);
        
        // Find matching parameter by name
        for (uint8_t j = 0; j < param_count; j++) {
            if (strcmp(parameters[j].name, saved_param.name) == 0) {
                // Validate bounds
                if (saved_param.value >= parameters[j].min_value && 
                    saved_param.value <= parameters[j].max_value) {
                    parameters[j].value = saved_param.value;
                    parameters[j].mutation_count = saved_param.mutation_count;
                    parameters[j].fitness_impact = saved_param.fitness_impact;
                    
                    Serial.printf("✅ Loaded parameter: %s = %d\n", 
                                  saved_param.name, saved_param.value);
                } else {
                    Serial.printf("⚠️  Parameter %s out of bounds, using default\n", 
                                  saved_param.name);
                }
                break;
            }
        }
    }
}

void ECSIntegration::saveParametersToEEPROM() {
    EEPROM.writeUShort(ECS_EEPROM_MAGIC_ADDR, ECS_EEPROM_MAGIC);
    EEPROM.writeUChar(ECS_EEPROM_VERSION_ADDR, 2);  // Version 2.0
    EEPROM.writeUChar(ECS_EEPROM_PARAM_COUNT_ADDR, param_count);
    
    int addr = ECS_EEPROM_PARAMS_START;
    
    for (uint8_t i = 0; i < param_count && addr < ECS_EEPROM_SIZE - sizeof(EvolvableParameter); i++) {
        EEPROM.writeBytes(addr, (const uint8_t*)&parameters[i], sizeof(EvolvableParameter));
        addr += sizeof(EvolvableParameter);
    }
    
    EEPROM.commit();
    Serial.printf("💾 Saved %d parameters to EEPROM\n", param_count);
}

void ECSIntegration::updateRunningAverages() {
    // Decay old metrics that haven't been updated recently
    unsigned long now = millis();
    
    for (int i = 0; i < 8; i++) {
        if (metrics[i].sample_count > 0 && (now - metrics[i].timestamp > 60000)) {
            // Decay by 1% per minute of inactivity
            metrics[i].running_average *= 0.99;
        }
    }
}

void ECSIntegration::pruneErrorLog() {
    // This function could implement more sophisticated log pruning
    // For now, the circular buffer handles it automatically
}

bool ECSIntegration::validateParameter(const char* name, int32_t value) {
    for (uint8_t i = 0; i < param_count; i++) {
        if (strcmp(parameters[i].name, name) == 0) {
            return (value >= parameters[i].min_value && value <= parameters[i].max_value);
        }
    }
    return false;
}

void ECSIntegration::sendESPNowMessage(ECSMessageType type, const char* payload) {
    if (!ecs_connected) return;
    
    // Create message with header
    struct {
        uint16_t sequence;
        uint8_t type;
        uint8_t payload_size;
        char payload[ECS_MESSAGE_BUFFER_SIZE - 4];
    } message;
    
    message.sequence = message_sequence++;
    message.type = (uint8_t)type;
    
    size_t payload_len = strlen(payload);
    message.payload_size = min(payload_len, sizeof(message.payload) - 1);
    
    strncpy(message.payload, payload, message.payload_size);
    message.payload[message.payload_size] = '\0';
    
    size_t total_size = 4 + message.payload_size;
    
    esp_err_t result = esp_now_send(ecs_coordinator_mac, (uint8_t*)&message, total_size);
    
    if (result != ESP_OK) {
        reportError(ERROR_WARNING, 7001, "ESP-NOW send failed", __FUNCTION__);
    }
}