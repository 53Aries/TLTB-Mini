#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

namespace WiFiManager {
  // WiFi states
  enum WiFiState {
    WIFI_DISABLED,
    WIFI_AP_MODE,
    WIFI_STA_CONNECTING,
    WIFI_STA_CONNECTED,
    WIFI_STA_FAILED
  };

  // Initialize WiFi system
  void begin(Preferences* prefs);
  
  // Service routine - call in main loop
  void service();
  
  // Get current WiFi state
  WiFiState getState();
  
  // Get current IP address
  String getIPAddress();
  
  // Get AP SSID when in AP mode
  String getAPSSID();
  
  // Check if WiFi just connected (for notifications)
  bool checkWiFiJustConnected();
  
  // Start Access Point mode for configuration
  void startAP();
  
  // Connect to WiFi network
  bool connectToWiFi(const String& ssid, const String& password);
  
  // Save WiFi credentials
  void saveCredentials(const String& ssid, const String& password);
  
  // Clear saved credentials
  void clearCredentials();
  
  // Get saved SSID
  String getSavedSSID();
  
  // Check if credentials are saved
  bool hasCredentials();
}