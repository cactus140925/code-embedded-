#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// Cấu hình màn hình OLED 1.3 inch (SH1106) sử dụng I2C
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Khai báo chân kết nối
const int encoderPinA = 2; 
const int encoderPinB = 3;
const int motorENA = 9;
const int motorIN1 = 8;
const int motorIN2 = 7;

volatile long encoderCount = 0; // Biến lưu số xung, dùng volatile vì nằm trong ngắt

void setup() {
  // Cấu hình chân Motor
  pinMode(motorENA, OUTPUT);
  pinMode(motorIN1, OUTPUT);
  pinMode(motorIN2, OUTPUT);

  // Cấu hình chân Encoder
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // Thiết lập ngắt khi chân A thay đổi trạng thái
  attachInterrupt(digitalPinToInterrupt(encoderPinA), readEncoder, RISING);

  // Khởi tạo OLED
  u8g2.begin();
  
  // Chạy động cơ thử nghiệm (tốc độ 150/255)
  digitalWrite(motorIN1, HIGH);
  digitalWrite(motorIN2, LOW);
  analogWrite(motorENA, 150);
}

void loop() {
  u8g2.firstPage();
  do {
    // Nội dung vẽ đặt ở đây
    u8g2.setFont(u8g2_font_ncenB10_tr);  
    u8g2.drawStr(0, 20, "Encoder Value:");
    
    char buf[10];
    ltoa(encoderCount, buf, 10);
    u8g2.setFont(u8g2_font_logisoso24_tf);
    u8g2.drawStr(10, 55, buf);
    
  } while ( u8g2.nextPage() );

  delay(100); 
}

// Hàm xử lý ngắt để đọc Encoder
void readEncoder() {
  if (digitalRead(encoderPinB) == LOW) {
    encoderCount++;
  } else {
    encoderCount--;
  }
}