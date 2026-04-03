#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <HTTPClient.h> 
#include "driver/temp_sensor.h"

// --- 1. CẤU HÌNH WIFI & MQTT ---
const char* ssid = "Dat"; 
const char* password = "123456789";

const char* mqtt_server = "8841aface2d445b79ee4622bb41afb36.s1.eu.hivemq.cloud"; 
const int mqtt_port = 8883; 
const char* mqtt_user = "dat140925"; 
const char* mqtt_pass = "Dat140925@";
const char* mqtt_topic = "esp32/data/demo";

// --- CẤU HÌNH GOOGLE SHEET ---
String GOOGLE_SCRIPT_ID = "AKfycbzOAQVg5oNFIY7ABpoauYqWPFdlF38PBoIPEn3MfCNsVKlhZLoeOe2sE7u0hpEXAKv2"; // <--- DÁN MÃ SCRIPT CỦA BẠN VÀO ĐÂY

WiFiClientSecure espClient;
PubSubClient client(espClient);

// --- 2. CẤU HÌNH CẢM BIẾN ---
#define TOUCH_PIN 1
#define TOUCH_PIN2 3

// --- BIẾN TOÀN CỤC (Shared Variables) ---
float currentTemp = 0;
int currentT1 = 0;
int currentT2 = 0;
String currentStatus = "Normal"; // Biến để ghi chú xem là gửi thường hay báo động

// --- QUẢN LÝ TASK ---
TaskHandle_t GoogleTaskHandle = NULL; // Biến để nắm đầu Task Google

// --- BIẾN QUẢN LÝ THỜI GIAN ---
unsigned long lastTime = 0;
const long interval = 2000;
bool lastTouchState1 = false;
bool lastTouchState2 = false;

// Biến chống Spam khi báo động (Chỉ gửi báo động tối đa 5 giây/lần)
unsigned long lastAlarmTime = 0; 
const long alarmCooldown = 5000; 

// ================================================================
// TASK RIÊNG: CHUYÊN GỬI GOOGLE SHEET (Core 0)
// ================================================================
void TaskGoogleSheet(void * parameter) {
  WiFiClientSecure clientHTTPS;
  clientHTTPS.setInsecure(); 

  for(;;) { 
    // QUAN TRỌNG: Thay vTaskDelay bằng ulTaskNotifyTake
    // Lệnh này có nghĩa: "Ngủ 20 giây. NHƯNG nếu ai đó gọi (Notify) thì dậy ngay lập tức"
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(20000)); 

    // Tỉnh dậy (Do hết 20s hoặc do bị gọi) -> Kiểm tra Wifi và gửi
    if(WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      
      // Tạo URL
      String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?";
      url += "temp=" + String(currentTemp);
      url += "&t1=" + String(currentT1);
      url += "&t2=" + String(currentT2);
      // Bạn có thể sửa Google Script để nhận thêm cột "status" nếu muốn
      
      Serial.print("[Core 0] Dang gui Google Sheet (" + currentStatus + ")...");
      
      http.begin(clientHTTPS, url);
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      
      int httpCode = http.GET();
      
      if (httpCode > 0) {
        Serial.println(" -> OK!");
      } else {
        Serial.printf(" -> Loi: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
      
      // Gửi xong thì reset trạng thái về bình thường
      currentStatus = "Dinh_Ky_20s"; 
    }
  }
}

// ================================================================
// CÁC HÀM CƠ BẢN
// ================================================================
void setup_wifi() {
  delay(10);
  Serial.print("Ket noi WiFi: "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi OK!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Ket noi MQTT...");
    String clientId = "ESP32-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("OK!");
    } else {
      Serial.print("Loi rc="); Serial.print(client.state()); delay(5000);
    }
  }
}

bool checktouch(int touch) { return (touch > 130000); } // Logic cũ cho MQTT

void sendData(float temp, int tVal1, int tVal2, bool st1, bool st2) {
  char buffer[200];
  snprintf(buffer, sizeof(buffer), 
    "{\"temperature\":%.2f, \"touch_val\":%d, \"touch_val2\":%d, \"touch1\":%d, \"touch2\":%d}", 
    temp, tVal1, tVal2, st1, st2
  );
  client.publish(mqtt_topic, buffer);
}

void setup() {
  Serial.begin(115200);
  
  temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
  temp_sensor_set_config(temp_sensor);
  temp_sensor_start();
  
  setup_wifi();
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);

  // TẠO TASK VÀ LƯU HANDLE VÀO BIẾN "GoogleTaskHandle"
  xTaskCreatePinnedToCore(
    TaskGoogleSheet,   
    "GoogleTask",      
    10000,             
    NULL,              
    1,                 
    &GoogleTaskHandle, // <--- Quan trọng: Lưu địa chỉ task để tí nữa còn "gọi" nó
    0                  
  );
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // 1. ĐỌC DỮ LIỆU
  float temp = 0;
  temp_sensor_read_celsius(&temp);
  int touchVal = touchRead(TOUCH_PIN);
  int touchVal2 = touchRead(TOUCH_PIN2);
  
  // Cập nhật biến toàn cục cho Core 0
  currentTemp = temp;
  currentT1 = touchVal;
  currentT2 = touchVal2;
  
  bool currentTouch1 = checktouch(touchVal);
  bool currentTouch2 = checktouch(touchVal2);

  // --- LOGIC MỚI: KIỂM TRA ĐIỀU KIỆN BÁO ĐỘNG ---
  // Điều kiện: Nhiệt > 40 HOẶC Touch1 > 200k HOẶC Touch2 > 200k
  if (temp > 40 || touchVal > 200000 || touchVal2 > 200000) {
    
    // Kiểm tra Cooldown: Chỉ gửi nếu lần gửi trước cách đây quá 5 giây
    // (Tránh trường hợp nhiệt độ cao liên tục làm ESP gửi spam 1000 lần/phút dẫn đến treo)
    if (millis() - lastAlarmTime > alarmCooldown) {
      
      Serial.println("[Core 1] PHAT HIEN BAO DONG! Goi Core 0 day ngay!");
      currentStatus = "ALARM_KANZ"; // Đánh dấu là tin nhắn khẩn cấp
      
      // LỆNH QUAN TRỌNG: Đánh thức Task Google ngay lập tức
      xTaskNotifyGive(GoogleTaskHandle); 
      
      lastAlarmTime = millis(); // Lưu thời gian để tính cooldown
    }
  }

  // 2. GỬI MQTT (Logic cũ)
  if (currentTouch1 != lastTouchState1 || currentTouch2 != lastTouchState2) {
    sendData(temp, touchVal, touchVal2, currentTouch1, currentTouch2);
    lastTouchState1 = currentTouch1;
    lastTouchState2 = currentTouch2;
    delay(100); 
  }

  // 3. MQTT HEARTBEAT
  if (millis() - lastTime >= interval) {
    lastTime = millis(); 
    sendData(temp, touchVal, touchVal2, currentTouch1, currentTouch2);
  }
}