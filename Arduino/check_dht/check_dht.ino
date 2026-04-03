#include "DHT.h"

// Định nghĩa chân cắm và loại cảm biến
#define DHTPIN 2       // Nối chân Data của DHT11 vào GPIO 2 trên ESP32-C3
#define DHTTYPE DHT11  // Chọn loại cảm biến là DHT11

// Khởi tạo đối tượng DHT
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Mở cổng Serial để xem kết quả trên máy tính
  Serial.begin(115200);
  
  // Chờ một chút để cổng USB Serial của C3 ổn định
  delay(2000);
  Serial.println("ESP32-C3 Bat dau test DHT11!");

  // Khởi động thư viện DHT
  dht.begin();
}

void loop() {
  // DHT11 cần thời gian nghỉ tối thiểu 2 giây giữa các lần đọc
  delay(2000);

  // Lệnh đọc độ ẩm
  float h = dht.readHumidity();
  // Lệnh đọc nhiệt độ (độ C)
  float t = dht.readTemperature();

  // Kiểm tra xem dữ liệu đọc về có bị lỗi NaN (Not a Number) hay không
  if (isnan(h) || isnan(t)) {
    Serial.println(">> THAT BAI: Khong doc duoc du lieu, kiem tra lai day cam, dien tro hoac DHT11 da chet!");
    return;
  }

  // Nếu thành công, in ra màn hình
  Serial.print(">> THANH CONG! Nhiet do: ");
  Serial.print(t);
  Serial.print(" *C  |  Do am: ");
  Serial.print(h);
  Serial.println(" %");
}
