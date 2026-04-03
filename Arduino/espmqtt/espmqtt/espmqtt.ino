#include <PubSubClient.h>
#include <WiFi.h>
#include <DHTesp.h>

#define DHTPIN 7
#define DHTTYPE DHTesp::DHT11
DHTesp dht;

#define SUNNY 10
#define BTN1 4
#define BTN2 8
#define LED1 5
#define LED2 11

const char* ssid = "Dat";
const char* password = "123456789";

// MQTT settings
const char* mqtt_server = "10.50.237.106";
const int mqtt_port = 1883;
const char* mqtt_client_id = "ESP32-Relays";
const char* relayTopicSub = "esp32/relay/control";
const char* relayTopicPub = "esp32/relay/state";

WiFiClient espClient;
PubSubClient client(espClient);

TempAndHumidity data;
int sun = 0;
bool led1State = false;
bool led2State = false;
bool autoLed1 = false;
bool autoLed2 = false;

unsigned long lastTime = 0;
const int timeset = 2000;

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("🔌 Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Đã kết nối WiFi!");
  Serial.print("📡 IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("🔄 Kết nối MQTT...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("✅ MQTT đã kết nối");
      client.subscribe(relayTopicSub);
    } else {
      Serial.print("❌ Thất bại: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void sendMQTTData() {
  String payload = "{\"temperature\":" + String(data.temperature, 2) +
                   ",\"humidity\":" + String(data.humidity, 2) +
                   ",\"sunlight\":" + String(sun) +
                   ",\"led1\":" + String(led1State ? "true" : "false") +
                   ",\"led2\":" + String(led2State ? "true" : "false") + "}";
  client.publish(relayTopicPub, payload.c_str(), true);
  Serial.println("📤 Đã gửi dữ liệu MQTT: " + payload);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.printf("📩 Nhận MQTT [%s]: %s\n", topic, message.c_str());

  // Parse JSON thủ công
  if (String(topic) == relayTopicSub) {
    led1State = message.indexOf("\"led1\":true") != -1;
    led2State = message.indexOf("\"led2\":true") != -1;
    digitalWrite(LED1, led1State ? HIGH : LOW);
    digitalWrite(LED2, led2State ? HIGH : LOW);
    Serial.printf("🖲 LED1 set to %s, LED2 set to %s from MQTT\n", 
                  led1State ? "ON" : "OFF", led2State ? "ON" : "OFF");
  }
}

void checkButtons() {
  static bool lastBtn1 = HIGH, lastBtn2 = HIGH;
  static unsigned long lastDebounce1 = 0, lastDebounce2 = 0;
  const unsigned long debounceDelay = 50;
  unsigned long now = millis();

  bool btn1 = digitalRead(BTN1);
  bool btn2 = digitalRead(BTN2);

  if (btn1 != lastBtn1 && (now - lastDebounce1 > debounceDelay)) {
    if (btn1 == LOW) {
      led1State = !led1State;
      digitalWrite(LED1, led1State ? HIGH : LOW);
      Serial.printf("🖲 BTN1 pressed → LED1 %s\n", led1State ? "ON" : "OFF");
      sendMQTTData();
    }
    lastDebounce1 = now;
  }
  lastBtn1 = btn1;

  if (btn2 != lastBtn2 && (now - lastDebounce2 > debounceDelay)) {
    if (btn2 == LOW) {
      led2State = !led2State;
      digitalWrite(LED2, led2State ? HIGH : LOW);
      Serial.printf("🖲 BTN2 pressed → LED2 %s\n", led2State ? "ON" : "OFF");
      sendMQTTData();
    }
    lastDebounce2 = now;
  }
  lastBtn2 = btn2;
}

void autoControl() {
  if (!led1State) {
    if (data.temperature > 33 && !autoLed1) {
      autoLed1 = true;
      led1State = true;
      digitalWrite(LED1, HIGH);
      Serial.println("⚡ Auto ON LED1 (temp > 33°C)");
      sendMQTTData();
    } else if (data.temperature <= 33 && autoLed1) {
      autoLed1 = false;
      led1State = false;
      digitalWrite(LED1, LOW);
      Serial.println("⚡ Auto OFF LED1 (temp <= 33°C)");
      sendMQTTData();
    }
  }

  if (!led2State) {
    if (sun < 70 && !autoLed2) {
      autoLed2 = true;
      led2State = true;
      digitalWrite(LED2, HIGH);
      Serial.println("⚡ Auto ON LED2 (sun < 70%)");
      sendMQTTData();
    } else if (sun >= 70 && autoLed2) {
      autoLed2 = false;
      led2State = false;
      digitalWrite(LED2, LOW);
      Serial.println("⚡ Auto OFF LED2 (sun >= 70%)");
      sendMQTTData();
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.setup(DHTPIN, DHTTYPE);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(SUNNY, INPUT);

  connectToWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}

void loop() {
  if (!client.connected()) mqttReconnect();
  client.loop();

  unsigned long now = millis();

  // Đọc dữ liệu cảm biến
  data = dht.getTempAndHumidity();
  sun = 100 - map(analogRead(SUNNY), 0, 4095, 0, 100);

  if (now - lastTime >= timeset) {
    if (!isnan(data.temperature) && !isnan(data.humidity)) {
      Serial.printf("🌡 %.1f°C | 💧 %.1f%% | ☀️ %d%%\n", data.temperature, data.humidity, sun);
      sendMQTTData();
    } else {
      Serial.println("❌ Failed to read DHT11 sensor");
    }
    lastTime = now;
  }

  checkButtons();
  autoControl();
}