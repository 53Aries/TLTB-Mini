#include <Arduino.h>
#include <Preferences.h>

#include "pins.hpp"
#include "relays.hpp"
#include "RF.hpp"
#include "buzzer.hpp"
#include "rotary_switch.hpp"
#include "wifi_manager.hpp"
#include "web_server.hpp"

// ---------------- Globals ----------------
Preferences prefs;
static uint32_t g_lastStatusPrint = 0;
static RotaryMode g_lastMode = MODE_ALL_OFF;

// WiFi connection signal function
void signalWiFiConnected() {
  for (int i = 0; i < 3; i++) {
    Buzzer::beep();
    delay(100);
  }
}

// ---------------- Serial Commands ----------------
void handleSerialCommands() {
  if (!Serial.available()) return;
  
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toUpperCase();
  
  if (cmd.startsWith("LEARN")) {
    // Extract channel number: "LEARN 0", "LEARN 1", etc.
    int channel = 0;
    if (cmd.length() > 6) {
      channel = cmd.substring(6).toInt();
    }
    if (channel < 0 || channel > 5) {
      Serial.println("Invalid channel. Use LEARN 0-5");
      return;
    }
    
    Serial.printf("Starting RF learning for channel %d (relay %s)...\n", 
                  channel, relayName((RelayIndex)channel));
    if (RF::startLearning(channel)) {
      Serial.println("Learning started. Press remote button within 10 seconds.");
    } else {
      Serial.println("Failed to start learning - another learning session may be active.");
    }
    
  } else if (cmd == "CLEAR") {
    Serial.println("Clearing all learned RF codes...");
    RF::clearAll();
    
  } else if (cmd == "STATUS") {
    RotaryMode currentMode = RotarySwitch::readPosition();
    Serial.printf("Switch Position: %s\n", RotarySwitch::getModeName(currentMode));
    Serial.printf("RF Active Relay: %d\n", RF::getActiveRelay());
    
    // Show learning status
    RF::LearningStatus learningStatus = RF::getLearningStatus();
    if (learningStatus.active) {
      Serial.printf("Learning: Active for channel %d (%d seconds remaining)\n", 
                    learningStatus.channel, learningStatus.timeRemaining / 1000);
    } else {
      Serial.println("Learning: Inactive");
    }
    
    Serial.println("Relay States:");
    for (int i = 0; i < R_COUNT; i++) {
      Serial.printf("  %s: %s\n", relayName((RelayIndex)i), 
                    relayIsOn(i) ? "ON" : "OFF");
    }
    
  } else if (cmd == "HELP") {
    Serial.println("Available commands:");
    Serial.println("  LEARN <0-5>  - Learn RF code for relay channel");
    Serial.println("  CLEAR        - Clear all learned RF codes");
    Serial.println("  STATUS       - Show system status");
    Serial.println("  HELP         - Show this help");
    
  } else if (cmd.length() > 0) {
    Serial.printf("Unknown command: %s\n", cmd.c_str());
    Serial.println("Type HELP for available commands");
  }
}

// ---------------- Status LED ----------------
void updateStatusLED() {
  static uint32_t lastBlink = 0;
  static bool ledState = false;
  
  RotaryMode currentMode = RotarySwitch::readPosition();
  
  RF::LearningStatus learningStatus = RF::getLearningStatus();
  if (learningStatus.active) {
    // Fast blink during learning
    if (millis() - lastBlink > 100) {
      ledState = !ledState;
      digitalWrite(PIN_STATUS_LED, ledState);
      lastBlink = millis();
    }
  } else if (currentMode == MODE_RF_ENABLE) {
    // Slow blink when RF is enabled
    if (millis() - lastBlink > 500) {
      ledState = !ledState;
      digitalWrite(PIN_STATUS_LED, ledState);
      lastBlink = millis();
    }
  } else {
    // Solid on for manual modes, off for ALL_OFF
    digitalWrite(PIN_STATUS_LED, currentMode != MODE_ALL_OFF);
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== TLTB Mini - ESP32-S3 ===");
  
  // Initialize status LED
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, HIGH);
  
  // Initialize preferences
  prefs.begin("tltb_mini", false);
  
  // Initialize WiFi first
  WiFiManager::begin(&prefs);
  
  // Initialize Web Server
  WebServer::begin(&prefs);
  
  // Give WiFi time to initialize
  delay(1000);
  
  // Start web server
  WebServer::start();
  
  // Initialize modules
  relaysBegin();
  Serial.println("Relays initialized");
  
  RotarySwitch::begin();
  Serial.println("Rotary switch initialized");
  
  Buzzer::begin();
  Serial.println("Buzzer initialized");
  
  if (RF::begin()) {
    Serial.println("RF receiver initialized");
  } else {
    Serial.println("RF receiver failed to initialize");
  }
  
  // Startup beep
  Buzzer::beep();
  delay(200);
  Buzzer::beep();
  
  Serial.println("=== System Ready ===");
  
  // Wait a moment for WiFi to stabilize
  delay(2000);
  
  // WiFi details omitted intentionally
  
  // Show initial status
  RotaryMode initialMode = RotarySwitch::readPosition();
  Serial.printf("Initial switch position: %s\n", RotarySwitch::getModeName(initialMode));
}

// ---------------- Main Loop ----------------
void loop() {
  // Handle WiFi and Web Server
  WiFiManager::service();
  WebServer::service();
  
  // Check for WiFi connection notification
  if (WiFiManager::checkWiFiJustConnected()) {
    signalWiFiConnected();
  }
  
  // Handle serial commands
  handleSerialCommands();
  
  // Update status LED
  updateStatusLED();
  
  // Read current rotary switch position
  RotaryMode currentMode = RotarySwitch::readPosition();
  
  // Print mode changes
  if (currentMode != g_lastMode) {
    Serial.printf("Switch changed to: %s\n", RotarySwitch::getModeName(currentMode));
    g_lastMode = currentMode;
    Buzzer::beep();
  }
  
  // Let RF run, but we will enforce rotary below unless in MODE_RF_ENABLE
  RF::service();
  
  // Rotary has final say unless P2 (RF enabled)
  RotarySwitch::enforceMode(currentMode);
  
  // Periodic status print (every 30 seconds)
  if (millis() - g_lastStatusPrint > 30000) {
    g_lastStatusPrint = millis();
    Serial.printf("[Status] Mode: %s, RF Active: %d, Enable: %s\n", 
                  RotarySwitch::getModeName(currentMode),
                  RF::getActiveRelay(),
                  relayIsOn(R_ENABLE) ? "ON" : "OFF");
                  
    // WiFi diagnostics omitted
  }
  
  delay(1); // keep responsive
}