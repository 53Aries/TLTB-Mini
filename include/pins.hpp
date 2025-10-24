#pragma once

// TLTB Mini - ESP32-C3 Pin Definitions
// Optimized for ESP32-C3 DevKit M1 with minimal external components

// ======================= RF (SYN480R Receiver) =======================
#define PIN_RF_DATA     2   // RF receiver data pin (level-shifted to 3.3V)

// ======================= Rotary 1P8T Mode Selector =======================
#define PIN_ROT_P1      3   // All Off
#define PIN_ROT_P2      4   // RF Enable  
#define PIN_ROT_P3      5   // Left
#define PIN_ROT_P4      6   // Right
#define PIN_ROT_P5      7   // Brake
#define PIN_ROT_P6      8   // Tail
#define PIN_ROT_P7      9   // Marker
#define PIN_ROT_P8      10  // Aux

// ======================= Relays (active-low) =======================
#define PIN_RELAY_LEFT     0   // Left Turn
#define PIN_RELAY_RIGHT    1   // Right Turn  
#define PIN_RELAY_BRAKE    18  // Brake Lights
#define PIN_RELAY_TAIL     19  // Tail Lights
#define PIN_RELAY_MARKER   20  // Marker Lights
#define PIN_RELAY_AUX      21  // Auxiliary
#define PIN_RELAY_ENABLE   12  // Enable 12V buck

// Array for relay status logic
static const int RELAY_PIN[] = {
  PIN_RELAY_LEFT,
  PIN_RELAY_RIGHT,
  PIN_RELAY_BRAKE,
  PIN_RELAY_TAIL,
  PIN_RELAY_MARKER,
  PIN_RELAY_AUX,
  PIN_RELAY_ENABLE
};

// ======================= Buzzer =======================
#define PIN_BUZZER      13  // Piezo buzzer for feedback

// ======================= Status LED =======================
#define PIN_STATUS_LED  14  // Status indication LED