#define BLYNK_TEMPLATE_ID "TMPL6iCrstT2J"
#define BLYNK_TEMPLATE_NAME "controldht"
#define BLYNK_AUTH_TOKEN "x1LCc8G53oHhmwLbRIAXy45odqBKA3lw"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// --- CẤU HÌNH ---
char ssid[] = "Dat";
char pass[] = "123456789";

#define DHTPIN 4          // Chân GPIO nối với DHT11
#define DHTTYPE DHT11     // Loại cảm biến

DHT dht(DHTPIN, DHTTYPE);

// Khai báo Handle cho các Task
TaskHandle_t TaskWifiHandle;
TaskHandle_t TaskDHTHandle;

// Biến toàn cục để kiểm tra trạng thái WiFi cho Blynk
bool isWifiConnected = false;

// --- TASK 1: QUẢN LÝ KẾT NỐI WIFI ---
void TaskWifi(void *pvParameters) {
  while (true) {
    if (WiFi.status() == WL_CONNECTED) {
      isWifiConnected = true;
      // Nếu đã kết nối, kiểm tra lại sau mỗi 5 giây
      vTaskDelay(5000 / portTICK_PERIOD_MS); 
    } else {
      isWifiConnected = false;
      Serial.println("[Task 1] WiFi mất kết nối. Đang kết nối lại...");
      WiFi.begin(ssid, pass);
      
      // Chờ kết nối (Timeout 10s)
      int timer = 0;
      while (WiFi.status() != WL_CONNECTED && timer < 10) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        Serial.print(".");
        timer++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[Task 1] Đã kết nối WiFi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
      } else {
        Serial.println("\n[Task 1] Kết nối thất bại. Thử lại sau.");
      }
    }
    // Delay ngắn để tránh treo task
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// --- TASK 2: ĐỌC DHT11 VÀ GỬI BLYNK ---
void TaskDHT(void *pvParameters) {
  // Cấu hình Blynk (không dùng Blynk.begin để tránh chặn code nếu mất wifi)
  Blynk.config(BLYNK_AUTH_TOKEN);

  // Biến dùng để tạo delay không chặn (non-blocking delay) cho việc đọc cảm biến
  unsigned long lastReadTime = 0;
  const long interval = 2000; // Đọc mỗi 2 giây

  while (true) {
    // Chỉ chạy Blynk và đọc cảm biến khi có WiFi
    if (isWifiConnected) {
      Blynk.run(); // Hàm này cần gọi liên tục để giữ kết nối với Server

      unsigned long currentMillis = millis();
      if (currentMillis - lastReadTime >= interval) {
        lastReadTime = currentMillis;

        // Đọc độ ẩm và nhiệt độ
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        // Kiểm tra lỗi đọc cảm biến
        if (isnan(h) || isnan(t)) {
          Serial.println("[Task 2] Lỗi đọc cảm biến DHT11!");
        } else {
          Serial.printf("[Task 2] Nhiệt độ: %.1f C, Độ ẩm: %.1f %%\n", t, h);
          
          // Gửi lên Blynk (V0 là Nhiệt độ, V1 là Độ ẩm)
          Blynk.virtualWrite(V0, t);
          Blynk.virtualWrite(V1, h);
        }
      }
    }
    
    // Delay cực ngắn để nhường CPU cho các tác vụ nền (Watchdog)
    // Lưu ý: Không được delay lâu ở đây vì Blynk.run() cần chạy liên tục
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Tạo Task 1: Chạy trên Core 0 (Thường dùng cho tác vụ hệ thống/Wifi)
  xTaskCreatePinnedToCore(
    TaskWifi,      // Hàm thực thi
    "TaskWifi",    // Tên Task
    4096,          // Stack size
    NULL,          // Tham số
    1,             // Độ ưu tiên (Thấp hơn hoặc bằng Task 2)
    &TaskWifiHandle, // Handle
    0              // Core ID (0)
  );

  // Tạo Task 2: Chạy trên Core 1 (Thường dùng cho ứng dụng người dùng)
  xTaskCreatePinnedToCore(
    TaskDHT,       // Hàm thực thi
    "TaskDHT",     // Tên Task
    4096,          // Stack size
    NULL,          // Tham số
    1,             // Độ ưu tiên
    &TaskDHTHandle,// Handle
    1              // Core ID (1)
  );
}

void loop() {
  // Loop để trống vì FreeRTOS đã xử lý trong các Task
  // Hoặc có thể xóa TaskWifi và dùng Loop làm Task xử lý WiFi nếu muốn đơn giản hơn
  vTaskDelete(NULL); // Xóa task loop mặc định để tiết kiệm tài nguyên
}