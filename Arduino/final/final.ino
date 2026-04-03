#include <HTTPClient.h>
#include <PubSubClient.h>
#include <DHTesp.h>

// WiFi Info
const char* ssid = "Dat";
const char* password = "123456789";

// HTTP (sensor)
const char* sensorDataURL = "http://172.20.10.3:1880/dht11";

// MQTT
const char* mqtt_server = "172.20.10.3";
const int mqtt_port = 1883;
const char* mqtt_client_id = "ESP32-Relays";
const char* relayTopicSub = "esp32/relay/control";
const char* relayTopicPub = "esp32/relay/state";

// GPIO
#define DHT_PIN 14
#define RELAY1_PIN 5
#define RELAY2_PIN 4
#define RELAY3_PIN 19
#define SOIL_PIN 34
#define QUANG 32
#define KICK_PIN 32
#define BTN1_PIN 18
#define BTN2_PIN 15
#define BTN3_PIN 16
#define BUZZ 22 

WiFiClient espClient;
PubSubClient mqttClient(espClient);
DHTesp dht;

// Trạng thái điều khiển
bool testRelay1 = false;
bool testRelay2 = false;
bool testRelay3 = false;
bool relay1State = false;
bool relay2State = false;
bool relay3State = false;

// Cảm biến
TempAndHumidity data;
int soilMoisture = 0;
int light = 0;

// Thời gian
unsigned long lastSendTime = 0;
unsigned long lastSensorPrint = 0;
const unsigned long printInterval = 2000;
const unsigned long sendInterval = 5000;

// Button debounce
const unsigned long debounceDelay = 50;
unsigned long lastDebounce1 = 0, lastDebounce2 = 0, lastDebounce3 = 0;

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("🔌 Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ Đã kết nối WiFi!");
}

void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("🔄 Kết nối MQTT...");
    if (mqttClient.connect(mqtt_client_id)) {
      Serial.println("✅ MQTT đã kết nối");
      mqttClient.subscribe(relayTopicSub);
    } else {
      Serial.print("❌ Thất bại: ");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

void sendRelayState() {
  String payload = "{\"relay1\":" + String(testRelay1 ? "true" : "false") +
                   ",\"relay2\":" + String(testRelay2 ? "true" : "false") +
                   ",\"relay3\":" + String(testRelay3 ? "true" : "false") + "}";
  mqttClient.publish(relayTopicPub, payload.c_str(), true);
  Serial.println("📤 Đã gửi trạng thái relay: " + payload);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) message += (char)payload[i];

  Serial.printf("📩 Nhận MQTT [%s]: %s\n", topic, message.c_str());

  // Parse JSON thủ công
  testRelay1 = message.indexOf("\"relay1\":true") != -1;
  testRelay2 = message.indexOf("\"relay2\":true") != -1;
  testRelay3 = message.indexOf("\"relay3\":true") != -1;
}

int readSoil() {
  digitalWrite(KICK_PIN, HIGH);
  delay(10);
  int val = analogRead(SOIL_PIN);
  digitalWrite(KICK_PIN, LOW);
  return val;
}

int readQuang() {
  return map(analogRead(QUANG), 0, 4095, 0, 100);
}

void checkButtons() {
  static bool lastBtn1 = LOW, lastBtn2 = LOW, lastBtn3 = LOW;
  bool currBtn1 = digitalRead(BTN1_PIN);
  bool currBtn2 = digitalRead(BTN2_PIN);
  bool currBtn3 = digitalRead(BTN3_PIN);
  unsigned long now = millis();

  if (currBtn1 != lastBtn1) lastDebounce1 = now;
  if ((now - lastDebounce1) > debounceDelay && currBtn1 == HIGH) {
    testRelay1 = !testRelay1;
    Serial.printf("🔘 BTN1 → %d\n", testRelay1);
    sendRelayState();
    while (digitalRead(BTN1_PIN) == HIGH);
  }
  lastBtn1 = currBtn1;

  if (currBtn2 != lastBtn2) lastDebounce2 = now;
  if ((now - lastDebounce2) > debounceDelay && currBtn2 == HIGH) {
    testRelay2 = !testRelay2;
    Serial.printf("🔘 BTN2 → %d\n", testRelay2);
    sendRelayState();
    while (digitalRead(BTN2_PIN) == HIGH);
  }
  lastBtn2 = currBtn2;

  if (currBtn3 != lastBtn3) lastDebounce3 = now;
  if ((now - lastDebounce3) > debounceDelay && currBtn3 == HIGH) {
    testRelay3 = !testRelay3;
    Serial.printf("🔘 BTN3 → %d\n", testRelay3);
    sendRelayState();
    while (digitalRead(BTN3_PIN) == HIGH);
  }
  lastBtn3 = currBtn3;
}

void setup() {
  Serial.begin(115200);
  dht.setup(DHT_PIN, DHTesp::DHT11);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(SOIL_PIN, INPUT);
  pinMode(KICK_PIN, OUTPUT);
  pinMode(QUANG, INPUT);
  pinMode(BTN1_PIN, INPUT);
  pinMode(BTN2_PIN, INPUT);
  pinMode(BTN3_PIN, INPUT);
  pinMode(BUZZ,OUTPUT);
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);
  digitalWrite(RELAY3_PIN, LOW);
  digitalWrite(BUZZ,LOW);
  connectToWiFi();
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
}
void loop() {
  if (!mqttClient.connected()) mqttReconnect();
  mqttClient.loop();

  unsigned long now = millis();

  // Đọc dữ liệu cảm biến
  data = dht.getTempAndHumidity();
  soilMoisture = 100 - map(readSoil(), 0, 4095, 0, 100);
  light = 100- readQuang();

  checkButtons();
  // Tự động điều khiển (logic OR với manual)
  relay3State = testRelay3 || data.temperature > 35; 
  relay2State = testRelay2 || soilMoisture < 20;
  relay1State = testRelay1 || light <30;  
  digitalWrite(RELAY1_PIN, relay1State ? HIGH : LOW);
  digitalWrite(RELAY2_PIN, relay2State ? HIGH : LOW);
  digitalWrite(RELAY3_PIN, relay3State ? HIGH : LOW);
  bool shouldBuzz = data.temperature > 35 || soilMoisture < 20 || light < 30;

// Nếu vừa phát hiện điều kiện kích hoạt BUZZ
static unsigned long buzzerStartTime = 0;
static unsigned long lastBuzzEndTime = 0;
static bool buzzerActive = false;

if (shouldBuzz && !buzzerActive && millis() - lastBuzzEndTime >= 5000) {
  // Nếu đủ điều kiện và đã qua 10s kể từ lần kêu trước
  buzzerStartTime = millis();     // ghi lại thời điểm bắt đầu kêu
  buzzerActive = true;
  digitalWrite(BUZZ, HIGH);
}

// Nếu còi đang kêu và đã đủ 3 giây → tắt
if (buzzerActive && millis() - buzzerStartTime >= 3000) {
  buzzerActive = false;
  digitalWrite(BUZZ, LOW);
  lastBuzzEndTime = millis();     // ghi lại thời điểm kết thúc kêu
}

  if (now - lastSensorPrint >= printInterval) {
    Serial.printf("🌡 Nhiệt độ: %.1f°C | 💧 %.1f%% | 🌱 %d%% | 🔆 Ánh sáng: %d%%\n", data.temperature, data.humidity, soilMoisture, light);
    Serial.printf("🔌 Relay: %d %d %d\n", relay1State, relay2State, relay3State);
    lastSensorPrint = now;
  }

  // Gửi dữ liệu cảm biến qua HTTP
  if (now - lastSendTime >= sendInterval) {
    lastSendTime = now;
    if (!isnan(data.temperature) && !isnan(data.humidity)) {
      HTTPClient http;
      http.begin(sensorDataURL);
      http.addHeader("Content-Type", "application/json");

      String json = "{\"temperature\":" + String(data.temperature, 2) +
                    ",\"humidity\":" + String(data.humidity, 2) +
                    ",\"soilMoisture\":" + String(soilMoisture) +
                    ",\"light\":" + String(light) + "}";

      int code = http.POST(json);
      if (code > 0) 
      {
        Serial.printf("📤 Gửi HTTP cảm biến OK | Phản hồi: %d\n", code);
      }
      else Serial.printf("❌ Gửi HTTP lỗi: %s\n", http.errorToString(code).c_str());

      http.end();
    }
  }
}
