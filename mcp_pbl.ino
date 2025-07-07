#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

// Replace with your Wi-Fi credentials
const char* ssid = "BEAST";
const char* password = "Shri@Hari";

WebServer server(80);
String vacantStr = "Unknown";
String ipAddress = "Not connected";
String connectionStatus = "Connecting...";

// Parking slot states
struct ParkingSlot {
  bool isAvailable;
  String bookingId;
  String bookedTime;
  float price;
  String type;  // "car" or "bike"
};

ParkingSlot slots[10] = {
  {true, "", "", 100.0, "car"},  // P1
  {true, "", "", 100.0, "car"},  // P2
  {true, "", "", 100.0, "car"},  // P3
  {false, "NA", "NA", 80.0, "bike"},  // P4
  {false, "NA", "NA", 80.0, "bike"},  // P5
  {false, "NA", "NA", 80.0, "bike"},  // P6
  {false, "NA", "NA", 80.0, "bike"},  // P7
  {false, "NA", "NA", 80.0, "bike"},  // P8
  {false, "NA", "NA", 80.0, "bike"},  // P9
  {false, "NA", "NA", 80.0, "bike"}   // P10
};

// Store HTML template in PROGMEM to save RAM
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <meta http-equiv='refresh' content='3'>
    <title>ParkDekho - Smart Parking</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { 
            font-family: 'Segoe UI', sans-serif; 
            background: linear-gradient(135deg, #1a1a2e, #16213e);
            color: #fff;
            min-height: 100vh;
        }
        .container { 
            max-width: 1200px; 
            margin: 0 auto; 
            padding: 20px; 
        }
        .header { 
            background: linear-gradient(45deg, #00b4d8, #0077b6);
            padding: 30px;
            text-align: center;
            border-radius: 20px;
            margin-bottom: 30px;
            box-shadow: 0 10px 20px rgba(0,0,0,0.2);
        }
        .header h1 { 
            font-size: 3em; 
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .header p { 
            font-size: 1.2em;
            opacity: 0.9;
        }
        .status-container { 
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin: 20px 0;
        }
        .status-box { 
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            padding: 20px;
            border-radius: 15px;
            text-align: center;
            border: 1px solid rgba(255, 255, 255, 0.2);
            transition: transform 0.3s ease;
        }
        .status-box:hover {
            transform: translateY(-5px);
        }
        .parking-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 20px;
            margin: 30px 0;
        }
        .parking-slot {
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            padding: 20px;
            border-radius: 15px;
            text-align: center;
            border: 1px solid rgba(255, 255, 255, 0.2);
            transition: all 0.3s ease;
            position: relative;
            overflow: hidden;
        }
        .parking-slot::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: url('data:image/svg+xml,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="%23ffffff20"><path d="M18 4l2 4h-3l-2-4h-2l2 4h-3l-2-4H8l2 4H7L5 4H4c-1.1 0-1.99.9-1.99 2L2 18c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V4h-4z"/></svg>') center/50% no-repeat;
            opacity: 0.1;
        }
        .slot-available { 
            background: linear-gradient(45deg, #2ecc71, #27ae60);
            border: none;
        }
        .slot-occupied { 
            background: linear-gradient(45deg, #e74c3c, #c0392b);
            border: none;
        }
        .slot-number {
            font-size: 2em;
            font-weight: bold;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .slot-status {
            font-size: 1.2em;
            text-transform: uppercase;
            letter-spacing: 2px;
        }
        .apology {
            display: none;
            background: rgba(255, 193, 7, 0.2);
            backdrop-filter: blur(10px);
            padding: 20px;
            border-radius: 15px;
            margin: 20px 0;
            text-align: center;
            border: 1px solid rgba(255, 193, 7, 0.3);
        }
        .apology.show {
            display: block;
            animation: fadeIn 0.5s;
        }
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(-20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        .status-icon {
            font-size: 2em;
            margin-bottom: 10px;
        }
        .status-value {
            font-size: 1.5em;
            font-weight: bold;
            margin: 10px 0;
        }
        .status-label {
            font-size: 0.9em;
            opacity: 0.8;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
    </style>
</head>
<body>
    <div class='container'>
        <div class='header'>
            <h1>ParkDekho</h1>
            <p>Smart Parking Management System</p>
        </div>
        
        <div class='status-container'>
            <div class='status-box'>
                <div class='status-icon'>🚗</div>
                <div class='status-value' id='slots'>%SLOTS%</div>
                <div class='status-label'>Available Slots</div>
            </div>
            <div class='status-box'>
                <div class='status-icon'>📶</div>
                <div class='status-value'>%STATUS%</div>
                <div class='status-label'>Connection Status</div>
            </div>
            <div class='status-box'>
                <div class='status-icon'>🌐</div>
                <div class='status-value'>%IP%</div>
                <div class='status-label'>Device IP</div>
            </div>
        </div>

        <div class='parking-grid' id='parkingGrid'>
            %PARKING_SLOTS%
        </div>

        <div class='apology' id='apology'>
            <h3>Sorry! 🚫</h3>
            <p>All parking slots are currently occupied. Please try again later.</p>
        </div>
    </div>

    <script>
        function updateStatus() {
            const slots = document.getElementById('slots').innerText;
            const apology = document.getElementById('apology');
            const parkingSlots = document.querySelectorAll('.parking-slot');
            
            if (slots === '0') {
                apology.classList.add('show');
                parkingSlots.forEach(slot => {
                    slot.classList.add('slot-occupied');
                    slot.classList.remove('slot-available');
                    slot.querySelector('.slot-status').textContent = 'Occupied';
                });
            } else {
                apology.classList.remove('show');
                parkingSlots.forEach((slot, index) => {
                    if (index < parseInt(slots)) {
                        slot.classList.add('slot-available');
                        slot.classList.remove('slot-occupied');
                        slot.querySelector('.slot-status').textContent = 'Available';
                    } else {
                        slot.classList.add('slot-occupied');
                        slot.classList.remove('slot-available');
                        slot.querySelector('.slot-status').textContent = 'Occupied';
                    }
                });
            }
        }
        updateStatus();
    </script>
</body>
</html>
)rawliteral";

void handleRoot() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<meta http-equiv='refresh' content='3'>";
    html += "<title>ParkDekho - Smart Parking</title>";
    
    // Modern CSS with animations and better styling
    html += "<style>";
    html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
    html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #f0f2f5; }";
    html += ".container { max-width: 1400px; margin: 0 auto; padding: 20px; display: grid; grid-template-columns: 1fr 300px; gap: 20px; }";
    html += ".main-content { grid-column: 1; }";
    html += ".sidebar { grid-column: 2; }";
    html += ".header { background: linear-gradient(135deg, #2E8B57, #1a5c3a); color: white; padding: 30px; text-align: center; box-shadow: 0 4px 15px rgba(0,0,0,0.1); border-radius: 15px; margin-bottom: 20px; }";
    html += ".header h1 { font-size: 3em; margin-bottom: 10px; text-shadow: 2px 2px 4px rgba(0,0,0,0.2); }";
    html += ".header p { font-size: 1.4em; opacity: 0.9; }";
    html += ".logo { width: 100px; height: 100px; margin: 0 auto 20px; background: url('data:image/svg+xml,<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"white\"><path d=\"M18 4l2 4h-3l-2-4h-2l2 4h-3l-2-4H8l2 4H7L5 4H4c-1.1 0-1.99.9-1.99 2L2 18c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V4h-4z\"/></svg>') center/contain no-repeat; }";
    html += ".status-container { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; margin: 20px 0; }";
    html += ".status-box { background: white; padding: 25px; border-radius: 15px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); text-align: center; transition: transform 0.3s ease; }";
    html += ".status-box:hover { transform: translateY(-5px); }";
    html += ".parking-diagram { background: white; padding: 20px; border-radius: 15px; margin: 20px 0; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }";
    html += ".parking-grid { display: grid; grid-template-columns: repeat(5, 1fr); gap: 10px; margin: 20px 0; }";
    html += ".parking-slot { padding: 15px; border-radius: 10px; text-align: center; font-weight: bold; transition: all 0.3s ease; position: relative; }";
    html += ".slot-available { background: #d4edda; color: #155724; cursor: pointer; }";
    html += ".slot-occupied { background: #f8d7da; color: #721c24; }";
    html += ".slot-na { background: #e2e3e5; color: #383d41; }";
    html += ".booking-modal { display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.5); z-index: 1000; }";
    html += ".modal-content { background: white; width: 90%; max-width: 500px; margin: 50px auto; padding: 20px; border-radius: 15px; }";
    html += ".chatbot { position: fixed; bottom: 20px; right: 20px; background: white; padding: 15px; border-radius: 50%; box-shadow: 0 4px 15px rgba(0,0,0,0.1); cursor: pointer; }";
    html += ".chat-window { display: none; position: fixed; bottom: 80px; right: 20px; width: 300px; height: 400px; background: white; border-radius: 15px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }";
    html += ".price-tag { position: absolute; bottom: 5px; right: 5px; font-size: 0.8em; color: #666; }";
    html += ".vehicle-type { position: absolute; top: 5px; right: 5px; font-size: 0.8em; color: #666; }";
    html += ".map-container { height: 200px; background: #e9ecef; border-radius: 10px; margin: 20px 0; position: relative; }";
    html += ".map-container::before { content: 'Live Map View'; position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); color: #666; }";
    html += ".connected { color: #28a745; }";
    html += ".disconnected { color: #dc3545; }";
    html += "</style>";
    
    html += "</head><body>";
    
    // Header with Logo
    html += "<div class='header'>";
    html += "<div class='logo'></div>";
    html += "<h1>ParkDekho</h1>";
    html += "<p>Smart Parking Management System</p>";
    html += "</div>";
    
    html += "<div class='container'>";
    html += "<div class='main-content'>";
    
    // Status Boxes
    html += "<div class='status-container'>";
    html += "<div class='status-box'>";
    html += "<h2>Available Slots</h2>";
    html += "<p id='slots'>" + vacantStr + "</p>";
    html += "</div>";
    
    html += "<div class='status-box'>";
    html += "<h2>Connection Status</h2>";
    html += "<div class='wifi-status'>";
    String statusClass = (connectionStatus == "Connected") ? "connected" : "disconnected";
    html += "<p class='" + statusClass + "'>" + connectionStatus + "</p>";
    html += "</div>";
    html += "</div>";
    
    html += "<div class='status-box'>";
    html += "<h2>Device IP</h2>";
    html += "<p>" + ipAddress + "</p>";
    html += "</div>";
    html += "</div>";
    
    // Parking Diagram
    html += "<div class='parking-diagram'>";
    html += "<h2>Live Parking Layout</h2>";
    html += "<div class='parking-grid' id='parkingGrid'>";
    
    // Generate parking slots
    for(int i = 0; i < 10; i++) {
        String slotClass = slots[i].isAvailable ? "slot-available" : 
                          (i < 3 ? "slot-occupied" : "slot-na");
        html += "<div class='parking-slot " + slotClass + "' data-slot='" + String(i + 1) + "'>";
        html += "P" + String(i + 1);
        html += "<div class='vehicle-type'>" + slots[i].type + "</div>";
        html += "<div class='price-tag'>₹" + String(slots[i].price) + "/hr</div>";
        html += "</div>";
    }
    
    html += "</div>";
    html += "</div>";
    
    // Map Container
    html += "<div class='map-container'></div>";
    
    html += "</div>"; // End main-content
    
    // Sidebar
    html += "<div class='sidebar'>";
    
    // Booking Section
    html += "<div class='status-box'>";
    html += "<h2>Quick Booking</h2>";
    html += "<button onclick='showBookingModal()' style='width: 100%; padding: 10px; background: #2E8B57; color: white; border: none; border-radius: 5px; cursor: pointer;'>Book Now</button>";
    html += "</div>";
    
    // Price Rates
    html += "<div class='status-box'>";
    html += "<h2>Price Rates</h2>";
    html += "<p>Car Parking: ₹100/hr</p>";
    html += "<p>Bike Parking: ₹80/hr</p>";
    html += "<p>Daily Rate (Car): ₹800</p>";
    html += "<p>Daily Rate (Bike): ₹600</p>";
    html += "</div>";
    
    html += "</div>"; // End sidebar
    
    // Booking Modal
    html += "<div class='booking-modal' id='bookingModal'>";
    html += "<div class='modal-content'>";
    html += "<h2>Book Parking Slot</h2>";
    html += "<form id='bookingForm'>";
    html += "<input type='text' placeholder='Your Name' required style='width: 100%; padding: 10px; margin: 10px 0;'>";
    html += "<input type='tel' placeholder='Phone Number' required style='width: 100%; padding: 10px; margin: 10px 0;'>";
    html += "<select required style='width: 100%; padding: 10px; margin: 10px 0;'>";
    html += "<option value=''>Select Vehicle Type</option>";
    html += "<option value='car'>Car</option>";
    html += "<option value='bike'>Bike</option>";
    html += "</select>";
    html += "<input type='datetime-local' required style='width: 100%; padding: 10px; margin: 10px 0;'>";
    html += "<select required style='width: 100%; padding: 10px; margin: 10px 0;'>";
    html += "<option value=''>Select Duration</option>";
    html += "<option value='1'>1 Hour</option>";
    html += "<option value='2'>2 Hours</option>";
    html += "<option value='3'>3 Hours</option>";
    html += "<option value='4'>4 Hours</option>";
    html += "<option value='5'>5 Hours</option>";
    html += "</select>";
    html += "<button type='submit' style='width: 100%; padding: 10px; background: #2E8B57; color: white; border: none; border-radius: 5px; cursor: pointer;'>Confirm Booking</button>";
    html += "</form>";
    html += "</div>";
    html += "</div>";
    
    // Chatbot
    html += "<div class='chatbot'>💬</div>";
    html += "<div class='chat-window' id='chatWindow'>";
    html += "<div style='padding: 15px;'>";
    html += "<h3>ParkDekho Assistant</h3>";
    html += "<div id='chatMessages' style='height: 300px; overflow-y: auto;'></div>";
    html += "<input type='text' placeholder='Type your message...' style='width: 100%; padding: 10px; margin-top: 10px;'>";
    html += "</div>";
    html += "</div>";
    
    // JavaScript
    html += "<script>";
    html += "function showBookingModal() {";
    html += "  document.getElementById('bookingModal').style.display = 'block';";
    html += "}";
    
    html += "function updateParkingStatus() {";
    html += "  const slots = document.getElementById('slots').innerText;";
    html += "  const parkingSlots = document.querySelectorAll('.parking-slot');";
    html += "  const availableSlots = parseInt(slots);";
    
    html += "  parkingSlots.forEach((slot, index) => {";
    html += "    if (index < 3) {";
    html += "      if (index < availableSlots) {";
    html += "        slot.classList.add('slot-available');";
    html += "        slot.classList.remove('slot-occupied');";
    html += "      } else {";
    html += "        slot.classList.add('slot-occupied');";
    html += "        slot.classList.remove('slot-available');";
    html += "      }";
    html += "    } else {";
    html += "      slot.classList.add('slot-na');";
    html += "      slot.classList.remove('slot-available', 'slot-occupied');";
    html += "    }";
    html += "  });";
    html += "}";
    
    html += "document.querySelector('.chatbot').addEventListener('click', function() {";
    html += "  const chatWindow = document.getElementById('chatWindow');";
    html += "  chatWindow.style.display = chatWindow.style.display === 'none' ? 'block' : 'none';";
    html += "});";
    
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
            // Update slot states based on vacant count
            int vacantCount = vacantStr.toInt();
            for(int i = 0; i < 3; i++) {
                slots[i].isAvailable = (i < vacantCount);
            }
        }
    }
} 