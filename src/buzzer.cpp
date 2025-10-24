#include "buzzer.hpp"
#include "pins.hpp"

namespace {
  uint32_t g_faultLastMs = 0;
  bool g_faultToneOn = false;
}

namespace Buzzer {

void begin() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
}

void beep() {
  // Short 100ms beep at 2kHz
  tone(PIN_BUZZER, 2000, 100);
}

void tick(bool faultActive, uint32_t currentMs) {
  if (!faultActive) {
    g_faultToneOn = false;
    noTone(PIN_BUZZER);
    return;
  }
  
  // Fault pattern: 500ms on, 500ms off
  if (currentMs - g_faultLastMs >= 500) {
    g_faultLastMs = currentMs;
    g_faultToneOn = !g_faultToneOn;
    
    if (g_faultToneOn) {
      tone(PIN_BUZZER, 1000); // 1kHz fault tone
    } else {
      noTone(PIN_BUZZER);
    }
  }
}

void learnSuccess() {
  // Three ascending tones
  tone(PIN_BUZZER, 1000, 150);
  delay(200);
  tone(PIN_BUZZER, 1500, 150);
  delay(200);
  tone(PIN_BUZZER, 2000, 150);
}

}