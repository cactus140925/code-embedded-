#include <DHT.h>
#include <HardwareSerial.h>

// --- CẤU HÌNH LORA ---
#define RX_PIN 4 
#define TX_PIN 5

// --- CẤU HÌNH CẢM BIẾN & ĐÈN ---
#define LED_PIN 7      // Chân đèn LED
#define DHT_PIN 3      // Chân data của DHT
#define LUX_PIN 15     // Chân Analog của Quang trở (LDR)

// Chọn loại cảm biến bạn đang dùng (bỏ comment dòng tương ứng)
#define DHTTYPE DHT22   // Nếu dùng DHT11 màu xanh dương
//#define DHTTYPE DHT22   // Nếu dùng DHT22 màu trắng

DHT dht(DHT_PIN, DHTTYPE);

// Biến lưu trạng thái
int ledState = LOW; 
unsigned long lastSendTime = 0;       // Lưu mốc thời gian gửi gần nhất
const unsigned long interval = 3000;  // Cứ 3000ms (3 giây) thì gửi 1 lần

void setup() {
  Serial.begin(115200);
  
  // Khởi động LoRa
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // Khởi động các chân và cảm biến
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);
  
  pinMode(LUX_PIN, INPUT); // Chân Analog đọc quang trở
  dht.begin();             // Khởi động DHT
  
  Serial.println("--- ESP32 MULTI-TASK READY ---");
}

void loop() {
  // ============================================================
  // NHIỆM VỤ 1: LẮNG NGHE LỆNH TỪ ARDUINO (ĐỂ BẬT/TẮT ĐÈN)
  // ============================================================
  if (Serial1.available()) {
    String msg = Serial1.readStringUntil('\n');
    msg.trim();
    
    if (msg == "1") {
      ledState = !ledState; // Đảo trạng thái
      digitalWrite(LED_PIN, ledState);
      
      // Phản hồi nhanh để báo đã nhận lệnh
      Serial.print("Nhan lenh tu Arduino -> Den hien tai: ");
      Serial.println(ledState ? "ON" : "OFF");
    }
  }

  // ============================================================
  // NHIỆM VỤ 2: ĐỌC CẢM BIẾN VÀ GỬI ĐI (MỖI 3 GIÂY 1 LẦN)
  // ============================================================
  // Dùng millis() thay vì delay() để không bị treo máy
  if (millis() - lastSendTime > interval) {
    lastSendTime = millis(); // Cập nhật thời gian
    
    // 1. Đọc dữ liệu
    float t = dht.readTemperature(); // Đọc nhiệt độ
    float h = dht.readHumidity();    // Đọc độ ẩm (nếu cần)
    int luxVal = analogRead(LUX_PIN);// Đọc quang trở (0 - 4095)
    
    // Kiểm tra cảm biến có bị lỗi không
    if (isnan(t) || isnan(h)) {
      Serial.println("Loi doc DHT!");
      return;
    }

    // 2. Đóng gói tin nhắn gửi đi
    // Định dạng: "Nhiet:28.5|Lux:2048|Den:1"
    String goi_tin = "Nhiet:" + String(t, 1) + 
                     "|Lux:" + String(luxVal) + 
                     "|Den:" + String(ledState);
    
    // 3. Gửi qua LoRa sang Arduino
    Serial1.println(goi_tin);
    
    // 4. In ra màn hình ESP32 để kiểm tra
    Serial.print("Da gui di: ");
    Serial.println(goi_tin);
  }
}