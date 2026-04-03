#include <DHT.h>

#define DHTPIN 2     //  dữ liệu của DHT11 nối với chân số 2 của Arduino Nano
#define DHTTYPE DHT11   // Loại cảm biến là DHT11
#define TRIG 7
#define ECHO 6
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600); // Khởi tạo Serial với baud rate 9600
  dht.begin();        // Khởi tạo cảm biến DHT11
  pinMode(TRIG,OUTPUT);
  pinMode(ECHO,INPUT);
}

void loop() {
  delay(2000);        // Đợi 2 giây (DHT11 yêu cầu ít nhất 1 giây giữa các lần đọc)

  float humidity = dht.readHumidity();    // Đọc độ ẩm
  float temperature = dht.readTemperature(); // Đọc nhiệt độ (đơn vị Celsius)

  // Kiểm tra xem dữ liệu có hợp lệ không
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT11 sensor!");
    return;
  }

  // In dữ liệu ra Serial Monitor
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");
  long duration;
  float distance;
  digitalWrite(TRIG, LOW);  // Đặt Trig LOW để "làm sạch" tín hiệu trước khi phát
delayMicroseconds(2);        // Đợi 2 microseconds để đảm bảo Trig ổn định ở LOW

digitalWrite(TRIG, HIGH); // Kích hoạt Trig HIGH để phát xung siêu âm
delayMicroseconds(10);       // Giữ HIGH trong 10 microseconds (thời gian chuẩn để phát sóng)
digitalWrite(TRIG, LOW);  // Đặt lại Trig LOW để kết thúc phát

// Đọc thời gian phản hồi từ Echo
duration = pulseIn(ECHO, HIGH);  // Đo thời gian chân Echo ở HIGH (từ khi nhận sóng đến khi hết)

// Tính khoảng cách
distance = duration * 0.034 / 2;  // Công thức: (thời gian * tốc độ âm thanh) / 2

// In kết quả
Serial.print("Distance: ");
Serial.print(distance);
Serial.println(" cm");
}