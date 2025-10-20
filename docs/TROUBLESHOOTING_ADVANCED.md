# 🔧 Project Jumbo: Advanced Troubleshooting Guide

## Overview

This guide addresses complex debugging scenarios in the Project Jumbo ecosystem, focusing on sophisticated issues that require deep system knowledge and advanced diagnostic techniques.

---

## 🌐 Trust Network Debugging

### Understanding Trust Networks

Trust networks in Project Jumbo emerge through repeated successful interactions between bots, creating reliability scores that influence coordination decisions.

#### Trust Network Architecture

```cpp
typedef struct {
    char peer_bot_id[16];
    float trust_score;          // 0.0 - 1.0
    uint32_t interaction_count;
    uint32_t success_count;
    uint32_t failure_count;
    uint64_t last_interaction;
    float reliability_trend;    // Recent performance slope
} trust_record_t;
```

### 1. Trust Score Stagnation

#### Symptoms

- Trust scores remain static despite ongoing interactions
- Bots avoid coordination with specific peers
- Communication patterns become rigid and non-adaptive

#### Diagnostic Steps

**Check Trust Update Logic:**

```cpp
void diagnose_trust_updates() {
    // Enable trust debugging
    #define TRUST_DEBUG 1
    
    // Log all trust calculations
    for (auto& record : trust_network) {
        Serial.printf("Bot %s: Trust=%.3f, Success=%d/%d, Trend=%.3f\n",
                     record.peer_bot_id,
                     record.trust_score,
                     record.success_count,
                     record.interaction_count,
                     record.reliability_trend);
    }
}
```

**Verify Trust Decay Parameters:**

```cpp
// Check if trust decay is too aggressive or too conservative
float calculate_trust_decay(uint64_t last_interaction) {
    uint64_t time_since = millis() - last_interaction;
    float decay_rate = 0.001f;  // Adjust if stagnant
    
    if (time_since > TRUST_TIMEOUT) {
        return exp(-decay_rate * time_since);
    }
    return 1.0f;
}
```

#### Solutions

**1. Adjust Trust Parameters:**

```cpp
// Fine-tune trust calculation parameters
const float TRUST_LEARNING_RATE = 0.1f;    // Increase for faster adaptation
const float TRUST_DECAY_RATE = 0.001f;     // Decrease for longer memory
const float TRUST_MIN_THRESHOLD = 0.2f;    // Lower for more interaction
const uint32_t TRUST_MIN_SAMPLES = 5;      // Reduce for faster decisions
```

**2. Implement Trust Reset Mechanism:**

```cpp
void reset_low_trust_connections() {
    for (auto& record : trust_network) {
        if (record.trust_score < 0.3f && 
            record.interaction_count > 20) {
            // Give low-trust bots a fresh chance
            record.trust_score = 0.5f;
            record.success_count = record.interaction_count / 2;
            log_trust_reset(record.peer_bot_id);
        }
    }
}
```

**3. Add Trust Exploration Mechanism:**

```cpp
bool should_try_untrusted_bot(float current_trust) {
    // Occasionally try low-trust bots for exploration
    float exploration_probability = 0.1f * (1.0f - current_trust);
    return (random_float() < exploration_probability);
}
```

### 2. Trust Network Fragmentation

#### Symptoms

- Swarm splits into isolated subgroups
- Cross-group communication failure
- Reduced collective intelligence

#### Diagnostic Tools

**Network Connectivity Analysis:**

```cpp
void analyze_trust_connectivity() {
    // Build adjacency matrix
    float trust_matrix[MAX_BOTS][MAX_BOTS];
    
    // Find connected components
    std::vector<std::vector<int>> components = find_trust_components(trust_matrix);
    
    Serial.printf("Trust network has %d components:\n", components.size());
    for (auto& component : components) {
        Serial.print("Component: ");
        for (int bot_id : component) {
            Serial.printf("Bot%d ", bot_id);
        }
        Serial.println();
    }
}
```

**Bridge Bot Detection:**

```cpp
struct bot_centrality_t {
    char bot_id[16];
    float betweenness_centrality;
    float closeness_centrality;
    bool is_bridge_bot;
};

bot_centrality_t calculate_bot_centrality(const char* bot_id) {
    // Calculate network centrality metrics
    // Identify bots critical for network connectivity
}
```

#### Solutions

**1. Implement Trust Bridging:**

```cpp
void establish_trust_bridges() {
    auto components = find_trust_components();
    
    if (components.size() > 1) {
        // Select bridge bots from each component
        for (size_t i = 0; i < components.size(); ++i) {
            for (size_t j = i + 1; j < components.size(); ++j) {
                establish_bridge_connection(components[i], components[j]);
            }
        }
    }
}
```

**2. Periodic Trust Refresh:**

```cpp
void periodic_trust_maintenance() {
    static uint64_t last_refresh = 0;
    
    if (millis() - last_refresh > TRUST_REFRESH_INTERVAL) {
        // Gradually increase trust for isolated bots
        encourage_cross_group_interaction();
        
        // Reduce trust barriers temporarily
        apply_trust_boost_for_exploration();
        
        last_refresh = millis();
    }
}
```

---

## 🧬 Evolution Stagnation

Evolution stagnation occurs when genetic algorithms converge to local optima or lose diversity, preventing further improvement.

### 1. Premature Convergence Detection

#### Symptoms

- Fitness scores plateau for many generations
- Parameter diversity decreases rapidly
- Population becomes genetically homogeneous

#### Diagnostic Metrics

**Population Diversity Analysis:**

```cpp
struct diversity_metrics_t {
    float parameter_variance[PARAM_COUNT];
    float fitness_variance;
    float genetic_distance_mean;
    float shannon_entropy;
    int effective_population_size;
};

diversity_metrics_t calculate_population_diversity() {
    diversity_metrics_t metrics = {0};
    
    // Calculate parameter variance across population
    for (int param = 0; param < PARAM_COUNT; param++) {
        float mean = calculate_parameter_mean(param);
        float variance = 0;
        
        for (auto& individual : population) {
            float diff = individual.parameters[param] - mean;
            variance += diff * diff;
        }
        metrics.parameter_variance[param] = variance / population.size();
    }
    
    // Calculate fitness variance
    metrics.fitness_variance = calculate_fitness_variance();
    
    // Shannon entropy of parameter distributions
    metrics.shannon_entropy = calculate_shannon_entropy();
    
    return metrics;
}
```

**Convergence Rate Monitoring:**

```cpp
void monitor_convergence_rate() {
    static float fitness_history[CONVERGENCE_WINDOW];
    static int history_index = 0;
    
    fitness_history[history_index] = current_generation_best_fitness;
    history_index = (history_index + 1) % CONVERGENCE_WINDOW;
    
    // Calculate improvement rate
    float improvement_rate = calculate_improvement_slope(fitness_history);
    
    if (improvement_rate < STAGNATION_THRESHOLD) {
        Serial.printf("Warning: Evolution stagnation detected (rate=%.6f)\n", 
                     improvement_rate);
        trigger_diversity_restoration();
    }
}
```

#### Solutions

**1. Adaptive Mutation Rate:**

```cpp
float calculate_adaptive_mutation_rate(diversity_metrics_t diversity) {
    float base_rate = 0.1f;
    float diversity_factor = 1.0f / (diversity.shannon_entropy + 0.1f);
    
    // Increase mutation rate when diversity is low
    float adaptive_rate = base_rate * (1.0f + diversity_factor);
    
    return std::min(adaptive_rate, 0.5f);  // Cap at 50%
}
```

**2. Immigration Strategy:**

```cpp
void introduce_immigrants() {
    const int immigrant_count = population.size() * 0.1f;  // 10% immigrants
    
    for (int i = 0; i < immigrant_count; i++) {
        // Replace worst performers with random individuals
        int worst_index = find_worst_fitness_index();
        
        // Generate completely random individual
        individual_t immigrant;
        randomize_individual(immigrant);
        
        population[worst_index] = immigrant;
        
        Serial.printf("Introduced immigrant at index %d\n", worst_index);
    }
}
```

**3. Multi-Population Evolution:**

```cpp
struct evolution_island_t {
    std::vector<individual_t> population;
    float best_fitness;
    int generations_without_improvement;
};

class IslandEvolution {
    std::vector<evolution_island_t> islands;
    
    void evolve_islands() {
        // Evolve each island independently
        for (auto& island : islands) {
            evolve_single_island(island);
        }
        
        // Periodic migration between islands
        if (generation % MIGRATION_INTERVAL == 0) {
            perform_island_migration();
        }
    }
    
    void perform_island_migration() {
        // Exchange best individuals between islands
        for (size_t i = 0; i < islands.size(); i++) {
            for (size_t j = i + 1; j < islands.size(); j++) {
                exchange_best_individuals(islands[i], islands[j]);
            }
        }
    }
};
```

### 2. Fitness Function Issues

#### Symptoms

- Fitness scores don't reflect actual performance
- Optimization toward undesired behaviors
- Fitness values cluster around specific values

#### Diagnostic Approaches

**Multi-Objective Fitness Analysis:**

```cpp
struct fitness_components_t {
    float exploration_score;
    float obstacle_avoidance;
    float cooperation_score;
    float energy_efficiency;
    float task_completion;
    float communication_effectiveness;
};

void analyze_fitness_components() {
    for (auto& individual : population) {
        fitness_components_t components = decompose_fitness(individual);
        
        Serial.printf("Bot %s: Exp=%.3f, Avoid=%.3f, Coop=%.3f, Eff=%.3f\n",
                     individual.bot_id,
                     components.exploration_score,
                     components.obstacle_avoidance,
                     components.cooperation_score,
                     components.energy_efficiency);
    }
}
```

**Fitness Landscape Exploration:**

```cpp
void map_fitness_landscape() {
    // Sample fitness at regular parameter intervals
    for (float motor_speed = 100; motor_speed <= 300; motor_speed += 10) {
        for (float threshold = 50; threshold <= 200; threshold += 10) {
            individual_t test_individual;
            test_individual.motor_speed = motor_speed;
            test_individual.sensor_threshold = threshold;
            
            float fitness = evaluate_fitness_simulation(test_individual);
            log_fitness_sample(motor_speed, threshold, fitness);
        }
    }
}
```

#### Solutions

**1. Dynamic Fitness Weighting:**

```cpp
float calculate_adaptive_fitness(individual_t& individual, int generation) {
    fitness_components_t components = decompose_fitness(individual);
    
    // Adjust weights based on generation and current needs
    float exploration_weight = 0.3f + 0.2f * sin(generation * 0.1f);
    float cooperation_weight = 0.2f + 0.1f * (generation / 100.0f);
    float efficiency_weight = 0.3f;
    float avoidance_weight = 0.2f;
    
    float weighted_fitness = 
        components.exploration_score * exploration_weight +
        components.cooperation_score * cooperation_weight +
        components.energy_efficiency * efficiency_weight +
        components.obstacle_avoidance * avoidance_weight;
    
    return weighted_fitness;
}
```

**2. Novelty Search Integration:**

```cpp
struct behavior_signature_t {
    float exploration_pattern[SIGNATURE_SIZE];
    float interaction_frequency;
    float movement_entropy;
};

float calculate_novelty_score(individual_t& individual) {
    behavior_signature_t signature = extract_behavior_signature(individual);
    
    float min_distance = FLT_MAX;
    for (auto& archive_signature : behavior_archive) {
        float distance = calculate_behavior_distance(signature, archive_signature);
        min_distance = std::min(min_distance, distance);
    }
    
    return min_distance;  // Higher novelty = more unique behavior
}
```

---

## 📡 ESP-NOW Packet Loss

ESP-NOW packet loss can severely impact swarm coordination and real-time communication.

### 1. Packet Loss Detection

#### Real-time Monitoring

```cpp
struct packet_stats_t {
    uint32_t sent_count;
    uint32_t acked_count;
    uint32_t lost_count;
    uint32_t retransmit_count;
    float loss_rate;
    uint32_t avg_latency_ms;
};

class PacketMonitor {
    std::map<uint8_t, packet_stats_t> peer_stats;
    
    void update_packet_stats(uint8_t peer_id, bool success, uint32_t latency) {
        auto& stats = peer_stats[peer_id];
        stats.sent_count++;
        
        if (success) {
            stats.acked_count++;
            stats.avg_latency_ms = (stats.avg_latency_ms + latency) / 2;
        } else {
            stats.lost_count++;
        }
        
        stats.loss_rate = (float)stats.lost_count / stats.sent_count;
        
        if (stats.loss_rate > PACKET_LOSS_THRESHOLD) {
            handle_high_packet_loss(peer_id);
        }
    }
};
```

#### Channel Quality Assessment

```cpp
void assess_channel_quality() {
    for (int channel = 1; channel <= 13; channel++) {
        wifi_set_channel(channel);
        delay(100);  // Allow channel switch
        
        uint32_t noise_level = measure_background_noise();
        float channel_utilization = scan_channel_utilization(channel);
        
        Serial.printf("Channel %d: Noise=%ddBm, Util=%.1f%%\n",
                     channel, noise_level, channel_utilization * 100);
        
        channel_quality[channel] = calculate_channel_score(noise_level, 
                                                         channel_utilization);
    }
    
    int best_channel = find_best_channel();
    if (best_channel != current_channel) {
        switch_to_channel(best_channel);
    }
}
```

### 2. Adaptive Retransmission

#### Smart Retry Logic

```cpp
class AdaptiveRetransmission {
    struct retry_params_t {
        uint8_t max_retries;
        uint32_t base_timeout_ms;
        float backoff_multiplier;
        uint32_t max_timeout_ms;
    };
    
    std::map<uint8_t, retry_params_t> peer_retry_params;
    
    bool send_with_adaptive_retry(uint8_t peer_id, const uint8_t* data, size_t len) {
        auto& params = peer_retry_params[peer_id];
        
        for (uint8_t attempt = 0; attempt < params.max_retries; attempt++) {
            if (esp_now_send(peer_id, data, len) == ESP_OK) {
                // Wait for ACK with exponential backoff
                uint32_t timeout = params.base_timeout_ms * 
                                 pow(params.backoff_multiplier, attempt);
                timeout = std::min(timeout, params.max_timeout_ms);
                
                if (wait_for_ack(peer_id, timeout)) {
                    update_retry_success(peer_id, attempt);
                    return true;
                }
            }
            
            // Add jitter to prevent synchronization issues
            delay(random(10, 50));
        }
        
        update_retry_failure(peer_id);
        return false;
    }
    
    void update_retry_parameters(uint8_t peer_id, float success_rate) {
        auto& params = peer_retry_params[peer_id];
        
        if (success_rate > 0.9f) {
            // High success rate - reduce retries and timeouts
            params.max_retries = std::max(2, params.max_retries - 1);
            params.base_timeout_ms = std::max(10, params.base_timeout_ms - 5);
        } else if (success_rate < 0.7f) {
            // Low success rate - increase retries and timeouts
            params.max_retries = std::min(8, params.max_retries + 1);
            params.base_timeout_ms = std::min(100, params.base_timeout_ms + 10);
        }
    }
};
```

### 3. Message Prioritization

#### Priority Queue System

```cpp
enum MessagePriority {
    PRIORITY_EMERGENCY = 0,
    PRIORITY_CRITICAL = 1,
    PRIORITY_HIGH = 2,
    PRIORITY_NORMAL = 3,
    PRIORITY_LOW = 4
};

struct prioritized_message_t {
    uint8_t priority;
    uint8_t peer_id;
    std::vector<uint8_t> data;
    uint64_t timestamp;
    uint8_t retry_count;
};

class MessageQueue {
    std::priority_queue<prioritized_message_t> message_queue;
    
    void process_message_queue() {
        while (!message_queue.empty() && can_send_message()) {
            auto message = message_queue.top();
            message_queue.pop();
            
            if (should_drop_message(message)) {
                log_message_dropped(message);
                continue;
            }
            
            if (send_message_with_priority(message)) {
                log_message_sent(message);
            } else {
                // Requeue with increased retry count
                message.retry_count++;
                if (message.retry_count < get_max_retries(message.priority)) {
                    message_queue.push(message);
                }
            }
        }
    }
    
    bool should_drop_message(const prioritized_message_t& message) {
        // Drop old low-priority messages
        uint64_t age = millis() - message.timestamp;
        uint32_t max_age = get_max_age_for_priority(message.priority);
        
        return age > max_age;
    }
};
```

### 4. Network Optimization

#### Automatic Channel Selection

```cpp
class ChannelManager {
    struct channel_metrics_t {
        float packet_loss_rate;
        uint32_t average_latency;
        uint32_t collision_count;
        float noise_floor;
    };
    
    std::array<channel_metrics_t, 14> channel_metrics;
    
    void optimize_channel_usage() {
        int current_best = find_best_performing_channel();
        
        if (should_switch_channel(current_best)) {
            coordinate_channel_switch(current_best);
        }
    }
    
    void coordinate_channel_switch(int new_channel) {
        // Broadcast channel switch notification
        channel_switch_msg_t switch_msg = {
            .new_channel = new_channel,
            .switch_time = millis() + CHANNEL_SWITCH_DELAY,
            .coordinator_id = get_local_device_id()
        };
        
        broadcast_channel_switch(switch_msg);
        
        // Wait for coordination period
        delay(CHANNEL_SWITCH_DELAY);
        
        // Switch to new channel
        wifi_set_channel(new_channel);
        current_channel = new_channel;
        
        Serial.printf("Switched to channel %d\n", new_channel);
    }
};
```

---

## 📝 Vocabulary Pruning Issues

Emergent vocabulary systems can develop inefficient or redundant communication patterns that need intelligent pruning.

### 1. Vocabulary Bloat Detection

#### Signal Usage Analysis

```cpp
struct vocabulary_entry_t {
    uint32_t signal_id;
    char pattern[32];
    uint32_t usage_count;
    uint64_t last_used;
    float context_entropy;
    float effectiveness_score;
    std::vector<uint32_t> associated_contexts;
};

class VocabularyAnalyzer {
    std::vector<vocabulary_entry_t> vocabulary;
    
    void analyze_vocabulary_health() {
        float total_entropy = 0;
        int redundant_signals = 0;
        int underused_signals = 0;
        
        for (auto& entry : vocabulary) {
            // Calculate usage frequency
            float usage_frequency = calculate_usage_frequency(entry);
            
            // Check for redundancy
            if (find_similar_signals(entry).size() > 1) {
                redundant_signals++;
                entry.effectiveness_score *= 0.8f;  // Reduce effectiveness
            }
            
            // Check for underuse
            if (usage_frequency < MIN_USAGE_THRESHOLD) {
                underused_signals++;
                mark_for_potential_pruning(entry);
            }
            
            total_entropy += entry.context_entropy;
        }
        
        vocabulary_health_score = calculate_health_score(total_entropy, 
                                                        redundant_signals,
                                                        underused_signals);
        
        if (vocabulary_health_score < HEALTH_THRESHOLD) {
            trigger_vocabulary_maintenance();
        }
    }
};
```

#### Context Overlap Detection

```cpp
float calculate_context_overlap(const vocabulary_entry_t& entry1, 
                              const vocabulary_entry_t& entry2) {
    std::set<uint32_t> contexts1(entry1.associated_contexts.begin(),
                                 entry1.associated_contexts.end());
    std::set<uint32_t> contexts2(entry2.associated_contexts.begin(), 
                                 entry2.associated_contexts.end());
    
    std::set<uint32_t> intersection;
    std::set_intersection(contexts1.begin(), contexts1.end(),
                         contexts2.begin(), contexts2.end(),
                         std::inserter(intersection, intersection.begin()));
    
    std::set<uint32_t> union_set;
    std::set_union(contexts1.begin(), contexts1.end(),
                  contexts2.begin(), contexts2.end(),
                  std::inserter(union_set, union_set.begin()));
    
    return (float)intersection.size() / union_set.size();  // Jaccard similarity
}
```

### 2. Intelligent Pruning Strategies

#### Entropy-Based Pruning

```cpp
void prune_low_entropy_signals() {
    // Sort signals by context entropy
    std::sort(vocabulary.begin(), vocabulary.end(),
             [](const vocabulary_entry_t& a, const vocabulary_entry_t& b) {
                 return a.context_entropy < b.context_entropy;
             });
    
    int pruned_count = 0;
    auto it = vocabulary.begin();
    
    while (it != vocabulary.end() && pruned_count < MAX_PRUNE_COUNT) {
        if (it->context_entropy < MIN_ENTROPY_THRESHOLD &&
            it->usage_count < MIN_USAGE_COUNT) {
            
            // Notify swarm about signal removal
            broadcast_signal_removal(it->signal_id);
            
            Serial.printf("Pruned signal %d (entropy=%.3f, usage=%d)\n",
                         it->signal_id, it->context_entropy, it->usage_count);
            
            it = vocabulary.erase(it);
            pruned_count++;
        } else {
            ++it;
        }
    }
}
```

#### Redundancy Resolution

```cpp
void resolve_signal_redundancy() {
    std::vector<std::vector<size_t>> redundant_groups = find_redundant_groups();
    
    for (auto& group : redundant_groups) {
        if (group.size() <= 1) continue;
        
        // Find the most effective signal in the group
        size_t best_index = 0;
        float best_score = 0;
        
        for (size_t idx : group) {
            float score = calculate_signal_effectiveness(vocabulary[idx]);
            if (score > best_score) {
                best_score = score;
                best_index = idx;
            }
        }
        
        // Merge contexts and usage from redundant signals
        for (size_t idx : group) {
            if (idx != best_index) {
                merge_signal_contexts(vocabulary[best_index], vocabulary[idx]);
                
                // Schedule for removal
                vocabulary[idx].effectiveness_score = -1.0f;  // Mark for deletion
            }
        }
    }
    
    // Remove marked signals
    vocabulary.erase(
        std::remove_if(vocabulary.begin(), vocabulary.end(),
                      [](const vocabulary_entry_t& entry) {
                          return entry.effectiveness_score < 0;
                      }),
        vocabulary.end());
}
```

### 3. Vocabulary Evolution

#### Adaptive Signal Generation

```cpp
void generate_new_signals_if_needed() {
    float communication_efficiency = calculate_communication_efficiency();
    
    if (communication_efficiency < EFFICIENCY_THRESHOLD) {
        // Analyze communication gaps
        std::vector<uint32_t> uncovered_contexts = find_uncovered_contexts();
        
        for (uint32_t context : uncovered_contexts) {
            if (vocabulary.size() < MAX_VOCABULARY_SIZE) {
                vocabulary_entry_t new_signal = generate_signal_for_context(context);
                
                if (validate_new_signal(new_signal)) {
                    vocabulary.push_back(new_signal);
                    broadcast_new_signal(new_signal);
                    
                    Serial.printf("Generated new signal %d for context %d\n",
                                 new_signal.signal_id, context);
                }
            }
        }
    }
}

vocabulary_entry_t generate_signal_for_context(uint32_t context) {
    vocabulary_entry_t new_signal;
    
    // Generate unique signal ID
    new_signal.signal_id = generate_unique_signal_id();
    
    // Create pattern based on context characteristics
    generate_context_specific_pattern(new_signal.pattern, context);
    
    // Initialize metrics
    new_signal.usage_count = 0;
    new_signal.last_used = millis();
    new_signal.context_entropy = 0.0f;
    new_signal.effectiveness_score = 0.5f;  // Neutral starting score
    new_signal.associated_contexts.push_back(context);
    
    return new_signal;
}
```

#### Signal Effectiveness Tracking

```cpp
void update_signal_effectiveness() {
    for (auto& entry : vocabulary) {
        // Track successful communications using this signal
        float success_rate = calculate_signal_success_rate(entry.signal_id);
        
        // Track response time when using this signal
        float avg_response_time = calculate_avg_response_time(entry.signal_id);
        
        // Update effectiveness score
        float time_factor = 1.0f / (1.0f + avg_response_time / 1000.0f);
        entry.effectiveness_score = 0.7f * entry.effectiveness_score + 
                                  0.3f * (success_rate * time_factor);
        
        // Update context entropy based on usage patterns
        entry.context_entropy = calculate_context_entropy(entry);
        
        // Age-based decay for unused signals
        uint64_t age = millis() - entry.last_used;
        if (age > SIGNAL_MAX_AGE) {
            entry.effectiveness_score *= 0.99f;  // Gradual decay
        }
    }
}
```

---

## 🛠️ Advanced Diagnostic Tools

### 1. System Health Dashboard

```cpp
struct system_health_t {
    float trust_network_connectivity;
    float evolution_diversity_score;
    float communication_efficiency;
    float vocabulary_health_score;
    float overall_system_health;
    
    std::vector<std::string> active_warnings;
    std::vector<std::string> critical_issues;
};

system_health_t generate_system_health_report() {
    system_health_t health;
    
    // Calculate individual metrics
    health.trust_network_connectivity = calculate_network_connectivity();
    health.evolution_diversity_score = calculate_population_diversity().shannon_entropy;
    health.communication_efficiency = calculate_communication_efficiency();
    health.vocabulary_health_score = calculate_vocabulary_health();
    
    // Aggregate overall health
    health.overall_system_health = (health.trust_network_connectivity +
                                   health.evolution_diversity_score +
                                   health.communication_efficiency +
                                   health.vocabulary_health_score) / 4.0f;
    
    // Generate warnings and critical issues
    if (health.trust_network_connectivity < 0.6f) {
        health.active_warnings.push_back("Trust network fragmentation detected");
    }
    
    if (health.evolution_diversity_score < 0.3f) {
        health.critical_issues.push_back("Evolution stagnation - diversity critically low");
    }
    
    return health;
}
```

### 2. Automated Recovery Procedures

```cpp
class AutoRecoverySystem {
    std::map<std::string, std::function<void()>> recovery_procedures;
    
public:
    AutoRecoverySystem() {
        // Register recovery procedures
        recovery_procedures["trust_fragmentation"] = []() {
            establish_trust_bridges();
            apply_trust_boost_for_exploration();
        };
        
        recovery_procedures["evolution_stagnation"] = []() {
            increase_mutation_rate();
            introduce_immigrants();
            reset_fitness_evaluation();
        };
        
        recovery_procedures["packet_loss"] = []() {
            optimize_channel_usage();
            adjust_retry_parameters();
            reduce_message_frequency();
        };
        
        recovery_procedures["vocabulary_bloat"] = []() {
            prune_low_entropy_signals();
            resolve_signal_redundancy();
            optimize_signal_usage();
        };
    }
    
    void monitor_and_recover() {
        system_health_t health = generate_system_health_report();
        
        for (const auto& issue : health.critical_issues) {
            auto procedure = find_recovery_procedure(issue);
            if (procedure != recovery_procedures.end()) {
                Serial.printf("Executing recovery procedure for: %s\n", issue.c_str());
                procedure->second();
            }
        }
    }
};
```

---

## 📊 Performance Monitoring

### Real-time Metrics Collection

```cpp
class MetricsCollector {
    struct metrics_snapshot_t {
        uint64_t timestamp;
        float trust_scores[MAX_BOTS];
        float fitness_scores[MAX_BOTS];
        float packet_loss_rates[MAX_BOTS];
        int vocabulary_size;
        float communication_efficiency;
    };
    
    std::deque<metrics_snapshot_t> metrics_history;
    
public:
    void collect_metrics() {
        metrics_snapshot_t snapshot;
        snapshot.timestamp = millis();
        
        // Collect trust scores
        for (int i = 0; i < active_bot_count; i++) {
            snapshot.trust_scores[i] = get_bot_average_trust(i);
        }
        
        // Collect fitness scores
        for (int i = 0; i < active_bot_count; i++) {
            snapshot.fitness_scores[i] = get_bot_fitness(i);
        }
        
        // Collect communication metrics
        for (int i = 0; i < active_bot_count; i++) {
            snapshot.packet_loss_rates[i] = get_packet_loss_rate(i);
        }
        
        snapshot.vocabulary_size = vocabulary.size();
        snapshot.communication_efficiency = calculate_communication_efficiency();
        
        metrics_history.push_back(snapshot);
        
        // Limit history size
        if (metrics_history.size() > MAX_HISTORY_SIZE) {
            metrics_history.pop_front();
        }
    }
    
    void export_metrics_csv(const char* filename) {
        FILE* file = fopen(filename, "w");
        
        fprintf(file, "timestamp,bot_id,trust_score,fitness_score,packet_loss,vocab_size,comm_eff\n");
        
        for (const auto& snapshot : metrics_history) {
            for (int bot = 0; bot < active_bot_count; bot++) {
                fprintf(file, "%llu,%d,%.3f,%.3f,%.3f,%d,%.3f\n",
                       snapshot.timestamp, bot,
                       snapshot.trust_scores[bot],
                       snapshot.fitness_scores[bot], 
                       snapshot.packet_loss_rates[bot],
                       snapshot.vocabulary_size,
                       snapshot.communication_efficiency);
            }
        }
        
        fclose(file);
    }
};
```

---

**This advanced troubleshooting guide provides comprehensive solutions for the most complex issues in autonomous swarm systems. Use these diagnostic tools and recovery procedures to maintain optimal system performance. 🔧🤖✨**
