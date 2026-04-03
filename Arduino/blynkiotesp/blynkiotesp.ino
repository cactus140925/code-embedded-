#define BLYNK_TEMPLATE_ID   "TMPL6iCrstT2J"   // ID lấy từ Device Info
#define BLYNK_TEMPLATE_NAME "controldht"     // Tên template
#define BLYNK_AUTH_TOKEN    "x1LCc8G53oHhmwLbRIAXy45odqBKA3lw" // Token trong Device Info

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHTesp.h>

// ==== Thông tin WiFi ====
char ssid[] = "Dat";
char pass[] = "123456789";

// ==== Cảm biến DHT11 ====
#define DHT_PIN 32
DHTesp dht;

// ==== LED điều khiển từ Blynk ====
#define LED_PIN 14
bool ledState = false;

// ====== Nhận lệnh từ Blynk (V2) ======
BLYNK_WRITE(V2) {
  int pinValue = param.asInt(); // 0 hoặc 1
  ledState = pinValue;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  Serial.printf("LED trạng thái: %s\n", ledState ? "ON" : "OFF");
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.setup(DHT_PIN, DHTesp::DHT11);

  // Kết nối Blynk IoT
  Serial.println("🔌 Đang kết nối WiFi...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("✅ Đã kết nối WiFi, IP: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  // Kiểm tra WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Mất kết nối WiFi! Đang thử kết nối lại...");
    WiFi.begin(ssid, pass);
    delay(2000);
    return; // bỏ qua các lệnh khác khi mất kết nối
  }

  Blynk.run(); // Chạy Blynk

  // Đọc dữ liệu cảm biến
  TempAndHumidity data = dht.getTempAndHumidity();
  if (!isnan(data.temperature) && !isnan(data.humidity)) {
    Serial.printf("🌡 %.1f°C | 💧 %.1f%%\n", data.temperature, data.humidity);
    Blynk.virtualWrite(V0, data.temperature);
    Blynk.virtualWrite(V1, data.humidity);
  } else {
    Serial.println("❌ Lỗi đọc DHT11");
  }

  delay(2000);
}
