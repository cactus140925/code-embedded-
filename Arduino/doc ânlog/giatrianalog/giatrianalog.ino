int analogPin = A0; // Chân analog để đọc giá trị từ cảm biến
int ledPins[] = {1, 2, 3, 4, 5, 6, 7, 8}; // Mảng chứa các chân LED

void setup() {
  Serial.begin(9600);

  // Thiết lập các chân LED là OUTPUT
  for (int i = 0; i < 8; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
}

void batdan() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(ledPins[i], HIGH); // Bật LED
    delay(500);
  }
}

void tatdan() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(ledPins[i], LOW); // Tắt LED
    delay(500);
  }
}

void xenke() {
  for (int i = 0; i < 8; i++) {
    if (i % 2 == 0) {
      digitalWrite(ledPins[i], HIGH); // Bật các LED ở vị trí chẵn
    } else {
      digitalWrite(ledPins[i], LOW); // Tắt các LED ở vị trí lẻ
    }
  }
  delay(500);
}

void loop() {
  int docanalog = analogRead(analogPin); // Đọc giá trị từ chân analog
  if (docanalog >= 0 && docanalog < 400) {
    batdan();
  } else if (docanalog >= 400 && docanalog < 800) {
    tatdan();
  } else {
    xenke();
  }
}
