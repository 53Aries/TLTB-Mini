#include "wifi_manager.hpp"
#include <DNSServer.h>

namespace {
  Preferences* g_prefs = nullptr;
  WiFiManager::WiFiState g_state = WiFiManager::WIFI_DISABLED;
  uint32_t g_lastConnectionAttempt = 0;
  uint32_t g_connectionTimeout = 30000; // 30 seconds
  String g_deviceName = "TLTB-Mini";
  String g_apSSID = "";
  DNSServer g_dnsServer;
  bool g_apStarted = false;
  
  // Keys for preferences
  const char* KEY_WIFI_SSID = "wifi_ssid";
  const char* KEY_WIFI_PASS = "wifi_pass";
}

namespace WiFiManager {

void begin(Preferences* prefs) {
  g_prefs = prefs;
  
  // Generate unique AP SSID based on MAC address
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  g_apSSID = g_deviceName + "-" + mac.substring(8);
  
  WiFi.mode(WIFI_OFF);
  delay(100);
  
  // Check if we have saved credentials
  if (hasCredentials()) {
    String ssid = getSavedSSID();
    String password = g_prefs->getString(KEY_WIFI_PASS, "");
    
    Serial.printf("[WiFi] Found saved credentials for: %s\n", ssid.c_str());
    
    // Try to connect automatically
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    g_state = WIFI_STA_CONNECTING;
    g_lastConnectionAttempt = millis();
  } else {
    Serial.println("[WiFi] No saved credentials, starting AP mode");
    startAP();
  }
}

void service() {
  switch (g_state) {
    case WIFI_STA_CONNECTING:
      if (WiFi.status() == WL_CONNECTED) {
        g_state = WIFI_STA_CONNECTED;
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
      } else if (millis() - g_lastConnectionAttempt > g_connectionTimeout) {
        g_state = WIFI_STA_FAILED;
        Serial.println("[WiFi] Connection failed, starting AP mode");
        startAP();
      }
      break;
      
    case WIFI_STA_CONNECTED:
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] Connection lost, attempting to reconnect");
        g_state = WIFI_STA_CONNECTING;
        g_lastConnectionAttempt = millis();
      }
      break;
      
    case WIFI_AP_MODE:
      if (g_apStarted) {
        g_dnsServer.processNextRequest();
      }
      break;
      
    default:
      break;
  }
}

WiFiState getState() {
  return g_state;
}

String getIPAddress() {
  switch (g_state) {
    case WIFI_STA_CONNECTED:
      return WiFi.localIP().toString();
    case WIFI_AP_MODE:
      return WiFi.softAPIP().toString();
    default:
      return "0.0.0.0";
  }
}

String getAPSSID() {
  return g_apSSID;
}

void startAP() {
  WiFi.mode(WIFI_AP);
  
  // Start Access Point
  bool success = WiFi.softAP(g_apSSID.c_str(), "TLTB1234"); // Default password
  
  if (success) {
    g_state = WIFI_AP_MODE;
    
    // Start DNS server for captive portal
    g_dnsServer.start(53, "*", WiFi.softAPIP());
    g_apStarted = true;
    
    Serial.printf("[WiFi] AP started: %s\n", g_apSSID.c_str());
    Serial.printf("[WiFi] AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.println("[WiFi] Default password: TLTB1234");
  } else {
    Serial.println("[WiFi] Failed to start AP");
    g_state = WIFI_DISABLED;
  }
}

bool connectToWiFi(const String& ssid, const String& password) {
  Serial.printf("[WiFi] Attempting to connect to: %s\n", ssid.c_str());
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  g_state = WIFI_STA_CONNECTING;
  g_lastConnectionAttempt = millis();
  
  return true; // Return immediately, check status in service()
}

void saveCredentials(const String& ssid, const String& password) {
  if (g_prefs) {
    g_prefs->putString(KEY_WIFI_SSID, ssid);
    g_prefs->putString(KEY_WIFI_PASS, password);
    Serial.printf("[WiFi] Credentials saved for: %s\n", ssid.c_str());
  }
}

void clearCredentials() {
  if (g_prefs) {
    g_prefs->remove(KEY_WIFI_SSID);
    g_prefs->remove(KEY_WIFI_PASS);
    Serial.println("[WiFi] Credentials cleared");
  }
}

String getSavedSSID() {
  if (g_prefs) {
    return g_prefs->getString(KEY_WIFI_SSID, "");
  }
  return "";
}

bool hasCredentials() {
  String ssid = getSavedSSID();
  return ssid.length() > 0;
}

}