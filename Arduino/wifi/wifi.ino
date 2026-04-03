#include <WiFi.h>
const char* ssid = "Dat";
const char* password = "123456789";
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
void setup() {

Serial.begin(115200);
}

void loop() {

  if(WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
}
