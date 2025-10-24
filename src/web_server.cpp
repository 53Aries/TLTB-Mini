#include "web_server.hpp"
#include "wifi_manager.hpp"
#include "RF.hpp"
#include "relays.hpp"
#include "rotary_switch.hpp"
#include <Arduino_JSON.h>
#include <WiFi.h>

namespace {
  AsyncWebServer* g_server = nullptr;
  Preferences* g_prefs = nullptr;
  bool g_serverRunning = false;
  
  // HTML pages as strings
  const char* HTML_INDEX = R"html(
<!DOCTYPE html>
<html>
<head>
    <title>TLTB Mini Configuration</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        .section { margin: 20px 0; padding: 15px; border: 1px solid #ddd; border-radius: 5px; }
        .status { background: #e8f5e8; }
        .wifi { background: #e8f0ff; }
        .rf { background: #fff8e8; }
        .relays { background: #f8e8ff; }
        button { padding: 10px 15px; margin: 5px; background: #007bff; color: white; border: none; border-radius: 3px; cursor: pointer; }
        button:hover { background: #0056b3; }
        button:disabled { background: #ccc; cursor: not-allowed; }
        input, select { padding: 8px; margin: 5px; border: 1px solid #ddd; border-radius: 3px; }
        .relay-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 10px; }
        .relay-item { padding: 10px; border: 1px solid #ddd; border-radius: 5px; text-align: center; }
        .relay-on { background: #d4edda; border-color: #c3e6cb; }
        .relay-off { background: #f8d7da; border-color: #f5c6cb; }
        .learn-button { background: #28a745; }
        .learn-button:hover { background: #218838; }
        .learn-active { background: #ffc107; color: black; }
        #status { font-weight: bold; margin: 10px 0; }
        .info-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚛 TLTB Mini Configuration</h1>
        
        <div class="section">
            <h3>📱 Android Users</h3>
            <p style="background: #fff3cd; padding: 10px; border-radius: 5px; border-left: 4px solid #ffc107;">
                <strong>Connection Tip:</strong> If Android shows "No internet" and tries to disconnect, 
                tap "Use network as is" or "Stay connected" in the WiFi notification. 
                This is normal for local device configuration networks.
            </p>
        </div>
        
        <div class="section status">
            <h2>📊 System Status</h2>
            <div id="status">Loading...</div>
            <div class="info-grid">
                <div><strong>WiFi:</strong> <span id="wifi-status">-</span></div>
                <div><strong>IP:</strong> <span id="ip-address">-</span></div>
                <div><strong>Switch:</strong> <span id="switch-position">-</span></div>
                <div><strong>RF Active:</strong> <span id="rf-active">-</span></div>
            </div>
            <div id="connection-info" style="display: none; background: #e8f5e8; padding: 10px; border-radius: 5px; margin: 10px 0;">
                <h4>🔗 Direct IP Access:</h4>
                <div><strong>IP Address:</strong> <a id="ip-link" href="#" target="_blank">-</a></div>
                <div><strong>Bookmark this page</strong> for easy future access!</div>
            </div>
            <button onclick="updateStatus()">🔄 Refresh</button>
        </div>

        <div class="section wifi">
            <h2>📶 WiFi Configuration</h2>
            <div>
                <label>SSID:</label>
                <input type="text" id="wifi-ssid" placeholder="Enter WiFi network name">
            </div>
            <div>
                <label>Password:</label>
                <input type="password" id="wifi-password" placeholder="Enter WiFi password">
            </div>
            <button onclick="connectWiFi()">🔗 Connect to WiFi</button>
            <button onclick="clearWiFi()">🗑️ Clear Saved WiFi</button>
        </div>

        <div class="section rf">
            <h2>📡 RF Remote Learning</h2>
            <p>Select a relay channel and press the button on your remote control to learn it.</p>
            <div class="relay-grid">
                <div class="relay-item">
                    <div>LEFT Turn</div>
                    <button class="learn-button" onclick="learnRF(0)" id="learn-0">Learn LEFT</button>
                </div>
                <div class="relay-item">
                    <div>RIGHT Turn</div>
                    <button class="learn-button" onclick="learnRF(1)" id="learn-1">Learn RIGHT</button>
                </div>
                <div class="relay-item">
                    <div>BRAKE Lights</div>
                    <button class="learn-button" onclick="learnRF(2)" id="learn-2">Learn BRAKE</button>
                </div>
                <div class="relay-item">
                    <div>TAIL Lights</div>
                    <button class="learn-button" onclick="learnRF(3)" id="learn-3">Learn TAIL</button>
                </div>
                <div class="relay-item">
                    <div>MARKER Lights</div>
                    <button class="learn-button" onclick="learnRF(4)" id="learn-4">Learn MARKER</button>
                </div>
                <div class="relay-item">
                    <div>AUX Output</div>
                    <button class="learn-button" onclick="learnRF(5)" id="learn-5">Learn AUX</button>
                </div>
            </div>
            <button onclick="clearAllRF()">🗑️ Clear All RF Codes</button>
            <div id="rf-status"></div>
        </div>

        <div class="section relays">
            <h2>🔌 Relay Status</h2>
            <div class="relay-grid" id="relay-status">
                Loading...
            </div>
        </div>
    </div>

    <script>
        function updateStatus() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('wifi-status').textContent = data.wifi_state;
                    document.getElementById('ip-address').textContent = data.ip_address;
                    document.getElementById('switch-position').textContent = data.switch_position;
                    document.getElementById('rf-active').textContent = data.rf_active_relay;
                    
                    // Show connection info if connected to WiFi
                    if (data.wifi_state === 'Connected' && data.ip_address !== '0.0.0.0') {
                        const connectionInfo = document.getElementById('connection-info');
                        const ipLink = document.getElementById('ip-link');
                        
                        ipLink.href = 'http://' + data.ip_address;
                        ipLink.textContent = 'http://' + data.ip_address;
                        
                        connectionInfo.style.display = 'block';
                    } else {
                        document.getElementById('connection-info').style.display = 'none';
                    }
                    
                    // Update relay status
                    updateRelayStatus(data.relays);
                })
                .catch(error => {
                    document.getElementById('status').textContent = 'Error: ' + error;
                });
        }

        function updateRelayStatus(relays) {
            const relayNames = ['LEFT', 'RIGHT', 'BRAKE', 'TAIL', 'MARKER', 'AUX', 'ENABLE'];
            let html = '';
            
            for (let i = 0; i < relays.length; i++) {
                const isOn = relays[i];
                const className = isOn ? 'relay-on' : 'relay-off';
                const status = isOn ? 'ON' : 'OFF';
                html += `<div class="relay-item ${className}">
                    <strong>${relayNames[i]}</strong><br>
                    Status: ${status}
                </div>`;
            }
            
            document.getElementById('relay-status').innerHTML = html;
        }

        function connectWiFi() {
            const ssid = document.getElementById('wifi-ssid').value;
            const password = document.getElementById('wifi-password').value;
            
            if (!ssid) {
                alert('Please enter WiFi SSID');
                return;
            }
            
            fetch('/api/wifi/connect', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({ssid: ssid, password: password})
            })
            .then(response => response.json())
            .then(data => {
                alert(data.message);
                if (data.success) {
                    setTimeout(() => {
                        window.location.reload();
                    }, 3000);
                }
            });
        }

        function clearWiFi() {
            if (confirm('Clear saved WiFi credentials?')) {
                fetch('/api/wifi/clear', {method: 'POST'})
                    .then(response => response.json())
                    .then(data => alert(data.message));
            }
        }

        function learnRF(channel) {
            const button = document.getElementById('learn-' + channel);
            button.textContent = 'Starting...';
            button.className = 'learn-active';
            button.disabled = true;
            
            fetch('/api/rf/learn', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({channel: channel})
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    // Start polling for learning status
                    startLearningStatusPoll(channel);
                } else {
                    alert(data.message);
                    resetLearnButton(channel);
                }
            });
        }

        function startLearningStatusPoll(channel) {
            const button = document.getElementById('learn-' + channel);
            const channelNames = ['LEFT', 'RIGHT', 'BRAKE', 'TAIL', 'MARKER', 'AUX'];
            
            const pollInterval = setInterval(() => {
                fetch('/api/status')
                    .then(response => response.json())
                    .then(data => {
                        const learning = data.learning;
                        
                        if (!learning.active) {
                            clearInterval(pollInterval);
                            if (learning.success) {
                                alert('Successfully learned RF code for ' + channelNames[channel]);
                            } else if (learning.timeout) {
                                alert('Learning timeout - no signal received within 30 seconds');
                            } else {
                                alert('Learning failed');
                            }
                            resetLearnButton(channel);
                        } else {
                            // Update button with countdown
                            const timeLeft = Math.ceil(learning.time_remaining);
                            button.textContent = `Learning... ${timeLeft}s`;
                        }
                    })
                    .catch(error => {
                        clearInterval(pollInterval);
                        alert('Error: ' + error);
                        resetLearnButton(channel);
                    });
            }, 1000); // Poll every second
        }

        function resetLearnButton(channel) {
            const button = document.getElementById('learn-' + channel);
            const channelNames = ['LEFT', 'RIGHT', 'BRAKE', 'TAIL', 'MARKER', 'AUX'];
            button.textContent = 'Learn ' + channelNames[channel];
            button.className = 'learn-button';
            button.disabled = false;
        }

        function clearAllRF() {
            if (confirm('Clear all learned RF codes?')) {
                fetch('/api/rf/clear', {method: 'POST'})
                    .then(response => response.json())
                    .then(data => alert(data.message));
            }
        }

        // Auto-refresh status every 5 seconds
        setInterval(updateStatus, 5000);
        
        // Initial load
        updateStatus();
    </script>
</body>
</html>
)html";

  void handleNotFound(AsyncWebServerRequest *request) {
    String path = request->url();
    Serial.printf("[WebServer] 404 Request: %s\n", path.c_str());
    request->send(404, "text/plain", "Not found - try http://192.168.4.1");
  }

  void handleIndex(AsyncWebServerRequest *request) {
    request->send(200, "text/html", HTML_INDEX);
  }

  void handleAPIStatus(AsyncWebServerRequest *request) {
    JSONVar json;
    
    json["wifi_state"] = [](){ 
      switch(WiFiManager::getState()) {
        case WiFiManager::WIFI_AP_MODE: return "AP Mode";
        case WiFiManager::WIFI_STA_CONNECTED: return "Connected";
        case WiFiManager::WIFI_STA_CONNECTING: return "Connecting";
        case WiFiManager::WIFI_STA_FAILED: return "Failed";
        default: return "Disabled";
      }
    }();
    
    json["ip_address"] = WiFiManager::getIPAddress();
    json["switch_position"] = RotarySwitch::getModeName(RotarySwitch::readPosition());
    json["rf_active_relay"] = RF::getActiveRelay();
    
    // Add learning status
    RF::LearningStatus learningStatus = RF::getLearningStatus();
    JSONVar learning;
    learning["active"] = learningStatus.active;
    learning["channel"] = learningStatus.channel;
    learning["time_remaining"] = learningStatus.timeRemaining / 1000; // Convert to seconds
    learning["success"] = learningStatus.success;
    learning["timeout"] = learningStatus.timeout;
    json["learning"] = learning;
    
    JSONVar relays;
    for (int i = 0; i < R_COUNT; i++) {
      relays[i] = relayIsOn(i);
    }
    json["relays"] = relays;
    
    request->send(200, "application/json", JSON.stringify(json));
  }

  void handleWiFiConnect(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    JSONVar json = JSON.parse((char*)data);
    
    if (json.hasOwnProperty("ssid")) {
      String ssid = (const char*)json["ssid"];
      String password = json.hasOwnProperty("password") ? (const char*)json["password"] : "";
      
      WiFiManager::saveCredentials(ssid, password);
      WiFiManager::connectToWiFi(ssid, password);
      
      JSONVar response;
      response["success"] = true;
      response["message"] = "Connecting to " + ssid + "...";
      request->send(200, "application/json", JSON.stringify(response));
    } else {
      JSONVar response;
      response["success"] = false;
      response["message"] = "Missing SSID";
      request->send(400, "application/json", JSON.stringify(response));
    }
  }

  void handleWiFiClear(AsyncWebServerRequest *request) {
    WiFiManager::clearCredentials();
    
    JSONVar response;
    response["success"] = true;
    response["message"] = "WiFi credentials cleared";
    request->send(200, "application/json", JSON.stringify(response));
  }

  void handleRFLearn(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    JSONVar json = JSON.parse((char*)data);
    
    if (json.hasOwnProperty("channel")) {
      int channel = (int)json["channel"];
      
      if (channel >= 0 && channel <= 5) {
        if (RF::isLearning()) {
          JSONVar response;
          response["success"] = false;
          response["message"] = "Learning already in progress";
          request->send(400, "application/json", JSON.stringify(response));
          return;
        }
        
        bool success = RF::startLearning(channel);
        
        JSONVar response;
        response["success"] = success;
        response["message"] = success ? 
          "Learning started for channel " + String(channel) + ". Press remote button within 30 seconds." :
          "Failed to start learning for channel " + String(channel);
        
        request->send(200, "application/json", JSON.stringify(response));
      } else {
        JSONVar response;
        response["success"] = false;
        response["message"] = "Invalid channel number";
        request->send(400, "application/json", JSON.stringify(response));
      }
    } else {
      JSONVar response;
      response["success"] = false;
      response["message"] = "Missing channel";
      request->send(400, "application/json", JSON.stringify(response));
    }
  }

  void handleRFClear(AsyncWebServerRequest *request) {
    bool success = RF::clearAll();
    
    JSONVar response;
    response["success"] = success;
    response["message"] = success ? "All RF codes cleared" : "Failed to clear RF codes";
    request->send(200, "application/json", JSON.stringify(response));
  }
}

namespace WebServer {

void begin(Preferences* prefs) {
  g_prefs = prefs;
  g_server = new AsyncWebServer(80);
  
  // Root page
  g_server->on("/", HTTP_GET, handleIndex);
  
  // Simple connectivity test
  g_server->on("/ping", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "pong");
  });
  
  // Device discovery endpoint
  g_server->on("/discover", HTTP_GET, [](AsyncWebServerRequest *request){
    JSONVar response;
    response["device"] = "TLTB-Mini";
    response["ip"] = WiFi.localIP().toString();
    response["hostname"] = "tltb-mini.local";
    response["mac"] = WiFi.macAddress();
    response["ap_ssid"] = WiFiManager::getAPSSID();
    request->send(200, "application/json", JSON.stringify(response));
  });
  
  // Diagnostic page
  g_server->on("/info", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body><h1>TLTB Mini Diagnostics</h1>";
    html += "<p><strong>WiFi Mode:</strong> " + String(WiFi.getMode()) + "</p>";
    html += "<p><strong>WiFi Status:</strong> " + String(WiFi.status()) + "</p>";
    html += "<p><strong>AP IP:</strong> " + WiFi.softAPIP().toString() + "</p>";
    html += "<p><strong>STA IP:</strong> " + WiFi.localIP().toString() + "</p>";
    html += "<p><strong>MAC:</strong> " + WiFi.macAddress() + "</p>";
    html += "<p><strong>Free Heap:</strong> " + String(ESP.getFreeHeap()) + "</p>";
    html += "<p><a href='/'>Back to Main</a></p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });
  
  // API endpoints
  g_server->on("/api/status", HTTP_GET, handleAPIStatus);
  g_server->on("/api/wifi/clear", HTTP_POST, handleWiFiClear);
  g_server->on("/api/rf/clear", HTTP_POST, handleRFClear);
  
  // POST endpoints with body
  g_server->onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (request->url() == "/api/wifi/connect") {
      handleWiFiConnect(request, data, len, index, total);
    } else if (request->url() == "/api/rf/learn") {
      handleRFLearn(request, data, len, index, total);
    }
  });
  
  // 404 handler
  g_server->onNotFound(handleNotFound);
  
  Serial.println("[WebServer] Initialized");
}

void start() {
  if (g_server && !g_serverRunning) {
    g_server->begin();
    g_serverRunning = true;
    Serial.println("[WebServer] Started on port 80");
    Serial.printf("[WebServer] Access at: http://%s\n", WiFi.localIP().toString().c_str());
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
      Serial.printf("[WebServer] AP mode access: http://%s\n", WiFi.softAPIP().toString().c_str());
    }
  }
}

void stop() {
  if (g_server && g_serverRunning) {
    g_server->end();
    g_serverRunning = false;
    Serial.println("[WebServer] Stopped");
  }
}

bool isRunning() {
  return g_serverRunning;
}

void service() {
  // Handle any background tasks if needed
  // The AsyncWebServer handles requests automatically
}

}