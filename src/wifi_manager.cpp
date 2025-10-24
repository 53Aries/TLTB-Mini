#include "wifi_manager.hpp"

namespace {
  Preferences* g_prefs = nullptr;
  WiFiManager::WiFiState g_state = WiFiManager::WIFI_DISABLED;
  uint32_t g_lastConnectionAttempt = 0;
  uint32_t g_connectionTimeout = 30000; // 30 seconds
  String g_deviceName = "TLTB-Mini";
  String g_apSSID = "";
  bool g_apStarted = false;
  bool g_wifiJustConnected = false;
  
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
  
  // Simple mode: always start AP for reliability
  Serial.println("[WiFi] Starting simple AP mode");
  startAP();
}

void service() {
  // Simple service loop - no DNS server processing
  
  switch (g_state) {
    case WIFI_STA_CONNECTING:
      if (WiFi.status() == WL_CONNECTED) {
        g_state = WIFI_STA_CONNECTED;
        IPAddress ip = WiFi.localIP();
        Serial.printf("[WiFi] Connected! IP: %s\n", ip.toString().c_str());
        
        // Set flag for audio signal
        g_wifiJustConnected = true;
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
      // AP mode is running, no additional processing needed
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

bool checkWiFiJustConnected() {
  if (g_wifiJustConnected) {
    g_wifiJustConnected = false; // Reset flag
    return true;
  }
  return false;
}

void startAP() {
  WiFi.mode(WIFI_AP);
  // Keep radio fully awake for reliability in AP mode
  WiFi.setSleep(false);
  // Use strong transmit power for better range/stability
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  
  // Simple AP configuration
  WiFi.softAPConfig(
    IPAddress(192, 168, 4, 1),    // AP IP
    IPAddress(192, 168, 4, 1),    // Gateway (ourselves)
    IPAddress(255, 255, 255, 0)   // Subnet mask
  );
  
  // Start Access Point with WPA2 password for better client compatibility
  // Channel 1, not hidden, max 4 clients
  const char* ap_password = "TLTB1234"; // 8+ chars as required by WPA2
  bool success = WiFi.softAP(g_apSSID.c_str(), ap_password, 1, 0, 4);
  
  if (success) {
    g_state = WIFI_AP_MODE;
    g_apStarted = true;
    
    Serial.printf("[WiFi] AP started: %s (password: %s)\n", g_apSSID.c_str(), ap_password);
    Serial.printf("[WiFi] AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.println("[WiFi] Connect to WiFi, then go to: http://192.168.4.1");
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