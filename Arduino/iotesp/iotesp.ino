#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <DHTesp.h>
#include <ArduinoJson.h>

// WiFi Info
const char* ssid = "Dat";
const char* password = "123456789";

// Node-RED URLs
const char* sensorDataURL = "http://172.19.248.19:1880/dht11";
const char* stateUpdateURL = "http://172.19.248.19:1880/updateRelayState";

// GPIO
#define DHT_PIN 14
#define RELAY1_PIN 5
#define RELAY2_PIN 4
#define RELAY3_PIN 19
#define SOIL_PIN 13
#define BTN1_PIN 18
#define BTN2_PIN 15
#define BTN3_PIN 16
#define KICK_PIN 32

DHTesp dht;
WebServer server(80);

// Trạng thái điều khiển
bool testRelay1 = false;
bool testRelay2 = false;
bool testRelay3 = false;
bool relay1State = false;
bool relay2State = false;
bool relay3State = false;

// Trạng thái trước đó để phát hiện thay đổi
bool prevRelay1State = false;
bool prevRelay2State = false;
bool prevRelay3State = false;

// Biến thời gian
unsigned long lastSensorPrint = 0;
unsigned long lastSendTime = 0;
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime3 = 0;
unsigned long lastStateUpdate = 0;

const unsigned long debounceDelay = 50;
const unsigned long printInterval = 2000;
const unsigned long sendInterval = 5000;
const unsigned long stateUpdateInterval = 1000; // Kiểm tra và gửi trạng thái mỗi 1s

// ===================== XỬ LÝ HTTP ===================== //
void handleRelay1Control() {
  String state = server.arg("state");
  if (state == "on") {
    testRelay1 = true;
    server.send(200, "text/plain", "Relay 1 ON (thủ công)");
  } else if (state == "off") {
    testRelay1 = false;
    server.send(200, "text/plain", "Relay 1 OFF (thủ công)");
  } else {
    server.send(400, "text/plain", "Sai cú pháp! Sử dụng ?state=on|off");
    return;
  }
  updateRelayStates();
}

void handleRelay2Control() {
  String state = server.arg("state");
  if (state == "on") {
    testRelay2 = true;
    server.send(200, "text/plain", "Relay 2 ON (thủ công)");
  } else if (state == "off") {
    testRelay2 = false;
    server.send(200, "text/plain", "Relay 2 OFF (thủ công)");
  } else {
    server.send(400, "text/plain", "Sai cú pháp! Sử dụng ?state=on|off");
    return;
  }
  updateRelayStates();
}

void handleRelay3Control() {
  String state = server.arg("state");
  if (state == "on") {
    testRelay3 = true;
    server.send(200, "text/plain", "Relay 3 ON (thủ công)");
  } else if (state == "off") {
    testRelay3 = false;
    server.send(200, "text/plain", "Relay 3 OFF (thủ công)");
  } else {
    server.send(400, "text/plain", "Sai cú pháp! Sử dụng ?state=on|off");
    return;
  }
  updateRelayStates();
}

// ===================== SETUP ===================== //
void setup() {
  Serial.begin(115200);
  dht.setup(DHT_PIN, DHTesp::DHT11);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
pinMode(SOIL_PIN, INPUT);
  pinMode(KICK_PIN, OUTPUT);

  pinMode(BTN1_PIN, INPUT_PULLUP); // Thêm pullup resistor
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);

  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);
  digitalWrite(RELAY3_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("\nĐang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Đã kết nối WiFi!");
  Serial.printf("🌐 Điều khiển Relay 1: http://%s/relay1?state=on|off\n", WiFi.localIP().toString().c_str());
  Serial.printf("🌐 Điều khiển Relay 2: http://%s/relay2?state=on|off\n", WiFi.localIP().toString().c_str());
  Serial.printf("🌐 Điều khiển Relay 3: http://%s/relay3?state=on|off\n", WiFi.localIP().toString().c_str());

  server.on("/relay1", handleRelay1Control);
  server.on("/relay2", handleRelay2Control);
  server.on("/relay3", handleRelay3Control);
  server.begin();
}

// ===================== CÁC HÀM HỖ TRỢ ===================== //
int readSoilSensor() {
  digitalWrite(KICK_PIN, HIGH);
  delay(10);
  int val = analogRead(SOIL_PIN);
  digitalWrite(KICK_PIN, LOW);
  return val;
}

void sendRelayState() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(stateUpdateURL);
    http.addHeader("Content-Type", "application/json");

    // Tạo JSON với ArduinoJson để đảm bảo format đúng
    StaticJsonDocument<200> doc;
    doc["relay1"] = relay1State;
    doc["relay2"] = relay2State;
    doc["relay3"] = relay3State;
    doc["source"] = "esp32"; // Thêm source để Node-RED biết là từ ESP32
    
    String jsonData;
    serializeJson(doc, jsonData);

    int code = http.POST(jsonData);
    if (code > 0) {
      Serial.printf("📤 Đã cập nhật trạng thái lên Node-RED | Mã phản hồi: %d\n", code);
      String response = http.getString();
      Serial.printf("📥 Phản hồi: %s\n", response.c_str());
    } else {
      Serial.printf("❌ Lỗi cập nhật trạng thái | %s\n", http.errorToString(code).c_str());
    }
    http.end();
  }
}

void updateRelayStates() {
  TempAndHumidity data = dht.getTempAndHumidity();
  
  // Logic điều khiển tự động
  relay1State = (testRelay1 || data.temperature > 30);
  relay2State = (testRelay2 || data.temperature > 33);
  relay3State = testRelay3;

  // Cập nhật phần cứng
  digitalWrite(RELAY1_PIN, relay1State ? HIGH : LOW);
  digitalWrite(RELAY2_PIN, relay2State ? HIGH : LOW);
  digitalWrite(RELAY3_PIN, relay3State ? HIGH : LOW);

  // Chỉ gửi khi có thay đổi trạng thái
  if (relay1State != prevRelay1State || 
      relay2State != prevRelay2State || 
      relay3State != prevRelay3State) {
    
    sendRelayState();
    
    // Cập nhật trạng thái trước đó
    prevRelay1State = relay1State;
    prevRelay2State = relay2State;
    prevRelay3State = relay3State;
  }
}

void checkButton() {
static bool lastBtn1 = HIGH; // HIGH vì dùng INPUT_PULLUP
  static bool lastBtn2 = HIGH;
  static bool lastBtn3 = HIGH;

  bool currentBtn1 = digitalRead(BTN1_PIN);
  bool currentBtn2 = digitalRead(BTN2_PIN);
  bool currentBtn3 = digitalRead(BTN3_PIN);
  unsigned long now = millis();

  // Button 1 - nhấn = LOW (do pullup)
  if ((currentBtn1 != lastBtn1) && (currentBtn1 == LOW) && (now - lastDebounceTime1 > debounceDelay)) {
    lastDebounceTime1 = now;
    testRelay1 = !testRelay1;
    Serial.printf("⚡ BTN1 pressed: testRelay1 => %d\n", testRelay1);
    updateRelayStates();
  }
  lastBtn1 = currentBtn1;

  // Button 2
  if ((currentBtn2 != lastBtn2) && (currentBtn2 == LOW) && (now - lastDebounceTime2 > debounceDelay)) {
    lastDebounceTime2 = now;
    testRelay2 = !testRelay2;
    Serial.printf("⚡ BTN2 pressed: testRelay2 => %d\n", testRelay2);
    updateRelayStates();
  }
  lastBtn2 = currentBtn2;

  // Button 3
  if ((currentBtn3 != lastBtn3) && (currentBtn3 == LOW) && (now - lastDebounceTime3 > debounceDelay)) {
    lastDebounceTime3 = now;
    testRelay3 = !testRelay3;
    Serial.printf("⚡ BTN3 pressed: testRelay3 => %d\n", testRelay3);
    updateRelayStates();
  }
  lastBtn3 = currentBtn3;
}

void sendSensorData() {
  TempAndHumidity data = dht.getTempAndHumidity();
  int soil = readSoilSensor();
  
  if (!isnan(data.temperature) && !isnan(data.humidity) && WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(sensorDataURL);
    http.addHeader("Content-Type", "application/json");

    // Sử dụng ArduinoJson để tạo JSON chính xác
    StaticJsonDocument<300> doc;
    doc["temperature"] = round(data.temperature * 100) / 100.0; // 2 chữ số thập phân
    doc["humidity"] = round(data.humidity * 100) / 100.0;
    doc["soilMoisture"] = soil;
    
    String jsonData;
    serializeJson(doc, jsonData);

    int code = http.POST(jsonData);
    if (code > 0) {
      Serial.printf("✅ Đã gửi DHT + Soil lên Node-RED | Phản hồi: %d\n", code);
    } else {
      Serial.printf("❌ Gửi sensor data thất bại | %s\n", http.errorToString(code).c_str());
    }
    http.end();
  }
}

// ===================== LOOP ===================== //
void loop() {
  server.handleClient();
  checkButton();

  unsigned long now = millis();
  TempAndHumidity data = dht.getTempAndHumidity();
  int soil = readSoilSensor();

  // In thông tin cảm biến
  if (now - lastSensorPrint >= printInterval) {
    lastSensorPrint = now;
    Serial.printf("🌡 Nhiệt độ: %.1f °C\n", data.temperature);
    Serial.printf("💧 Độ ẩm không khí: %.1f %%\n", data.humidity);
    Serial.printf("🌱 Độ ẩm đất (raw): %d\n", soil);
    Serial.printf("🔌 Relay 1: %d | Relay 2: %d | Relay 3: %d\n", relay1State, relay2State, relay3State);
    Serial.printf("🎛 Test modes - R1: %d | R2: %d | R3: %d\n", testRelay1, testRelay2, testRelay3);
  }

  // Gửi dữ liệu cảm biến
if (now - lastSendTime >= sendInterval) {
    lastSendTime = now;
    sendSensorData();
  }

  // Kiểm tra và cập nhật logic relay (không phụ thuộc vào button)
  updateRelayStates();
  
  delay(100); // Giảm tải CPU
}
