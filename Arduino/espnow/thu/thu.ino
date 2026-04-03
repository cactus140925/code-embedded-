#include <esp_now.h>
#include <WiFi.h>

// Chân LED trên S3 (Sửa lại nếu cần)
#define LED_PIN 4

// Cấu trúc dữ liệu (Phải khớp 100% với bên C3)
typedef struct struct_message {
  bool ledStatus;
} struct_message;

struct_message myData;

// Hàm nhận dữ liệu (Viết cho bản Core 2.0.17)
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  // Kiểm tra kích thước dữ liệu
  if (len != sizeof(myData)) {
    Serial.println("Lỗi: Sai cấu trúc dữ liệu!");
    return;
  }

  memcpy(&myData, incomingData, sizeof(myData));
  
  Serial.print("Đã nhận lệnh: ");
  if (myData.ledStatus) {
    Serial.println("BẬT");
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println("TẮT");
    digitalWrite(LED_PIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Lỗi khởi tạo ESP-NOW");
    return;
  }
  
  // Đăng ký hàm nhận
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("S3 Đang chờ dữ liệu...");
  Serial.print("MAC Address cua S3: ");
  Serial.println(WiFi.macAddress()); // In MAC ra để bạn kiểm tra
}

void loop() {}