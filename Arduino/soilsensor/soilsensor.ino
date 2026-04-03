#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <DHTesp.h>

// ----------- WiFi Info -----------
const char* ssid = "thuchanhAT T5";
const char* password = "12345679";

// ----------- Node-RED URL -----------
const char* serverName = "http://172.19.248.29:1880/dht11";         // Gửi sensor data
const char* relayStateURL = "http://172.19.248.29:1880/relay_state"; // Gửi trạng thái relay

// ----------- GPIO Pinout -----------
#define DHT_PIN      14
#define RELAY1_PIN    5
#define RELAY2_PIN    4
#define RELAY3_PIN   19
#define SOIL_PIN     34
#define BTN1_PIN     18
#define BTN2_PIN     15
#define BTN3_PIN     16
#define KICK_PIN     32

// ----------- Biến toàn cục -----------
DHTesp dht;
WebServer server(80);

bool testRelay1 = false;
bool testRelay2 = false;
bool testRelay3 = false;

bool relay1State = false;
bool relay2State = false;
bool relay3State = false;

unsigned long lastSensorPrint = 0;
unsigned long lastSendTime = 0;
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime3 = 0;

const unsigned long debounceDelay = 50;
const unsigned long printInterval = 2000;
const unsigned long sendInterval = 5000;

// ----------- Hàm gửi trạng thái relay về Node-RED -----------
void sendRelayStateToNodeRED(int relayID, bool state) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(relayStateURL);
    http.addHeader("Content-Type", "application/json");

    String json = "{\"relay\":" + String(relayID) + ",\"state\":" + String(state ? "true" : "false") + "}";
    int response = http.POST(json);
    Serial.printf("📤 Gửi trạng thái relay %d: %s → Phản hồi: %d\n", relayID, state ? "ON" : "OFF", response);
    http.end();
  }
}

// ----------- Xử lý điều khiển từ Node-RED -----------
void handleRelayControl(int relayNum, bool &testFlag) {
  String state = server.arg("state");
  if (state == "on") {
    testFlag = true;
    server.send(200, "text/plain", "Relay ON (thủ công)");
  } else if (state == "off") {
    testFlag = false;
    server.send(200, "text/plain", "Relay OFF (thủ công)");
  } else {
    server.send(400, "text/plain", "❌ Sai cú pháp! ?state=on|off");
  }
}

void handleRelay1Control() { handleRelayControl(1, testRelay1); }
void handleRelay2Control() { handleRelayControl(2, testRelay2); }
void handleRelay3Control() { handleRelayControl(3, testRelay3); }

// ----------- Đọc cảm biến đất -----------
int readSoil() {
  digitalWrite(KICK_PIN, HIGH);
  delay(50);
  int val = analogRead(SOIL_PIN);
  digitalWrite(KICK_PIN, LOW);
  return val;
}

// ----------- Xử lý nút nhấn vật lý -----------
void checkButton() {
  static bool lastBtn1 = LOW, lastBtn2 = LOW, lastBtn3 = LOW;
  bool currBtn1 = digitalRead(BTN1_PIN);
  bool currBtn2 = digitalRead(BTN2_PIN);
  bool currBtn3 = digitalRead(BTN3_PIN);
  unsigned long now = millis();

  if (currBtn1 != lastBtn1) {
lastDebounceTime1 = now;
    lastBtn1 = currBtn1;
  }
  if ((now - lastDebounceTime1) > debounceDelay && currBtn1 == HIGH) {
    testRelay1 = !testRelay1;
    Serial.printf("🔘 BTN1: đổi testRelay1 → %d\n", testRelay1);
    sendRelayStateToNodeRED(1, testRelay1);  // Gửi Node-RED
    while (digitalRead(BTN1_PIN) == HIGH);
  }

  if (currBtn2 != lastBtn2) {
    lastDebounceTime2 = now;
    lastBtn2 = currBtn2;
  }
  if ((now - lastDebounceTime2) > debounceDelay && currBtn2 == HIGH) {
    testRelay2 = !testRelay2;
    Serial.printf("🔘 BTN2: đổi testRelay2 → %d\n", testRelay2);
    sendRelayStateToNodeRED(2, testRelay2);  // Gửi Node-RED
    while (digitalRead(BTN2_PIN) == HIGH);
  }

  if (currBtn3 != lastBtn3) {
    lastDebounceTime3 = now;
    lastBtn3 = currBtn3;
  }
  if ((now - lastDebounceTime3) > debounceDelay && currBtn3 == HIGH) {
    testRelay3 = !testRelay3;
    Serial.printf("🔘 BTN3: đổi testRelay3 → %d\n", testRelay3);
    sendRelayStateToNodeRED(3, testRelay3);  // Gửi Node-RED
    while (digitalRead(BTN3_PIN) == HIGH);
  }
}

// ----------- Setup -----------
void setup() {
  Serial.begin(115200);
  dht.setup(DHT_PIN, DHTesp::DHT11);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(SOIL_PIN, INPUT);
  pinMode(KICK_PIN, OUTPUT);
  pinMode(BTN1_PIN, INPUT);
  pinMode(BTN2_PIN, INPUT);
  pinMode(BTN3_PIN, INPUT);

  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);
  digitalWrite(RELAY3_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("🔌 Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\n✅ Đã kết nối WiFi: %s\n", WiFi.localIP().toString().c_str());

  server.on("/relay1", handleRelay1Control);
  server.on("/relay2", handleRelay2Control);
  server.on("/relay3", handleRelay3Control);
  server.begin();
}

// ----------- Loop -----------
void loop() {
  server.handleClient();
  checkButton();

  unsigned long now = millis();
  TempAndHumidity data = dht.getTempAndHumidity();
  int soilRaw = readSoil();
  int soilMapped = map(soilRaw, 0, 4095, 0, 100);
  int soilMoisture = 100 - soilMapped;

  if (now - lastSensorPrint >= printInterval) {
    Serial.printf("🌡 Nhiệt độ: %.1f °C\n", data.temperature);
    Serial.printf("💧 Độ ẩm không khí: %.1f %%\n", data.humidity);
    Serial.printf("🌱 Độ ẩm đất: %d %%\n", soilMoisture);
    Serial.printf("🔌 Relay1: %d | Relay2: %d | Relay3: %d\n",
                  digitalRead(RELAY1_PIN), digitalRead(RELAY2_PIN), digitalRead(RELAY3_PIN));
    lastSensorPrint = now;
  }

  // Điều khiển relay
  relay1State = testRelay1 || data.temperature > 30;
  relay2State = testRelay2 || data.temperature > 33;
  relay3State = testRelay3;

  digitalWrite(RELAY1_PIN, relay1State ? HIGH : LOW);
  digitalWrite(RELAY2_PIN, relay2State ? HIGH : LOW);
digitalWrite(RELAY3_PIN, relay3State ? HIGH : LOW);

  // Gửi dữ liệu cảm biến lên Node-RED mỗi 5s
  if (now - lastSendTime >= sendInterval) {
    lastSendTime = now;

    if (!isnan(data.temperature) && !isnan(data.humidity) && WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");

      String jsonData = "{\"temperature\":" + String(data.temperature, 2) +
                        ",\"humidity\":" + String(data.humidity, 2) +
                        ",\"soilMoisture\":" + String(soilMoisture) + "}";

      int response = http.POST(jsonData);
      if (response > 0) {
        Serial.printf("📤 Gửi dữ liệu cảm biến: OK (%d)\n", response);
      } else {
        Serial.printf("❌ Gửi dữ liệu lỗi: %s\n", http.errorToString(response).c_str());
      }
      http.end();
    }
  }
}
