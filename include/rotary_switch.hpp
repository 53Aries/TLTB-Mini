#pragma once
#include <Arduino.h>
#include "relays.hpp"

// Rotary switch mode enumeration
enum RotaryMode {
  MODE_ALL_OFF = 0,   // P1
  MODE_RF_ENABLE,     // P2
  MODE_LEFT,          // P3
  MODE_RIGHT,         // P4
  MODE_BRAKE,         // P5
  MODE_TAIL,          // P6
  MODE_MARKER,        // P7
  MODE_AUX            // P8
};

namespace RotarySwitch {
  // Initialize rotary switch pins
  void begin();
  
  // Read current rotary switch position
  RotaryMode readPosition();
  
  // Enforce rotary switch mode (override RF control except in RF_ENABLE mode)
  void enforceMode(RotaryMode mode);
  
  // Get mode name as string
  const char* getModeName(RotaryMode mode);
}