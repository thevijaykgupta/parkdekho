#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Replace with your Wi-Fi credentials
const char* ssid = "BEAST";
const char* password = "Shri@Hari";

WebServer server(80);
String vacantStr = "Unknown";
String ipAddress = "Not connected";
String connectionStatus = "Connecting...";

void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='3'>";
  html += "<title>ParkDekho - Smart Parking</title>";
  
  // Modern CSS
  html += "<style>";
  html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #f0f2f5; }";
  html += ".container { max-width: 1200px; margin: 0 auto; padding: 20px; }";
  html += ".header { background: #2E8B57; color: white; padding: 20px; text-align: center; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }";
  html += ".header h1 { font-size: 2.5em; margin-bottom: 10px; }";
  html += ".header p { font-size: 1.2em; opacity: 0.9; }";
  html += ".status-container { display: flex; justify-content: center; gap: 20px; margin: 30px 0; flex-wrap: wrap; }";
  html += ".status-box { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); min-width: 250px; text-align: center; }";
  html += ".status-box h2 { color: #2E8B57; margin-bottom: 10px; }";
  html += ".status-box p { font-size: 1.5em; color: #333; }";
  html += ".apology { background: #fff3cd; color: #856404; padding: 20px; border-radius: 10px; margin: 20px 0; text-align: center; display: none; }";
  html += ".apology.show { display: block; animation: fadeIn 0.5s; }";
  html += "@keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }";
  html += ".wifi-status { display: flex; align-items: center; justify-content: center; gap: 10px; }";
  html += ".wifi-icon { width: 20px; height: 20px; }";
  html += ".connected { color: #28a745; }";
  html += ".disconnected { color: #dc3545; }";
  html += "</style>";
  
  html += "</head><body>";
  
  // Header
  html += "<div class='header'>";
  html += "<h1>ParkDekho</h1>";
  html += "<p>Smart Parking Management System</p>";
  html += "</div>";
  
  html += "<div class='container'>";
  
  // Status Boxes
  html += "<div class='status-container'>";
  
  // Available Slots
  html += "<div class='status-box'>";
  html += "<h2>Available Slots</h2>";
  html += "<p id='slots'>" + vacantStr + "</p>";
  html += "</div>";
  
  // Connection Status
  html += "<div class='status-box'>";
  html += "<h2>Connection Status</h2>";
  html += "<div class='wifi-status'>";
  html += "<span class='wifi-icon'>📶</span>";
  html += "<p class='" + (connectionStatus == "Connected" ? "connected" : "disconnected") + "'>" + connectionStatus + "</p>";
  html += "</div>";
  html += "</div>";
  
  // IP Address
  html += "<div class='status-box'>";
  html += "<h2>Device IP</h2>";
  html += "<p>" + ipAddress + "</p>";
  html += "</div>";
  
  html += "</div>";
  
  // Apology Message (shown when no slots are available)
  html += "<div class='apology' id='apology'>";
  html += "<h3>We're Sorry!</h3>";
  html += "<p>All parking slots are currently occupied. Please try again later or visit our alternative parking location.</p>";
  html += "</div>";
  
  // JavaScript to handle apology message
  html += "<script>";
  html += "function checkSlots() {";
  html += "  const slots = document.getElementById('slots').innerText;";
  html += "  const apology = document.getElementById('apology');";
  html += "  if (slots === '0' || slots === 'No slots available') {";
  html += "    apology.classList.add('show');";
  html += "  } else {";
  html += "    apology.classList.remove('show');";
  html += "  }";
  html += "}";
  html += "checkSlots();";
  html += "</script>";
  
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    ipAddress = WiFi.localIP().toString();
    connectionStatus = "Connected";
    Serial.println("\nWiFi connected!");
    Serial.println("IP address: http://" + ipAddress);
  } else {
    ipAddress = "Failed to connect";
    connectionStatus = "Failed";
    Serial.println("\nFailed to connect to WiFi.");
  }

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.startsWith("Vacant:")) {
      vacantStr = input.substring(7);
    }
  }
} 