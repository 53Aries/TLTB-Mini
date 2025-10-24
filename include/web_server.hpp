#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

namespace WebServer {
  // Initialize web server
  void begin(Preferences* prefs);
  
  // Start the web server
  void start();
  
  // Stop the web server
  void stop();
  
  // Check if server is running
  bool isRunning();
  
  // Service routine (if needed for any background tasks)
  void service();
}