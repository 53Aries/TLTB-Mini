#pragma once
#include <Arduino.h>

namespace RF {
  // Initialize RF receiver
  bool begin();
  
  // Service routine - call frequently in main loop
  void service();
  
  // Check if RF hardware is present
  bool isPresent();
  
  // Learn a remote control signal for a specific relay index (0-5)
  bool learn(int relayIndex);
  
  // Start learning mode asynchronously (non-blocking)
  bool startLearning(int relayIndex);
  
  // Check if learning is in progress
  bool isLearning();
  
  // Get learning progress/status
  struct LearningStatus {
    bool active;
    int channel;
    uint32_t startTime;
    uint32_t timeRemaining;
    bool success;
    bool timeout;
  };
  LearningStatus getLearningStatus();
  
  // Stop learning mode
  void stopLearning();
  
  // Clear all learned codes
  bool clearAll();
  
  // Get current active relay (-1 if none)
  int8_t getActiveRelay();
}