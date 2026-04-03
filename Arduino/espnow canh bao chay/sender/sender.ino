#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "DHT.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// --- CẤU HÌNH PHẦN CỨNG ---
uint8_t broadcastAddress[] = {0x88,0x56,0xA6,0x6F,0x1C,0x20}; // MAC C3

#define DHTPIN 4
#define DHTTYPE DHT22 // Bạn đang dùng DHT22
#define MQ2_PIN 3
#define LED_PIN 9 

DHT dht(DHTPIN, DHTTYPE);

// Cấu trúc dữ liệu
typedef struct struct_message {
  float nhietdo;
  float doam;
  int khoi;
  bool canhbao;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

// --- KHAI BÁO BIẾN CHO FREERTOS ---
QueueHandle_t dataQueue; // Hàng đợi để chứa dữ liệu

// Hàm callback gửi (Giữ nguyên để không lỗi, nhưng ta không xử lý gì ở đây)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {}

// ================= TASK 1: ĐỌC CẢM BIẾN & ĐIỀU KHIỂN LED =================
// Task này sẽ chạy độc lập, cứ 2 giây đọc 1 lần
void TaskReadSensor(void *pvParameters) {
  struct_message tempData;
  
  for(;;) { // Vòng lặp vô tận thay cho loop()
    // 1. Đọc dữ liệu
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int gas = analogRead(MQ2_PIN);

    if (isnan(t)) t = 0.0;
    if (isnan(h)) h = 0.0;

    // 2. Logic đèn (Xử lý ngay lập tức)
    bool trang_thai_den = false;
    if(gas > 2500 || t > 29) { 
      trang_thai_den = true;
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }

    // 3. Đóng gói dữ liệu
    tempData.nhietdo = t;
    tempData.doam = h;
    tempData.khoi = gas;
    tempData.canhbao = trang_thai_den;

    // 4. Gửi vào Hàng đợi (Queue)
    // xQueueOverwrite: Ghi đè dữ liệu cũ, luôn đảm bảo gửi đi dữ liệu mới nhất
    xQueueOverwrite(dataQueue, &tempData);

    // Nghỉ 2000ms (Dùng vTaskDelay thay cho delay để không chặn hệ thống)
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

// ================= TASK 2: GỬI ESP-NOW ĐA KÊNH =================
// Task này chuyên trách việc gửi, không liên quan đến việc đọc cảm biến
void TaskSender(void *pvParameters) {
  struct_message dataToSend;
  
  for(;;) {
    // Chờ dữ liệu từ Queue. Nếu không có dữ liệu, Task này sẽ ngủ (Block) để tiết kiệm điện.
    // portMAX_DELAY nghĩa là chờ mãi mãi cho đến khi có dữ liệu mới.
    if (xQueueReceive(dataQueue, &dataToSend, portMAX_DELAY) == pdTRUE) {
      
      bool gui_duoc = false;
      
      // Thuật toán Channel Hunting (Quét 11 kênh)
      for (int channel = 1; channel <= 11; channel++) {
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
        
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &dataToSend, sizeof(dataToSend));
        
        if (result == ESP_OK) {
          gui_duoc = true;
        }
        // Nghỉ cực ngắn 10ms để chuyển kênh ổn định
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }

      if (gui_duoc) {
        Serial.printf(">> Core %d: Da gui data (Gas: %d) tren moi kenh.\n", xPortGetCoreID(), dataToSend.khoi);
      } else {
        Serial.println(">> Loi gui!");
      }
    }
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  
  // Khởi tạo phần cứng
  dht.begin();
  pinMode(MQ2_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Khởi tạo WiFi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 

  // Khởi tạo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Lỗi ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);

  // Đăng ký Peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0; 
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Lỗi Peer");
    return;
  }

  // --- TẠO HÀNG ĐỢI VÀ TASK FREERTOS ---
  
  // 1. Tạo hàng đợi chứa được 1 struct_message
  dataQueue = xQueueCreate(1, sizeof(struct_message));

  // 2. Tạo Task Đọc Cảm Biến
  // Stack 2048, Priority 1 (Thấp), Chạy ở Core 1 (App Core)
  xTaskCreatePinnedToCore(TaskReadSensor, "DocCamBien", 2048, NULL, 1, NULL, 1);

  // 3. Tạo Task Gửi Dữ Liệu
  // Stack 4096 (WiFi cần nhiều RAM), Priority 2 (Cao hơn), Chạy ở Core 0 (Pro Core - Chuyên xử lý Wifi)
  xTaskCreatePinnedToCore(TaskSender, "GuiDaKenh", 4096, NULL, 2, NULL, 0);
}

void loop() {
  // Trong FreeRTOS, loop() để trống vì các Task đã chạy riêng rồi.
  // Xóa nó đi cũng được, nhưng Arduino IDE bắt buộc phải có hàm này.
  vTaskDelete(NULL); // Xóa task loop mặc định để giải phóng RAM
}