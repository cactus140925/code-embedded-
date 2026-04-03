#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

// --- CẤU HÌNH WIFI ---
const char* ssid = "Nha co rat nhieu cay 2020";     // <--- SỬA TÊN WIFI
const char* password = "phuongthaoquan@@";    // <--- SỬA PASS WIFI
#define UDP_PORT 4210                      // Cổng giao tiếp (giữ nguyên)

// --- CẤU HÌNH LED ---
#define PIN        48  // Chân LED RGB trên S3 (Sửa thành 38 hoặc 21 nếu ko sáng)
#define NUMPIXELS  1 
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

WiFiUDP udp;
char packetBuffer[255]; // Bộ đệm chứa dữ liệu nhận được

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.setBrightness(50);

  // Kết nối WiFi
  Serial.print("Dang ket noi WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // IN ĐỊA CHỈ IP RA MÀN HÌNH (QUAN TRỌNG)
  Serial.println("");
  Serial.print("Da ket noi! IP cua ESP32 la: ");
  Serial.println(WiFi.localIP()); 

  // Bắt đầu lắng nghe gói tin UDP
  udp.begin(UDP_PORT);
}

void loop() {
  // Kiểm tra xem có gói tin nào gửi đến không
  int packetSize = udp.parsePacket();
  if (packetSize) {
    // Đọc dữ liệu
    int len = udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0; // Kết thúc chuỗi

    // Chuyển chuỗi nhận được thành số
    int fingers = atoi(packetBuffer);
    
    // Đổi màu LED
    pixels.clear();
    switch (fingers) {
      case 1: pixels.setPixelColor(0, pixels.Color(255, 0, 0)); break; // Đỏ
      case 2: pixels.setPixelColor(0, pixels.Color(0, 255, 0)); break; // Xanh lá
      case 3: pixels.setPixelColor(0, pixels.Color(0, 0, 255)); break; // Xanh dương
      case 4: pixels.setPixelColor(0, pixels.Color(255, 255, 0)); break; // Vàng
      case 5: pixels.setPixelColor(0, pixels.Color(255, 0, 255)); break; // Tím
      default: pixels.clear(); break;
    }
    pixels.show();
  }
}