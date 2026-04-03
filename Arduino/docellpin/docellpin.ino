#include <U8g2lib.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// --- CÁC CHÂN KẾT NỐI ---
#define VOLT_PIN A0
#define AMP_PIN A1
#define GATE_PIN 4
#define BTN_PIN 2

const float vRef = 5.0;
const float rShunt = 1.0;
const float cutOffVolt = 3.0; 

// ==================================================
// HỆ SỐ HIỆU CHỈNH (CALIBRATION)
// Lấy (Áp đồng hồ đo) chia cho (Áp hiển thị cũ) = 3.83 / 3.73 = 1.0268
// Bạn có thể tự chỉnh lại số này cho đến khi mạch đo bằng đúng đồng hồ
// ==================================================
const float VOLT_CALIB = 1.0268; 
const float AMP_CALIB  = 1.0000; // Để tạm 1.0, sau này muốn chuẩn dòng thì chỉnh

// --- BIẾN ĐO ĐẠC ---
float vOpen = 0;
float capacity_mAh = 0.0;
float currentVLoad = 0;
float currentILoad = 0;
float savedVLoad = 0, savedILoad = 0, savedIR = 0;
float savedVOpen = 0; 

// --- BIẾN THỜI GIAN & NÚT BẤM ---
unsigned long pressTime = 0;
unsigned long lastUpdateTime = 0;
unsigned long stateTimer = 0;
unsigned long lastOledUpdate = 0;
bool isPressing = false;
bool longPressHandled = false;

// --- CẤU TRÚC MÁY TRẠNG THÁI ---
enum AppState {
  STATE_BOOT, STATE_STANDBY, STATE_NO_BATTERY, STATE_TESTING_IR,
  STATE_SHOWING_IR, STATE_MSG_DISCHARGE, STATE_DISCHARGING,
  STATE_MSG_STOPPED, STATE_COMPLETED
};
AppState currentState = STATE_BOOT;

// ==========================================
// HÀM ĐỌC ADC CHỐNG NHIỄU (Lấy trung bình 30 lần)
// ==========================================
float getFilteredVoltage() {
  long sum = 0;
  for(int i = 0; i < 30; i++) sum += analogRead(VOLT_PIN);
  return (sum / 30.0) * (vRef / 1023.0) * VOLT_CALIB;
}

float getFilteredCurrent() {
  long sum = 0;
  for(int i = 0; i < 30; i++) sum += analogRead(AMP_PIN);
  float vShunt = (sum / 30.0) * (vRef / 1023.0) * VOLT_CALIB * AMP_CALIB;
  return vShunt / rShunt;
}

void setup() {
  pinMode(GATE_PIN, OUTPUT);
  digitalWrite(GATE_PIN, LOW);
  pinMode(BTN_PIN, INPUT_PULLUP);

  u8g2.begin(); 
  u8g2.setBusClock(400000); 
  stateTimer = millis();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Dùng hàm chống nhiễu để đọc áp chờ
  vOpen = getFilteredVoltage();
  bool btnState = digitalRead(BTN_PIN);

  // ==========================================
  // 1. XỬ LÝ LOGIC NÚT NHẤN
  // ==========================================
  if (btnState == LOW && !isPressing) {
    isPressing = true;
    pressTime = currentMillis;
    longPressHandled = false;
  }

  if (btnState == LOW && isPressing) {
    if (currentMillis - pressTime > 1000 && !longPressHandled && currentState != STATE_DISCHARGING) {
      longPressHandled = true;
      currentState = STATE_MSG_DISCHARGE;
      stateTimer = currentMillis;
      capacity_mAh = 0.0;
      digitalWrite(GATE_PIN, HIGH);
    }
  }

  if (btnState == HIGH && isPressing) {
    isPressing = false;
    unsigned long holdDuration = currentMillis - pressTime;

    if (holdDuration > 50 && holdDuration <= 1000 && !longPressHandled) {
      if (currentState == STATE_DISCHARGING) {
        currentState = STATE_MSG_STOPPED;
        stateTimer = currentMillis;
        digitalWrite(GATE_PIN, LOW);
      } else if (vOpen > 0.5) {
        savedVOpen = vOpen;           
        currentState = STATE_TESTING_IR;
        stateTimer = currentMillis;
        digitalWrite(GATE_PIN, HIGH); 
      }
    }
  }

  // ==========================================
  // 2. XỬ LÝ LOGIC THEO TRẠNG THÁI
  // ==========================================
  switch (currentState) {
    case STATE_BOOT:
      if (currentMillis - stateTimer > 1500) currentState = STATE_STANDBY;
      break;

    case STATE_TESTING_IR:
      // Tăng thời gian chờ dòng điện ổn định lên 100ms
      if (currentMillis - stateTimer > 100) { 
        float vLoad = getFilteredVoltage();
        float iLoad = getFilteredCurrent();
        digitalWrite(GATE_PIN, LOW); // Tắt mạch ngay
        
        savedILoad = iLoad;
        savedIR = 0;
        if (savedILoad > 0) {
          savedIR = (savedVOpen - vLoad) / savedILoad; 
          // Nếu nhiễu làm áp sụt ngược (âm), ép về 0
          if (savedIR < 0) savedIR = 0; 
        }
        savedVLoad = vLoad;
        
        currentState = STATE_SHOWING_IR;
        stateTimer = currentMillis;
      }
      break;

    case STATE_SHOWING_IR:
      if (currentMillis - stateTimer > 4000) currentState = STATE_STANDBY;
      break;

    case STATE_MSG_DISCHARGE:
      if (currentMillis - stateTimer > 1000) {
        currentState = STATE_DISCHARGING;
        lastUpdateTime = currentMillis;
      }
      break;

    case STATE_DISCHARGING:
      {
        currentVLoad = getFilteredVoltage();
        currentILoad = getFilteredCurrent();

        if (currentVLoad <= cutOffVolt) {
          digitalWrite(GATE_PIN, LOW); 
          currentState = STATE_COMPLETED;
        } else {
          float elapsedTime_Hours = (currentMillis - lastUpdateTime) / 3600000.0;
          if (currentILoad > 0) {
            capacity_mAh += ((currentILoad * 1000.0) * elapsedTime_Hours);
          }
          lastUpdateTime = currentMillis;
        }
      }
      break;

    case STATE_COMPLETED:
    case STATE_STANDBY:
    case STATE_NO_BATTERY:
      if (vOpen < 0.5) currentState = STATE_NO_BATTERY;
      else if (currentState == STATE_NO_BATTERY) currentState = STATE_STANDBY;
      break;
  }

  // ==========================================
  // 3. VẼ LÊN MÀN HÌNH OLED
  // ==========================================
  if (currentMillis - lastOledUpdate > 100) {
    u8g2.firstPage();
    do {
      drawScreen();
    } while (u8g2.nextPage());
    
    lastOledUpdate = currentMillis;
  }
}

// --- HÀM XỬ LÝ HIỂN THỊ CHI TIẾT ---
void drawScreen() {
  u8g2.setFont(u8g2_font_8x13_tr); 

  switch (currentState) {
    case STATE_BOOT:
      u8g2.setFont(u8g2_font_9x15B_tr);
      u8g2.setCursor(10, 30); u8g2.print("Battery");
      u8g2.setCursor(10, 50); u8g2.print("Analyzer");
      break;

    case STATE_NO_BATTERY:
      u8g2.setCursor(0, 25); u8g2.print("---No Battery---");
      u8g2.setCursor(0, 45); u8g2.print(" Please Insert! ");
      break;

    case STATE_STANDBY:
      u8g2.setCursor(0, 15); u8g2.print("V_Pin: "); u8g2.print(vOpen, 2); u8g2.print(" V");
      u8g2.setCursor(0, 35); u8g2.print("Hold 1s: TestCap");
      u8g2.setCursor(0, 55); u8g2.print("1 Click: Test IR");
      break;

    case STATE_TESTING_IR:
      u8g2.setCursor(10, 35); u8g2.print("Testing IR...");
      break;

    case STATE_SHOWING_IR:
      u8g2.setCursor(0, 15); u8g2.print("V:  "); u8g2.print(savedVLoad, 2); u8g2.print(" V");
      u8g2.setCursor(0, 35); u8g2.print("I:  "); u8g2.print(savedILoad * 1000, 0); u8g2.print(" mA");
      u8g2.setCursor(0, 55); u8g2.print("IR: "); u8g2.print(savedIR * 1000, 0); u8g2.print(" mOhm");
      break;

    case STATE_MSG_DISCHARGE:
      u8g2.setCursor(0, 35); u8g2.print("Discharge Mode..");
      break;

    case STATE_DISCHARGING:
      u8g2.setCursor(0, 15); u8g2.print("V: "); u8g2.print(currentVLoad, 2); u8g2.print(" V");
      u8g2.setCursor(0, 35); u8g2.print("I: "); u8g2.print(currentILoad * 1000, 0); u8g2.print(" mA");
      u8g2.setCursor(0, 55); u8g2.print("C: "); u8g2.print(capacity_mAh, 0); u8g2.print(" mAh");
      break;

    case STATE_MSG_STOPPED:
      u8g2.setFont(u8g2_font_9x15B_tr);
      u8g2.setCursor(30, 40); u8g2.print("Stopped!");
      break;

    case STATE_COMPLETED:
      u8g2.setCursor(0, 25); u8g2.print("Test Completed!");
      u8g2.setCursor(0, 45); u8g2.print("Cap: "); u8g2.print(capacity_mAh, 0); u8g2.print(" mAh");
      break;
  }
}