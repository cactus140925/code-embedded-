#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI(); 

void setup(void) {
  Serial.begin(115200);
  tft.init();

  // --- 1. SỬA LỖI SAI MÀU (INVERSION) ---
  // Nếu màn hình bị trắng xóa hoặc màu bị âm bản -> Sửa false thành true
  // Nếu màn hình màu đen đúng chuẩn -> Giữ nguyên false
  tft.invertDisplay(true); 

  // --- 2. SỬA LỖI NGƯỢC CHỮ (MIRROR) ---
  // Nếu chữ bị ngược (soi gương) -> Dùng số 3
  // Nếu chữ bị lộn đầu -> Dùng số 1
  tft.setRotation(3); 

  // --- 3. SỬA LỖI LỆCH HÌNH/NHIỄU (OFFSET) ---
  // Vì dùng Rotation 3 trên GreenTab sẽ bị lệch 26 pixel.
  // Dòng này giúp đẩy hình về giữa và xóa nhiễu.
  tft.setViewport(0, 26, 160, 80); 

  // --- VẼ TEST ---
  tft.fillScreen(TFT_BLACK);
  
  // Test màu: Vẽ 3 hình tròn Đỏ - Xanh Lá - Xanh Dương
  // Nếu màu hiển thị đúng thứ tự Đỏ-Lá-Dương là OK
  tft.fillCircle(30, 40, 10, TFT_RED);
  tft.fillCircle(80, 40, 10, TFT_GREEN);
  tft.fillCircle(130, 40, 10, TFT_BLUE);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("DA HET LOI!", 80, 10, 2);
  tft.drawCentreString("R     G     B", 80, 60, 2);
}

void loop() {
  // --- MÀN HÌNH 1: KIỂM TRA KHUNG VIỀN ---
  tft.fillScreen(TFT_BLACK);
  
  // Vẽ khung đỏ sát mép màn hình để xem có bị mất cạnh nào không
  tft.drawRect(0, 0, 160, 80, TFT_RED);
  tft.drawRect(1, 1, 158, 78, TFT_YELLOW);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("TEST MAN HINH", 80, 20, 2);
  tft.drawCentreString("ESP8266", 80, 45, 2);
  
  delay(2000);

  // --- MÀN HÌNH 2: KIỂM TRA MÀU SẮC (RGB) ---
  // Màn hình đỏ
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.drawCentreString("RED", 80, 35, 2);
  delay(1000);

  // Màn hình xanh lá
  tft.fillScreen(TFT_GREEN);
  tft.setTextColor(TFT_BLACK, TFT_GREEN);
  tft.drawCentreString("GREEN", 80, 35, 2);
  delay(1000);

  // Màn hình xanh dương
  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.drawCentreString("BLUE", 80, 35, 2);
  delay(1000);
}