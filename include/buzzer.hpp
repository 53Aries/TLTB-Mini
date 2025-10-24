#pragma once
#include <Arduino.h>

namespace Buzzer {
  // Initialize buzzer
  void begin();
  
  // Single beep for feedback
  void beep();
  
  // Fault pattern buzzing (for protection system)
  void tick(bool faultActive, uint32_t currentMs);
  
  // Multi-tone sequence for learning confirmation
  void learnSuccess();
}