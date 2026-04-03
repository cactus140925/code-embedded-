#include <Servo.h>

Servo dongco;
#define IR 8
#define coi 7

void setup() {
  Serial.begin(9600);
  dongco.attach(9);
  pinMode(IR, INPUT);
  pinMode(coi, OUTPUT);
  digitalWrite(coi, LOW);  // Đảm bảo tắt còi ngay từ đầu
   dongco.write(0);
}

void loop() {
  int doc = digitalRead(IR);  // Đọc giá trị từ cảm biến IR
  noTone(coi);
  if (doc == LOW) {          // Nếu có tín hiệu từ cảm biến IR
    tone(coi, 2000);          // Bật còi với tần số 2000 Hz
    dongco.write(90);         // Quay động cơ servo tới góc 90 độ
    delay(1000);              // Đợi 1 giây
    noTone(coi);
    delay(3000);
                  // Tắt còi
  } else {
    dongco.write(0);          // Nếu không có tín hiệu, đặt động cơ về vị trí 0 độ
  }

}
