#include <DHT.h>

#define DHTPIN  35       // Chân DHT
#define DHTTYPE DHT11    // DHT11 thì đổi lại DHT11

DHT dht(DHTPIN, DHTTYPE);

// UART1 trên ESP32-S3
#define TXD1 43    // Chân TX UART1 (tùy board, nhiều board dùng GPIO17)
#define RXD1 18    // Không cần nếu chỉ gửi

void setup() {
  Serial.begin(115200);            // debug
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1); // UART1: 9600 baud
  dht.begin();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT ERROR");
    return;
  }

  // Gửi qua UART cho Arduino
  Serial1.print("T:");
  Serial1.print(t);
  Serial1.print(",H:");
  Serial1.print(h);
  Serial1.print("\n");

  // Debug
  Serial.print("Sent -> ");
  Serial.print("T:"); Serial.print(t);
  Serial.print(" H:"); Serial.println(h);

  delay(1000);
}
