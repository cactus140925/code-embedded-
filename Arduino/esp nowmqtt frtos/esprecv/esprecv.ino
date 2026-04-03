#include <WiFi.h>
#include <esp_now.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "esp32-hal-cpu.h"

// --- CẤU HÌNH WIFI ---
const char* ssid = "Dat"; 
const char* password = "123456789";

// --- CẤU HÌNH THỜI GIAN (LOGIC CHỐNG LAG) ---
const unsigned long WIFI_TRY_TIMEOUT = 10000;    // Thử kết nối tối đa 10 giây
const unsigned long WIFI_COOLDOWN_TIME = 300000; // Nghỉ 5 phút (300,000ms) mới thử lại

// --- PHẦN CỨNG ---
#define I2C_SDA 8
#define I2C_SCL 9
#define BTN_RED     4    
#define BTN_GREEN   3  
#define BTN_YELLOW  2 

// Màn hình HW I2C (Quan trọng để giảm tải CPU)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, I2C_SCL, I2C_SDA);

// --- BIẾN TOÀN CỤC ---
typedef struct mess {
  float nhiedo; float doam; int doat; int asang;
} mess;
mess incomingData;

typedef struct cmd_struct {
  int color_id; 
} cmd_struct;
cmd_struct commandData;

// MAC S3 Gateway (Điền đúng MAC S3 của bạn vào đây)
uint8_t s3Address[] = {0xDC, 0xB4, 0xD9, 0x05, 0xC2, 0x0C}; 

// Biến Wifi Logic
unsigned long wifiAttemptStart = 0; 
unsigned long wifiCooldownStart = 0; 
bool isWifiCooldownMode = false;     
bool wasConnected = false;

// Cờ báo hiệu & Chống rung
volatile bool newDataReceived = false;
volatile bool btnRedPressed = false;
volatile bool btnGreenPressed = false;
volatile bool btnYellowPressed = false;
volatile unsigned long lastIsrTime = 0;

// --- HÀM NGẮT (ISR) ---
void IRAM_ATTR isrRed() { if (millis() - lastIsrTime > 200) { btnRedPressed = true; lastIsrTime = millis(); } }
void IRAM_ATTR isrGreen() { if (millis() - lastIsrTime > 200) { btnGreenPressed = true; lastIsrTime = millis(); } }
void IRAM_ATTR isrYellow() { if (millis() - lastIsrTime > 200) { btnYellowPressed = true; lastIsrTime = millis(); } }

// --- CALLBACK ---
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingDataPtr, int len) {
  memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));
  newDataReceived = true; 
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {}

// Gửi lệnh cho S3
void sendColorCommand(int color) {
  commandData.color_id = color;
  esp_now_send(s3Address, (uint8_t *) &commandData, sizeof(commandData));
}

void updateDisplay() {
  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 128, 64);
  u8g2.setFont(u8g2_font_profont12_tr);
  
  u8g2.setCursor(5, 10); 
  if (WiFi.status() == WL_CONNECTED) {
     u8g2.print("WIFI: ON (CH"); u8g2.print(WiFi.channel()); u8g2.print(")");
  } else {
     // Hiển thị trạng thái chờ nếu đang mất mạng
     if (isWifiCooldownMode) u8g2.print("WIFI: WAIT 5M");
     else u8g2.print("WIFI: CONNECTING");
  }
  
  u8g2.drawLine(0, 12, 128, 12);
  char buf[30];
  sprintf(buf, "T: %.1f C", incomingData.nhiedo); u8g2.drawStr(5, 25, buf);
  sprintf(buf, "H: %.1f %%", incomingData.doam); u8g2.drawStr(70, 25, buf);
  sprintf(buf, "Dat: %d", incomingData.doat); u8g2.drawStr(5, 45, buf);
  sprintf(buf, "Sang: %d", incomingData.asang); u8g2.drawStr(5, 58, buf);
  u8g2.sendBuffer();
}

void setup() {
  setCpuFrequencyMhz(80); // Giảm xung cho mát
  Serial.begin(115200);
  
  pinMode(BTN_RED, INPUT_PULLUP);
  pinMode(BTN_GREEN, INPUT_PULLUP);
  pinMode(BTN_YELLOW, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(BTN_RED), isrRed, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_GREEN), isrGreen, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_YELLOW), isrYellow, FALLING);

  u8g2.begin();
  
  // Dùng AP_STA để khi ngắt Wifi Station (disconnect), 
  // ESP-NOW vẫn sống nhờ Interface AP.
  WiFi.mode(WIFI_AP_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm); // Giảm công suất phát
  
  // Bắt đầu thử kết nối
  Serial.println("Khoi dong...");
  WiFi.begin(ssid, password);
  wifiAttemptStart = millis();
  isWifiCooldownMode = false;

  if (esp_now_init() != ESP_OK) return;
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, s3Address, 6);
  peerInfo.channel = 0; // Tự động theo Wifi
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_AP; // Dùng AP interface cho ổn định
  esp_now_add_peer(&peerInfo);
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. LOGIC WIFI CHỐNG LAG (Giống hệt S3) ---
  if (WiFi.status() == WL_CONNECTED) {
      if (!wasConnected) {
          wasConnected = true;
          isWifiCooldownMode = false;
          Serial.printf("Wifi OK! CH: %d\n", WiFi.channel());
          
          // Cập nhật lại màn hình ngay để báo đã có mạng
          newDataReceived = true; 
      }
  } 
  else {
      // MẤT MẠNG
      wasConnected = false;
      
      if (!isWifiCooldownMode) {
          // Giai đoạn 1: Đang thử kết nối (Trong 10s đầu)
          if (currentMillis - wifiAttemptStart > WIFI_TRY_TIMEOUT) {
              Serial.println("Qua 10s khong duoc -> Ngat Wifi!");
              
              // NGẮT NGAY để nhả Radio cho ESP-NOW
              WiFi.disconnect(); 
              
              // Chuyển sang chế độ chờ 5 phút
              isWifiCooldownMode = true;
              wifiCooldownStart = currentMillis;
              newDataReceived = true; // Update màn hình báo "WAIT 5M"
          }
      } 
      else {
          // Giai đoạn 2: Đang nghỉ (Trong 5 phút)
          // Radio hoàn toàn rảnh rỗi cho ESP-NOW
          
          if (currentMillis - wifiCooldownStart > WIFI_COOLDOWN_TIME) {
              Serial.println("Het 5 phut -> Thu ket noi lai...");
              isWifiCooldownMode = false;
              wifiAttemptStart = currentMillis;
              WiFi.reconnect(); // Thử lại
              newDataReceived = true; // Update màn hình báo "CONNECTING"
          }
      }
  }

  // --- 2. XỬ LÝ NÚT NHẤN (Luôn mượt) ---
  if (btnRedPressed) { sendColorCommand(1); btnRedPressed = false; }
  if (btnGreenPressed) { sendColorCommand(2); btnGreenPressed = false; }
  if (btnYellowPressed) { sendColorCommand(3); btnYellowPressed = false; }

  // --- 3. MÀN HÌNH ---
  if (newDataReceived) {
    updateDisplay();
    newDataReceived = false; 
  }

  delay(5); // Cho CPU thở
}