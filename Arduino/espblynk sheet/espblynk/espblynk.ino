#define BLYNK_TEMPLATE_ID   "TMPL6iCrstT2J"
#define BLYNK_TEMPLATE_NAME "controldht"
#define BLYNK_AUTH_TOKEN    "x1LCc8G53oHhmwLbRIAXy45odqBKA3lw"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <HTTPClient.h>

char ssid[] = "Dat";
char pass[] = "123456789";

#define DHTPIN 7     // ⚠️ kiểm tra lại GPIO7 có dùng được cho DHT không
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define SUNNY 10
#define BTN1 4
#define BTN2 8
#define LED1 5
#define LED2 11

bool led1State = false;
bool led2State = false;
bool autoLed1 = false;
bool autoLed2 = false;

float temp = 0, hum = 0;
int sun = 0;

BlynkTimer timer;

// 🌐 Google Apps Script Web App URL (sau khi deploy đúng)
const char* googleScriptURL = "https://script.google.com/macros/s/AKfycbyZN8VQd5Rg7NOerCb6o9is43P76lhmt7cEuj-XP5SehVYFn0b5q7FRXmpL33J1CIf4/exec";

// ===== Blynk nút điều khiển =====
BLYNK_WRITE(V2) {
  led1State = param.asInt();
  digitalWrite(LED1, led1State ? HIGH : LOW);
  Serial.printf("LED1 set from Blynk → %s\n", led1State ? "ON" : "OFF");
}

BLYNK_WRITE(V3) {
  led2State = param.asInt();
  digitalWrite(LED2, led2State ? HIGH : LOW);
  Serial.printf("LED2 set from Blynk → %s\n", led2State ? "ON" : "OFF");
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V2, V3);
  Serial.println("✅ ESP32 connected to Blynk server");
}

// ===== Đọc cảm biến & gửi Blynk =====
void sendSensorData() {
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  sun = 100 - map(analogRead(SUNNY), 0, 4095, 0, 100);

  if (!isnan(temp) && !isnan(hum)) {
    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, hum);
    Blynk.virtualWrite(V4, sun);
    Serial.printf("🌡 %.1f°C | 💧 %.1f%% | ☀️ %d%%\n", temp, hum, sun);
  } else {
    Serial.println("❌ Failed to read DHT11 sensor");
  }
}

// ===== Gửi dữ liệu lên Google Sheet =====
void sendGoogleData(float temp, float hum, int sun, bool led1, bool led2) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(googleScriptURL);
    http.addHeader("Content-Type", "application/json");

    String json = "{";
    json += "\"temperature\":" + String(temp, 2) + ",";
    json += "\"humidity\":" + String(hum, 2) + ",";
    json += "\"sunlight\":" + String(sun) + ",";
    json += "\"led1\":" + String(led1 ? "true" : "false") + ",";
    json += "\"led2\":" + String(led2 ? "true" : "false");
    json += "}";

    int code = http.POST(json);
    if (code > 0) {
      Serial.printf("📤 Sent to Google Sheet → Code: %d\n", code);
    } else {
      Serial.printf("❌ Error sending to Google Sheet: %s\n", http.errorToString(code).c_str());
    }
    http.end();
  }
}

void sendGoogleTask() {
  sendGoogleData(temp, hum, sun, led1State, led2State); 
}

// ===== Auto control =====
void autoControl() {
  if (!led1State) {
    if (temp > 33 && !autoLed1) {
      autoLed1 = true;
      digitalWrite(LED1, HIGH);
      Blynk.virtualWrite(V2, 1);
      Serial.println("⚡ Auto ON LED1 (temp > 33°C)");
    } else if (temp <= 33 && autoLed1) {
      autoLed1 = false;
      digitalWrite(LED1, LOW);
      Blynk.virtualWrite(V2, 0);
      Serial.println("⚡ Auto OFF LED1 (temp <= 33°C)");
    }
  }

  if (!led2State) {
    if (sun < 70 && !autoLed2) {
      autoLed2 = true;
      digitalWrite(LED2, HIGH);
      Blynk.virtualWrite(V3, 1);
      Serial.println("⚡ Auto ON LED2 (sun < 70%)");
    } else if (sun >= 70 && autoLed2) {
      autoLed2 = false;
      digitalWrite(LED2, LOW);
      Blynk.virtualWrite(V3, 0);
      Serial.println("⚡ Auto OFF LED2 (sun >= 70%)");
    }
  }
}

// ===== Nút nhấn =====
void checkButtons() {
  static bool lastBtn1 = HIGH, lastBtn2 = HIGH;
  static unsigned long lastDebounce1 = 0, lastDebounce2 = 0;
  const unsigned long debounceDelay = 50;
  bool btn1 = digitalRead(BTN1);
  bool btn2 = digitalRead(BTN2);
  unsigned long now = millis();

  if (btn1 != lastBtn1 && (now - lastDebounce1 > debounceDelay)) {
    if (btn1 == LOW) {
      led1State = !led1State;
      digitalWrite(LED1, led1State ? HIGH : LOW);
      Blynk.virtualWrite(V2, led1State);
      Serial.printf("🖲 BTN1 pressed → LED1 %s\n", led1State ? "ON" : "OFF");
    }
    lastDebounce1 = now;
  }
  lastBtn1 = btn1;

  if (btn2 != lastBtn2 && (now - lastDebounce2 > debounceDelay)) {
    if (btn2 == LOW) {
      led2State = !led2State;
      digitalWrite(LED2, led2State ? HIGH : LOW);
      Blynk.virtualWrite(V3, led2State);
      Serial.printf("🖲 BTN2 pressed → LED2 %s\n", led2State ? "ON" : "OFF");
    }
    lastDebounce2 = now;
  }
  lastBtn2 = btn2;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  dht.begin();

  Serial.println("🔌 Connecting to WiFi & Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendSensorData);  // Blynk update
  timer.setInterval(30000L, sendGoogleTask); // Google Sheet update
}

void loop() {
  Blynk.run();
  timer.run();
  checkButtons();
  autoControl();  
}
