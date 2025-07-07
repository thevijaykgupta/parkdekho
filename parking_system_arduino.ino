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

  // CSS
  html += "<style>";
  html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #f0f2f5; }";
  html += ".container { max-width: 1200px; margin: 0 auto; padding: 20px; }";
  html += ".header { background: linear-gradient(135deg, #2E8B57, #1a5c3a); color: white; padding: 30px; text-align: center; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }";
  html += ".header h1 { font-size: 3em; margin-bottom: 10px; text-shadow: 2px 2px 4px rgba(0,0,0,0.2); }";
  html += ".header p { font-size: 1.4em; opacity: 0.9; }";
  html += ".status-container { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; margin: 30px 0; }";
  html += ".status-box { background: white; padding: 25px; border-radius: 15px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); text-align: center; transition: transform 0.3s ease; }";
  html += ".status-box:hover { transform: translateY(-5px); }";
  html += ".status-box h2 { color: #2E8B57; margin-bottom: 15px; font-size: 1.5em; }";
  html += ".status-box p { font-size: 2em; color: #333; font-weight: bold; }";
  html += ".wifi-status { display: flex; align-items: center; justify-content: center; gap: 10px; }";
  html += ".connected { color: #28a745; }";
  html += ".disconnected { color: #dc3545; }";
  html += ".parking-diagram { background: white; padding: 20px; border-radius: 15px; margin: 20px 0; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }";
  html += ".parking-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; margin: 20px 0; }";
  html += ".parking-slot { padding: 15px; border-radius: 10px; text-align: center; font-weight: bold; transition: all 0.3s ease; }";
  html += ".slot-available { background: #d4edda; color: #155724; }";
  html += ".slot-occupied { background: #f8d7da; color: #721c24; }";
  html += ".stats-container { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin: 20px 0; }";
  html += ".stat-box { background: white; padding: 20px; border-radius: 15px; text-align: center; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }";
  html += ".stat-value { font-size: 2em; color: #2E8B57; font-weight: bold; }";
  html += ".stat-label { color: #666; margin-top: 5px; }";
  html += ".apology { background: #fff3cd; color: #856404; padding: 25px; border-radius: 15px; margin: 20px 0; text-align: center; display: none; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }";
  html += ".apology.show { display: block; animation: fadeIn 0.5s; }";
  html += "@keyframes fadeIn { from { opacity: 0; transform: translateY(-20px); } to { opacity: 1; transform: translateY(0); } }";
  html += "</style>";

  html += "</head><body>";
  html += "<div class='header'><h1>ParkDekho</h1><p>Smart Parking Management System</p></div>";
  html += "<div class='container'>";

  // Status Cards
  html += "<div class='status-container'>";
  html += "<div class='status-box'><h2>Available Slots</h2><p id='slots'>" + vacantStr + "</p></div>";

  // Fix: Wi-Fi status line split up correctly
  String wifiClass = (connectionStatus == "Connected") ? "connected" : "disconnected";
  html += "<div class='status-box'><h2>Connection Status</h2><div class='wifi-status'><p class='" + wifiClass + "'>" + connectionStatus + "</p></div></div>";

  html += "<div class='status-box'><h2>Device IP</h2><p>" + ipAddress + "</p></div>";
  html += "</div>";

  // Parking Diagram
  html += "<div class='parking-diagram'><h2>Parking Layout</h2><div class='parking-grid' id='parkingGrid'>";
  for (int i = 1; i <= 3; i++) {
    html += "<div class='parking-slot slot-available'>P" + String(i) + "</div>";
  }
  html += "</div></div>";

  // Stats
  html += "<div class='stats-container'>";
  html += "<div class='stat-box'><div class='stat-value'>" + vacantStr + "</div><div class='stat-label'>Available Slots</div></div>";
  html += "<div class='stat-box'><div class='stat-value'>" + String(3 - vacantStr.toInt()) + "</div><div class='stat-label'>Occupied Slots</div></div>";
  html += "<div class='stat-box'><div class='stat-value'>" + String((vacantStr.toInt() * 100) / 3) + "%</div><div class='stat-label'>Availability Rate</div></div>";
  html += "</div>";

  // Apology Message
  html += "<div class='apology' id='apology'><h3>We're Sorry!</h3><p>All parking slots are currently occupied. Please try again later.</p></div>";

  // JavaScript
  html += "<script>";
  html += "function updateParkingStatus() {";
  html += "const slots = document.getElementById('slots').innerText;";
  html += "const apology = document.getElementById('apology');";
  html += "const parkingSlots = document.querySelectorAll('.parking-slot');";
  html += "const availableSlots = parseInt(slots);";
  html += "if (availableSlots === 0) {";
  html += "apology.classList.add('show');";
  html += "parkingSlots.forEach(slot => slot.classList.add('slot-occupied'));";
  html += "} else {";
  html += "apology.classList.remove('show');";
  html += "parkingSlots.forEach((slot, index) => {";
  html += "if (index < availableSlots) { slot.classList.add('slot-available'); slot.classList.remove('slot-occupied'); }";
  html += "else { slot.classList.add('slot-occupied'); slot.classList.remove('slot-available'); }";
  html += "});";
  html += "}";
  html += "}";
  html += "updateParkingStatus();";
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