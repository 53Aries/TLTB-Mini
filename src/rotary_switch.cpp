#include "rotary_switch.hpp"
#include "pins.hpp"

namespace RotarySwitch {

void begin() {
  // Initialize all rotary switch pins with pullups
  pinMode(PIN_ROT_P1, INPUT_PULLUP);
  pinMode(PIN_ROT_P2, INPUT_PULLUP);
  pinMode(PIN_ROT_P3, INPUT_PULLUP);
  pinMode(PIN_ROT_P4, INPUT_PULLUP);
  pinMode(PIN_ROT_P5, INPUT_PULLUP);
  pinMode(PIN_ROT_P6, INPUT_PULLUP);
  pinMode(PIN_ROT_P7, INPUT_PULLUP);
  pinMode(PIN_ROT_P8, INPUT_PULLUP);
}

RotaryMode readPosition() {
  // Inputs are PULLUP, so LOW = active position
  if (digitalRead(PIN_ROT_P1) == LOW) return MODE_ALL_OFF;
  if (digitalRead(PIN_ROT_P2) == LOW) return MODE_RF_ENABLE;
  if (digitalRead(PIN_ROT_P3) == LOW) return MODE_LEFT;
  if (digitalRead(PIN_ROT_P4) == LOW) return MODE_RIGHT;
  if (digitalRead(PIN_ROT_P5) == LOW) return MODE_BRAKE;
  if (digitalRead(PIN_ROT_P6) == LOW) return MODE_TAIL;
  if (digitalRead(PIN_ROT_P7) == LOW) return MODE_MARKER;
  if (digitalRead(PIN_ROT_P8) == LOW) return MODE_AUX;
  return MODE_ALL_OFF; // fallback if between detents or no input
}

void enforceMode(RotaryMode mode) {
  // In all non-RF modes, we *force* the relay states each loop.
  // This guarantees RF is effectively ignored unless in MODE_RF_ENABLE.
  auto allOff = [](){
    for (int i = 0; i < (int)R_COUNT; ++i) relayOff(i);
  };

  switch (mode) {
    case MODE_ALL_OFF:
      allOff();
      break;

    case MODE_RF_ENABLE:
      // Do not force anything; RF subsystem may control relays.
      break;

    case MODE_LEFT:
      allOff(); 
      relayOn(R_LEFT);
      break;

    case MODE_RIGHT:
      allOff(); 
      relayOn(R_RIGHT);
      break;

    case MODE_BRAKE:
      allOff(); 
      relayOn(R_BRAKE);
      break;

    case MODE_TAIL:
      allOff(); 
      relayOn(R_TAIL);
      break;

    case MODE_MARKER:
      allOff(); 
      relayOn(R_MARKER);
      break;

    case MODE_AUX:
      allOff(); 
      relayOn(R_AUX);
      break;
  }

  // Relay 7 (R_ENABLE) must be OFF when the selector is in position 1 (ALL_OFF)
  // and ON in all other positions. This is independent of RF control for the
  // other relays — we enforce it here.
  if (mode == MODE_ALL_OFF) {
    relayOff(R_ENABLE);
  } else {
    relayOn(R_ENABLE);
  }
}

const char* getModeName(RotaryMode mode) {
  switch (mode) {
    case MODE_ALL_OFF:   return "ALL OFF";
    case MODE_RF_ENABLE: return "RF ENABLE";
    case MODE_LEFT:      return "LEFT";
    case MODE_RIGHT:     return "RIGHT";
    case MODE_BRAKE:     return "BRAKE";
    case MODE_TAIL:      return "TAIL";
    case MODE_MARKER:    return "MARKER";
    case MODE_AUX:       return "AUX";
    default:             return "UNKNOWN";
  }
}

} // namespace RotarySwitch