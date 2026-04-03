#include <SoftwareSerial.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// --- CẤU HÌNH ---
SoftwareSerial myLoRa(2, 3); // RX, TX
#define BUTTON_PIN 4

// Màn hình (Chế độ Page Buffer tiết kiệm RAM)
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// --- BIẾN TOÀN CỤC (Dùng char array thay cho String) ---
char bufferNhiet[10] = "--";
char bufferLux[10]   = "--";
char bufferDen[10]   = "--";
char serialBuffer[64]; // Bộ đệm nhận tin nhắn
int bufIndex = 0;

// Biến trạng thái
bool needUpdateScreen = true; // Cờ báo hiệu cần vẽ lại màn hình
unsigned long lastButtonCheck = 0;
int lastBtnState = HIGH;

void setup() {
  Serial.begin(9600);
  myLoRa.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  u8g2.begin();
  
  // Màn hình chào
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(10, 30, "System Ready!");
  } while ( u8g2.nextPage() );
  delay(1000);
}

void loop() {
  // -------------------------------------------------
  // 1. ĐỌC NÚT NHẤN (Ưu tiên cao nhất, kiểm tra nhanh)
  // -------------------------------------------------
  int btnState = digitalRead(BUTTON_PIN);
  
  // Chỉ xử lý khi trạng thái thay đổi và đã qua thời gian chống rung (50ms)
  if (btnState != lastBtnState && (millis() - lastButtonCheck > 50)) {
    lastButtonCheck = millis();
    
    if (btnState == LOW) { // Nhấn xuống
      Serial.println(F("-> Nut bam! Gui lenh..."));
      myLoRa.println("1"); // Gửi sang ESP32
      
      // Vẽ chữ "Sending..." tạm thời
      drawStatus("Sending...");
      delay(200); // Dừng xíu để người dùng kịp nhìn
      needUpdateScreen = true; // Yêu cầu vẽ lại giao diện chính sau đó
    }
    lastBtnState = btnState;
  }

  // -------------------------------------------------
  // 2. NHẬN DỮ LIỆU TỪ LORA (Không dùng String)
  // -------------------------------------------------
  while (myLoRa.available()) {
    char c = myLoRa.read();
    
    if (c == '\n') { // Kết thúc dòng lệnh
      serialBuffer[bufIndex] = '\0'; // Kết thúc chuỗi
      parseData(serialBuffer);       // Phân tích dữ liệu
      bufIndex = 0;                  // Reset bộ đệm
      needUpdateScreen = true;       // Có dữ liệu mới -> Cho phép vẽ lại màn hình
    } 
    else {
      if (bufIndex < 63) { // Tránh tràn bộ đệm
        serialBuffer[bufIndex++] = c;
      }
    }
  }

  // -------------------------------------------------
  // 3. VẼ MÀN HÌNH (Chỉ chạy khi cần thiết)
  // -------------------------------------------------
  if (needUpdateScreen) {
    u8g2.firstPage();
    do {
      drawInterface();
    } while ( u8g2.nextPage() );
    needUpdateScreen = false; // Vẽ xong rồi thì thôi, không vẽ lại nữa
  }
}

// --- HÀM TÁCH DỮ LIỆU (Dùng chuỗi C thuần túy - Không tốn RAM) ---
// Dữ liệu mẫu: "Nhiet:28.5|Lux:2048|Den:1"
void parseData(char* data) {
  // Tìm các dấu gạch đứng
  char* phan1 = strtok(data, "|"); // Cắt lấy đoạn đầu: Nhiet:28.5
  char* phan2 = strtok(NULL, "|"); // Cắt đoạn tiếp: Lux:2048
  char* phan3 = strtok(NULL, "|"); // Cắt đoạn cuối: Den:1

  // Tách giá trị sau dấu hai chấm
  if (phan1) parseValue(phan1, bufferNhiet);
  if (phan2) parseValue(phan2, bufferLux);
  if (phan3) parseValue(phan3, bufferDen);
}

// Hàm phụ để lấy số sau dấu ":" (Ví dụ: "Nhiet:28.5" -> lấy "28.5")
void parseValue(char* str, char* dest) {
  char* val = strchr(str, ':'); // Tìm dấu :
  if (val) {
    strcpy(dest, val + 1); // Copy phần sau dấu : vào biến đích
  }
}

// --- HÀM VẼ GIAO DIỆN CHÍNH ---
void drawInterface() {
  u8g2.drawFrame(0, 0, 128, 64);
  
  u8g2.setFont(u8g2_font_helvB08_tr); 
  u8g2.drawStr(15, 12, "HE THONG GIAM SAT");
  u8g2.drawLine(0, 14, 128, 14);

  // Nhiệt độ
  u8g2.drawStr(5, 28, "Temp:");
  u8g2.drawStr(45, 28, bufferNhiet);
  u8g2.drawStr(85, 28, "oC");

  // Lux
  u8g2.drawStr(5, 43, "Lux :");
  u8g2.drawStr(45, 43, bufferLux);

  // Đèn
  u8g2.drawStr(5, 58, "Lamp:");
  u8g2.setFont(u8g2_font_helvB10_tr);
  
  if (strcmp(bufferDen, "1") == 0) {
    u8g2.drawStr(45, 58, "ON");
    u8g2.drawDisc(110, 54, 4);
  } else if (strcmp(bufferDen, "0") == 0) {
    u8g2.drawStr(45, 58, "OFF");
    u8g2.drawCircle(110, 54, 4);
  } else {
    u8g2.setFont(u8g2_font_helvB08_tr);
    u8g2.drawStr(45, 58, "Wait...");
  }
}

// Hàm vẽ thông báo trạng thái nhỏ
void drawStatus(const char* msg) {
  u8g2.firstPage();
  do {
    drawInterface(); // Vẽ lại nền cũ
    // Vẽ đè thông báo lên
    u8g2.setDrawColor(0);
    u8g2.drawBox(30, 20, 70, 20); // Xóa vùng giữa
    u8g2.setDrawColor(1);
    u8g2.drawFrame(30, 20, 70, 20);
    u8g2.setFont(u8g2_font_helvB08_tr);
    u8g2.drawStr(35, 34, msg);
  } while ( u8g2.nextPage() );
}