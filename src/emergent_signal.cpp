/*
 * 🌊 Emergent Signal Generator Implementation
 * ===========================================
 * Core implementation of truly emergent bot communication
 * Signals evolve from environmental interaction and peer feedback
 * 
 * Key Innovations:
 * - Context-driven signal creation (not predefined messages)
 * - Evolutionary signal refinement based on utility
 * - Peer learning without shared vocabulary
 * - Acoustic personality development
 * - Dynamic vocabulary growth
 * 
 * Author: Project Jumbo Team
 * Version: 2.0.0 - Emergent Language System
 */

#include "emergent_signal.h"
#include <random>

// ═══════════════════════════════════════════════════════════
// 🏗️ CONSTRUCTOR & INITIALIZATION
// ═══════════════════════════════════════════════════════════

EmergentSignalGenerator::EmergentSignalGenerator() {
  vocabularySize = 0;
  memorySize = 0;
  peerCount = 0;
  currentGeneration = 0;
  
  // Generate unique personality traits
  personalitySignature = random(1, 255);
  complexityPreference = random(1, 8); // Preference for 1-8 component signals
  innovationRate = random(10, 90);     // 10-90% chance to create new vs reuse
  
  // Initialize arrays
  memset(vocabulary, 0, sizeof(vocabulary));
  memset(contextMemory, 0, sizeof(contextMemory));
  memset(peerProfiles, 0, sizeof(peerProfiles));
  
  Serial.printf("🧬 Emergent Signal Generator initialized\n");
  Serial.printf("   Personality: 0x%02X, Complexity: %d, Innovation: %d%%\n", 
                personalitySignature, complexityPreference, innovationRate);
}

// ═══════════════════════════════════════════════════════════
// 🎵 CORE SIGNAL GENERATION
// ═══════════════════════════════════════════════════════════

SignalWord* EmergentSignalGenerator::generateSignalForContext(EnvironmentalContext context, EmotionalState emotion) {
  Serial.printf("🎵 Generating signal for context: %s, emotion: %s\n", 
                contextToString(context).c_str(), emotionToString(emotion).c_str());
  
  // Strategy 1: Look for existing signal that fits this context+emotion
  SignalWord* existingSignal = findExistingSignal(context, emotion);
  
  if (existingSignal != nullptr) {
    // Found existing signal - decide whether to reuse or innovate
    uint8_t innovationRoll = random(0, 100);
    
    if (innovationRoll > innovationRate) {
      // Reuse existing signal
      Serial.printf("♻️ Reusing existing signal (utility: %.2f)\n", existingSignal->utility);
      existingSignal->timesUsed++;
      existingSignal->lastUsed = millis();
      return existingSignal;
    }
    
    // Innovation: mutate existing signal slightly
    Serial.println("🔄 Mutating existing signal for variation");
    SignalWord mutatedSignal = *existingSignal;
    mutateSignal(&mutatedSignal);
    
    // Add mutated signal to vocabulary if space available
    if (vocabularySize < MAX_SIGNAL_VOCABULARY) {
      vocabulary[vocabularySize] = mutatedSignal;
      vocabulary[vocabularySize].createdAt = millis();
      vocabulary[vocabularySize].generation = currentGeneration;
      vocabulary[vocabularySize].timesUsed = 1;
      vocabulary[vocabularySize].utility = 0.5f; // Start with neutral utility
      vocabularySize++;
      return &vocabulary[vocabularySize - 1];
    }
  }
  
  // Strategy 2: Create completely new signal
  Serial.println("✨ Creating brand new signal");
  SignalWord newSignal = createNewSignal(context, emotion);
  
  // Add to vocabulary if space available
  if (vocabularySize < MAX_SIGNAL_VOCABULARY) {
    vocabulary[vocabularySize] = newSignal;
    vocabularySize++;
    return &vocabulary[vocabularySize - 1];
  } else {
    // Vocabulary full - replace least useful signal
    Serial.println("📚 Vocabulary full, replacing least useful signal");
    
    float lowestUtility = 1.0f;
    uint8_t replaceIndex = 0;
    
    for (uint8_t i = 0; i < vocabularySize; i++) {
      if (vocabulary[i].utility < lowestUtility) {
        lowestUtility = vocabulary[i].utility;
        replaceIndex = i;
      }
    }
    
    vocabulary[replaceIndex] = newSignal;
    return &vocabulary[replaceIndex];
  }
}

SignalWord* EmergentSignalGenerator::findExistingSignal(EnvironmentalContext context, EmotionalState emotion) {
  SignalWord* bestMatch = nullptr;
  float bestScore = 0.0f;
  
  for (uint8_t i = 0; i < vocabularySize; i++) {
    SignalWord* signal = &vocabulary[i];
    
    // Calculate contextual similarity
    float contextMatch = (signal->contextType == context) ? 1.0f : 0.0f;
    float emotionMatch = (abs(signal->emotionalValence - emotion) <= 1) ? 1.0f : 0.0f;
    
    // Weight by utility - prefer successful signals
    float totalScore = (contextMatch * 0.6f + emotionMatch * 0.4f) * signal->utility;
    
    if (totalScore > bestScore) {
      bestScore = totalScore;
      bestMatch = signal;
    }
  }
  
  // Only return if match is reasonably good
  return (bestScore > 0.3f) ? bestMatch : nullptr;
}

SignalWord EmergentSignalGenerator::createNewSignal(EnvironmentalContext context, EmotionalState emotion) {
  SignalWord newSignal;
  memset(&newSignal, 0, sizeof(SignalWord));
  
  // Set semantic properties
  newSignal.contextType = context;
  newSignal.emotionalValence = emotion;
  newSignal.generation = currentGeneration;
  
  // Generate acoustic properties based on context and emotion
  // Context influences number of components
  switch (context) {
    case CONTEXT_DANGER_SENSED:
    case CONTEXT_TASK_FAILURE:
      newSignal.componentCount = random(3, 6); // Urgent contexts = more complex
      break;
    case CONTEXT_TASK_SUCCESS:
    case CONTEXT_RESOURCE_FOUND:
      newSignal.componentCount = random(2, 4); // Success = moderate complexity
      break;
    default:
      newSignal.componentCount = random(1, complexityPreference + 1);
      break;
  }
  
  // Emotion influences component types and intensity
  for (uint8_t i = 0; i < newSignal.componentCount; i++) {
    // Choose component type based on emotion
    if (emotion >= EMOTION_POSITIVE) {
      // Positive emotions prefer higher frequencies and rising sweeps
      uint8_t positiveComponents[] = {COMPONENT_TONE_HIGH, COMPONENT_SWEEP_UP, COMPONENT_PULSE_FAST};
      newSignal.components[i] = positiveComponents[random(0, 3)];
      newSignal.intensities[i] = random(150, 255); // Higher intensity
    } else if (emotion <= EMOTION_NEGATIVE) {
      // Negative emotions prefer lower frequencies and falling sweeps
      uint8_t negativeComponents[] = {COMPONENT_TONE_LOW, COMPONENT_SWEEP_DOWN, COMPONENT_PULSE_SLOW};
      newSignal.components[i] = negativeComponents[random(0, 3)];
      newSignal.intensities[i] = random(100, 200); // Moderate intensity
    } else {
      // Neutral emotion - any component
      newSignal.components[i] = generateRandomComponent();
      newSignal.intensities[i] = generateRandomIntensity();
    }
    
    // Duration influenced by context urgency
    if (context == CONTEXT_DANGER_SENSED || context == CONTEXT_TASK_FAILURE) {
      newSignal.durations[i] = random(50, 200); // Short, urgent
    } else if (context == CONTEXT_WAITING || context == CONTEXT_EXPLORATION) {
      newSignal.durations[i] = random(200, 800); // Longer, relaxed
    } else {
      newSignal.durations[i] = random(100, 400); // Normal duration
    }
  }
  
  // Set evolutionary properties
  newSignal.utility = 0.5f; // Start with neutral utility
  newSignal.timesUsed = 1;
  newSignal.timesUnderstood = 0;
  newSignal.lastUsed = millis();
  newSignal.createdAt = millis();
  
  // Add personality signature
  newSignal.personalitySignature = personalitySignature;
  newSignal.complexityPreference = complexityPreference;
  
  Serial.printf("✨ Created new signal: %d components, context=%d, emotion=%d\n", 
                newSignal.componentCount, context, emotion);
  
  return newSignal;
}

// ═══════════════════════════════════════════════════════════
// 🧬 SIGNAL EVOLUTION & LEARNING
// ═══════════════════════════════════════════════════════════

void EmergentSignalGenerator::updateSignalUtility(SignalWord* signal, float outcome) {
  if (signal == nullptr) return;
  
  // Update utility using exponential moving average
  float alpha = 0.1f; // Learning rate
  signal->utility = (1.0f - alpha) * signal->utility + alpha * outcome;
  
  // Track understanding
  if (outcome > 0.5f) {
    signal->timesUnderstood++;
  }
  
  Serial.printf("📈 Updated signal utility: %.3f (outcome: %.3f)\n", signal->utility, outcome);
  
  // Evolution pressure: signals with very low utility get mutated
  if (signal->utility < 0.2f && signal->timesUsed > 5) {
    Serial.println("🔄 Low utility signal - applying evolutionary pressure");
    mutateSignal(signal);
    signal->generation = currentGeneration; // Mark as evolved
  }
}

void EmergentSignalGenerator::mutateSignal(SignalWord* signal) {
  uint8_t mutationType = random(0, 4);
  
  switch (mutationType) {
    case 0: // Mutate duration
      if (signal->componentCount > 0) {
        uint8_t componentIndex = random(0, signal->componentCount);
        signal->durations[componentIndex] += random(-50, 51);
        signal->durations[componentIndex] = constrain(signal->durations[componentIndex], 50, 1000);
      }
      break;
      
    case 1: // Mutate intensity
      if (signal->componentCount > 0) {
        uint8_t componentIndex = random(0, signal->componentCount);
        signal->intensities[componentIndex] += random(-30, 31);
        signal->intensities[componentIndex] = constrain(signal->intensities[componentIndex], 50, 255);
      }
      break;
      
    case 2: // Change component type
      if (signal->componentCount > 0) {
        uint8_t componentIndex = random(0, signal->componentCount);
        signal->components[componentIndex] = generateRandomComponent();
      }
      break;
      
    case 3: // Add or remove component
      if (signal->componentCount < MAX_SIGNAL_COMPONENTS && random(0, 2)) {
        // Add component
        signal->components[signal->componentCount] = generateRandomComponent();
        signal->durations[signal->componentCount] = generateRandomDuration();
        signal->intensities[signal->componentCount] = generateRandomIntensity();
        signal->componentCount++;
      } else if (signal->componentCount > 1) {
        // Remove component
        signal->componentCount--;
      }
      break;
  }
  
  Serial.println("🧬 Signal mutated");
}

// ═══════════════════════════════════════════════════════════
// 🤝 PEER LEARNING & COMMUNICATION
// ═══════════════════════════════════════════════════════════

void EmergentSignalGenerator::learnFromPeerSignal(uint8_t* peerMac, SignalWord* signal, EnvironmentalContext context) {
  Serial.printf("🧠 Learning from peer signal in context: %s\n", contextToString(context).c_str());
  
  // Find or create peer profile
  PeerSignalProfile* profile = nullptr;
  for (uint8_t i = 0; i < peerCount; i++) {
    if (macEquals(peerProfiles[i].peerMac, peerMac)) {
      profile = &peerProfiles[i];
      break;
    }
  }
  
  if (profile == nullptr && peerCount < 8) {
    // Create new peer profile
    profile = &peerProfiles[peerCount];
    memcpy(profile->peerMac, peerMac, 6);
    profile->signalCount = 0;
    profile->trustLevel = 0.5f; // Start with neutral trust
    profile->personalitySignature = signal->personalitySignature;
    profile->lastInteraction = millis();
    peerCount++;
  }
  
  if (profile != nullptr) {
    // Store this signal if we have space
    if (profile->signalCount < 16) {
      profile->knownSignals[profile->signalCount] = *signal;
      profile->signalCount++;
    }
    
    // Update last interaction
    profile->lastInteraction = millis();
    
    // Store in context memory
    if (memorySize < MAX_CONTEXT_MEMORY) {
      SignalMemory& memory = contextMemory[memorySize];
      memcpy(memory.senderMac, peerMac, 6);
      memory.receivedSignal = *signal;
      memory.contextWhenReceived = context;
      memory.timestamp = millis();
      memory.outcomeScore = 0.0f; // Will be updated based on response outcome
      memorySize++;
    }
    
    Serial.printf("📚 Learned signal from peer %s (total signals: %d)\n", 
                  macToString(peerMac).c_str(), profile->signalCount);
  }
}

bool EmergentSignalGenerator::sendEmergentMessage(SignalWord* signal, EnvironmentalContext context, EmotionalState emotion) {
  if (signal == nullptr) return false;
  
  EmergentMessage message;
  memset(&message, 0, sizeof(EmergentMessage));
  
  // Fill message structure
  message.protocolVersion = 0x02;
  WiFi.macAddress(message.senderMac);
  message.timestamp = millis();
  message.sequenceNumber = random(0, 255);
  message.signal = *signal;
  message.currentContext = context;
  message.currentEmotion = emotion;
  message.confidence = min(255, (int)(signal->utility * 255));
  message.expectsResponse = (context == CONTEXT_DANGER_SENSED || context == CONTEXT_TASK_FAILURE) ? 1 : 0;
  message.isResponse = 0;
  message.signalAge = min(255, (int)((millis() - signal->createdAt) / 1000)); // Age in seconds - cast to int
  
  // Calculate checksum
  message.checksum = 0;
  uint8_t* data = (uint8_t*)&message;
  for (uint16_t i = 0; i < sizeof(EmergentMessage) - 1; i++) {
    message.checksum ^= data[i];
  }
  
  // Send via ESP-NOW broadcast
  uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_err_t result = esp_now_send(broadcastAddr, (uint8_t*)&message, sizeof(EmergentMessage));
  
  if (result == ESP_OK) {
    Serial.printf("📡 Emergent signal broadcast successful\n");
    
    // Play the signal locally (optional - for debugging/demonstration)
    playSignalWord(signal);
    
    return true;
  } else {
    Serial.printf("❌ ESP-NOW send failed: %d\n", result);
    return false;
  }
}

void EmergentSignalGenerator::processReceivedMessage(EmergentMessage* message) {
  Serial.printf("📨 Processing emergent message from %s\n", macToString(message->senderMac).c_str());
  
  // Verify checksum
  uint8_t calculatedChecksum = 0;
  uint8_t* data = (uint8_t*)message;
  for (uint16_t i = 0; i < sizeof(EmergentMessage) - 1; i++) {
    calculatedChecksum ^= data[i];
  }
  
  if (calculatedChecksum != message->checksum) {
    Serial.println("❌ Message checksum failed");
    return;
  }
  
  // Learn from this peer's signal
  learnFromPeerSignal(message->senderMac, &message->signal, (EnvironmentalContext)message->currentContext);
  
  // If message expects response, try to respond appropriately
  if (message->expectsResponse) {
    Serial.println("💬 Message expects response - generating reply");
    
    // Generate response based on our current context and the received signal
    EnvironmentalContext ourContext = getCurrentContext();
    EmotionalState ourEmotion = getCurrentEmotionalState();
    
    SignalWord* responseSignal = generateSignalForContext(ourContext, ourEmotion);
    if (responseSignal != nullptr) {
      // Send response after short delay
      delay(random(100, 500)); // Avoid collision
      sendEmergentMessage(responseSignal, ourContext, ourEmotion);
    }
  }
}

// ═══════════════════════════════════════════════════════════
// 🔧 UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════

uint8_t generateRandomComponent() {
  uint8_t components[] = {
    COMPONENT_TONE_LOW, COMPONENT_TONE_MID, COMPONENT_TONE_HIGH,
    COMPONENT_PULSE_FAST, COMPONENT_PULSE_SLOW,
    COMPONENT_SWEEP_UP, COMPONENT_SWEEP_DOWN, COMPONENT_SILENCE
  };
  return components[random(0, 8)];
}

uint16_t generateRandomDuration() {
  return random(100, 500);
}

uint8_t generateRandomIntensity() {
  return random(100, 255);
}

float calculateAcousticSimilarity(SignalWord* sig1, SignalWord* sig2) {
  if (sig1 == nullptr || sig2 == nullptr) return 0.0f;
  
  float similarity = 0.0f;
  uint8_t commonComponents = min(sig1->componentCount, sig2->componentCount);
  
  // Compare component types
  for (uint8_t i = 0; i < commonComponents; i++) {
    if (sig1->components[i] == sig2->components[i]) {
      similarity += 0.4f / commonComponents;
    }
    
    // Compare durations (allow 50% variance)
    float durationRatio = min(sig1->durations[i], sig2->durations[i]) / 
                         max(sig1->durations[i], sig2->durations[i]);
    if (durationRatio > 0.5f) {
      similarity += 0.3f / commonComponents;
    }
    
    // Compare intensities (allow 30% variance)
    float intensityRatio = min(sig1->intensities[i], sig2->intensities[i]) / 
                          max(sig1->intensities[i], sig2->intensities[i]);
    if (intensityRatio > 0.7f) {
      similarity += 0.3f / commonComponents;
    }
  }
  
  return similarity;
}

String contextToString(EnvironmentalContext context) {
  switch (context) {
    case CONTEXT_OBSTACLE_NEAR: return "OBSTACLE_NEAR";
    case CONTEXT_OPEN_SPACE: return "OPEN_SPACE";
    case CONTEXT_PEER_DETECTED: return "PEER_DETECTED";
    case CONTEXT_TASK_SUCCESS: return "TASK_SUCCESS";
    case CONTEXT_TASK_FAILURE: return "TASK_FAILURE";
    case CONTEXT_RESOURCE_FOUND: return "RESOURCE_FOUND";
    case CONTEXT_DANGER_SENSED: return "DANGER_SENSED";
    case CONTEXT_EXPLORATION: return "EXPLORATION";
    case CONTEXT_WAITING: return "WAITING";
    case CONTEXT_FOLLOWING: return "FOLLOWING";
    case CONTEXT_LEADING: return "LEADING";
    default: return "UNKNOWN";
  }
}

String emotionToString(EmotionalState emotion) {
  switch (emotion) {
    case EMOTION_VERY_NEGATIVE: return "VERY_NEGATIVE";
    case EMOTION_NEGATIVE: return "NEGATIVE";
    case EMOTION_NEUTRAL: return "NEUTRAL";
    case EMOTION_POSITIVE: return "POSITIVE";
    case EMOTION_VERY_POSITIVE: return "VERY_POSITIVE";
    default: return "UNKNOWN";
  }
}

String macToString(uint8_t* mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

bool macEquals(uint8_t* mac1, uint8_t* mac2) {
  return memcmp(mac1, mac2, 6) == 0;
}

void playSignalWord(SignalWord* signal) {
  // This would connect to speaker/buzzer hardware
  // For now, just debug output
  Serial.printf("🔊 Playing signal: %d components\n", signal->componentCount);
  for (uint8_t i = 0; i < signal->componentCount; i++) {
    Serial.printf("   Component %d: type=%d, duration=%dms, intensity=%d\n", 
                  i, signal->components[i], signal->durations[i], signal->intensities[i]);
  }
}

// ═══════════════════════════════════════════════════════════
// 🧹 VOCABULARY MANAGEMENT
// ═══════════════════════════════════════════════════════════

void EmergentSignalGenerator::pruneUnusedSignals() {
  uint8_t writeIndex = 0;
  
  // Keep signals that have been used recently or frequently
  for (uint8_t readIndex = 0; readIndex < vocabularySize; readIndex++) {
    SignalWord* signal = &vocabulary[readIndex];
    
    // Keep signal if:
    // 1. It's been used in the last 10 minutes
    // 2. It has high utility (> 0.5)
    // 3. It's been used more than 5 times
    unsigned long timeSinceLastUse = millis() - signal->lastUsed;
    bool keepSignal = (timeSinceLastUse < 600000) ||  // 10 minutes
                      (signal->utility > 0.5) ||
                      (signal->timesUsed > 5);
    
    if (keepSignal && writeIndex != readIndex) {
      vocabulary[writeIndex] = vocabulary[readIndex];
    }
    
    if (keepSignal) {
      writeIndex++;
    } else {
      Serial.printf("🧹 Pruning unused signal (utility: %.2f, uses: %d)\n", 
                    signal->utility, signal->timesUsed);
    }
  }
  
  vocabularySize = writeIndex;
  Serial.printf("🧹 Vocabulary pruned: %d signals remaining\n", vocabularySize);
}