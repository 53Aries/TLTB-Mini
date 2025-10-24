#pragma once

// TLTB Mini - ESP32-S3 Pin Definitions
// Based on ESP32-S3-DevKitC-1 with full GPIO availability
// Compatible with original TLTB pin layout

// ======================= RF (SYN480R Receiver) =======================
#define PIN_RF_DATA     21  // RF receiver data pin (level-shifted to 3.3V)

// ======================= Rotary 1P8T Mode Selector =======================
#define PIN_ROT_P1      4   // All Off
#define PIN_ROT_P2      5   // RF Enable  
#define PIN_ROT_P3      6   // Left
#define PIN_ROT_P4      7   // Right
#define PIN_ROT_P5      15  // Brake
#define PIN_ROT_P6      16  // Tail
#define PIN_ROT_P7      17  // Marker
#define PIN_ROT_P8      18  // Aux

// ======================= Relays (active-low) =======================
#define PIN_RELAY_LEFT     8   // Left Turn
#define PIN_RELAY_RIGHT    9   // Right Turn  
#define PIN_RELAY_BRAKE    10  // Brake Lights
#define PIN_RELAY_TAIL     11  // Tail Lights
#define PIN_RELAY_MARKER   12  // Marker Lights
#define PIN_RELAY_AUX      13  // Auxiliary
#define PIN_RELAY_ENABLE   14  // Enable 12V buck

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
#define PIN_BUZZER      35  // Piezo buzzer for feedback

// ======================= Status LED =======================
#define PIN_STATUS_LED  2   // Built-in LED or external status LED

// ======================= I2C Bus (for future expansion) =======================
#define PIN_I2C_SDA     47  // I2C SDA (for sensors, displays, etc.)
#define PIN_I2C_SCL     48  // I2C SCL

// ======================= SPI (for future expansion) =======================
#define PIN_SPI_SCK     37  // SPI Clock
#define PIN_SPI_MOSI    38  // SPI MOSI
#define PIN_SPI_MISO    36  // SPI MISO