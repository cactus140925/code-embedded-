String input = "";

void setup() {
  Serial.begin(9600); // nhận từ ESP32
  Serial.println("Arduino ready");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    input += c;

    if (c == '\n') {             // ESP32 gửi xuống dòng
      int tIndex = input.indexOf("T:");
      int hIndex = input.indexOf(",H:");

      if (tIndex != -1 && hIndex != -1) {
        float temp = input.substring(tIndex + 2, hIndex).toFloat();
        float hum  = input.substring(hIndex + 3).toFloat();

        Serial.print("Nhiet do: ");
        Serial.println(temp);
        Serial.print("Do am: ");
        Serial.println(hum);
      }

      input = ""; // xoá buffer
    }
  }
}
