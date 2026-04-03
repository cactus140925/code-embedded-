#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <WiFiClientSecure.h> // Thêm thư viện HTTPS
#include <ESP_Mail_Client.h> 
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// ================= CẤU HÌNH NGƯỜI DÙNG =================
#define WIFI_SSID "Nha co rat nhieu cay 2020"
#define WIFI_PASSWORD "phuongthaoquan@@"

// --- EMAIL ---
#define EMAIL_GUI     "ddat278@gmail.com"
#define MAT_KHAU_APP  "ewvk icxs bktk vuiv" 
#define EMAIL_NHAN    "ddat278@gmail.com" 

// --- GOOGLE SHEETS ---
// Thay ID Script của bạn vào đây (Lấy từ bước Deploy Web App)
const char* GAS_ID = "AKfycbxHcv6kpAvUVXfOyj0Js3gI9omLPG_RgOSQrlKZrAWwN0xf2sPXol3fI165jiJHKEzw"; 
const char* host = "script.google.com";

// ================= CẤU HÌNH PHẦN CỨNG =================
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#define I2C_SDA 8
#define I2C_SCL 9

// ================= CẤU TRÚC DỮ LIỆU =================
typedef struct struct_message {
  float nhietdo;
  float doam;
  int khoi;
  bool canhbao;
} struct_message;

// ================= BIẾN FREERTOS =================
QueueHandle_t sensorQueue; // Dữ liệu từ ESP-NOW -> Display
QueueHandle_t emailQueue;  // Lệnh gửi Email
QueueHandle_t googleQueue; // Lệnh gửi Google Sheet (MỚI)

// Biến trạng thái hiển thị
bool isSendingEmail = false; 
bool isSendingGoogle = false; // Để hiển thị icon đang upload

// ================= HÀM NHẬN ESP-NOW (ISR) =================
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (len != sizeof(struct_message)) return;
  struct_message tempMsg;
  memcpy(&tempMsg, incomingData, sizeof(tempMsg));
  xQueueOverwriteFromISR(sensorQueue, &tempMsg, NULL);
}

// ================= TASK 1: GỬI GOOGLE SHEETS (MỚI) =================
// Task này chạy ngầm, chuyên lo việc đẩy dữ liệu lên Google
void TaskGoogle(void *pvParameters) {
  struct_message logData;
  WiFiClientSecure client;
  client.setInsecure(); 

  for(;;) {
    // Chờ dữ liệu từ googleQueue
    if (xQueueReceive(googleQueue, &logData, portMAX_DELAY) == pdTRUE) {
      
      isSendingGoogle = true; 
      Serial.println(">> TaskGoogle: Dang day du lieu len Sheet...");

      if (client.connect(host, 443)) {
        // --- ĐOẠN NÀY ĐÃ SỬA ---
        // Thêm tham số "&led=" vào cuối chuỗi
        // logData.canhbao là bool, khi chuyển sang String sẽ là "1" (True) hoặc "0" (False)
        String url = "/macros/s/" + String(GAS_ID) + "/exec?temp=" + String(logData.nhietdo) 
                     + "&fire=" + String(logData.khoi) 
                     + "&led=" + String(logData.canhbao);
        
        // Gửi GET Request
        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "User-Agent: ESP32Node\r\n" +
                     "Connection: close\r\n\r\n");
        
        while (client.connected()) {
          String line = client.readStringUntil('\n');
          if (line == "\r") break;
        }
        Serial.println(">> TaskGoogle: XONG! URL: " + url); // In URL ra để kiểm tra
      } else {
        Serial.println(">> TaskGoogle: Ket noi THAT BAI!");
      }
      
      client.stop();
      isSendingGoogle = false;
    }
  }
}

// ================= TASK 2: GỬI EMAIL (NỀN) =================
void TaskEmail(void *pvParameters) {
  struct_message mailData;
  SMTPSession smtp;
  ESP_Mail_Session session;
  session.server.host_name = "smtp.gmail.com";
  session.server.port = 465;
  session.login.email = EMAIL_GUI;
  session.login.password = MAT_KHAU_APP;
  session.login.user_domain = "";

  for(;;) {
    if (xQueueReceive(emailQueue, &mailData, portMAX_DELAY) == pdTRUE) {
      isSendingEmail = true;
      Serial.println(">> TaskEmail: Bat dau gui email...");

      SMTP_Message message;
      message.sender.name = "CANH BAO CHAY ESP32";
      message.sender.email = EMAIL_GUI;
      message.subject = "BAO DONG! PHAT HIEN KHOI/LUA!";
      message.addRecipient("Chu Nha", EMAIL_NHAN);
      
      String htmlMsg = "<div style=\"color:red;\"><h1>⚠️ CẢNH BÁO KHẨN CẤP!</h1></div>";
      htmlMsg += "<p>Phát hiện nguy cơ cháy tại khu vực giám sát.</p>";
      htmlMsg += "<ul><li><b>Gas:</b> " + String(mailData.khoi) + "</li>";
      htmlMsg += "<li><b>Nhiệt độ:</b> " + String(mailData.nhietdo) + " °C</li></ul>";
      message.html.content = htmlMsg.c_str();

      if (smtp.connect(&session)) {
        MailClient.sendMail(&smtp, &message);
        smtp.closeSession();
      }
      isSendingEmail = false;
    }
  }
}

// ================= TASK 3: HIỂN THỊ & ĐIỀU PHỐI (CHÍNH) =================
void TaskDisplay(void *pvParameters) {
  struct_message currentData;
  
  // Timer cho Email
  unsigned long lastMailTime = 0;
  const unsigned long mailInterval = 120000; // 2 phút mới gửi mail 1 lần

  // Timer cho Google Sheet
  unsigned long lastLogTime = 0;
  const unsigned long logInterval = 15000;   // 15 giây log lên Sheet 1 lần

  // Init data
  currentData.nhietdo = 0; currentData.doam = 0; currentData.khoi = 0; currentData.canhbao = false;

  for(;;) {
    // 1. Nhận dữ liệu ESP-NOW
    if (xQueueReceive(sensorQueue, &currentData, 10 / portTICK_PERIOD_MS) == pdTRUE) {
      
      // LOGIC 1: Gửi Email nếu cháy
      if (currentData.canhbao) {
        if (millis() - lastMailTime > mailInterval || lastMailTime == 0) {
           Serial.println(">> TaskDisplay: Lenh gui Email!");
           xQueueSend(emailQueue, &currentData, 0);
           lastMailTime = millis();
        }
      }

      // LOGIC 2: Log lên Google Sheet định kỳ (Không cần cháy cũng log để theo dõi)
      if (millis() - lastLogTime > logInterval) {
         Serial.println(">> TaskDisplay: Lenh gui Google Sheet!");
         xQueueSend(googleQueue, &currentData, 0);
         lastLogTime = millis();
      }
    }

    // 2. Vẽ màn hình
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64);

    // Thông tin cơ bản
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(5, 15); u8g2.print("T: "); u8g2.print(currentData.nhietdo, 1); u8g2.print("C");
    u8g2.setCursor(70, 15); u8g2.print("H: "); u8g2.print((int)currentData.doam); u8g2.print("%");
    u8g2.drawLine(0, 20, 128, 20);

    // Cảnh báo hoặc Mức Gas
    if (currentData.canhbao) {
      u8g2.setFont(u8g2_font_helvB14_tr);
      u8g2.setCursor(25, 45); u8g2.print("! CHAY !");
    } else {
      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.setCursor(10, 40); u8g2.print("Gas Level:");
      u8g2.setFont(u8g2_font_helvB14_tr);
      u8g2.setCursor(45, 60); u8g2.print(currentData.khoi);
    }

    // Status Bar (Dòng dưới cùng)
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.setCursor(2, 62);
    if (isSendingEmail) u8g2.print("[MAIL]"); // Hiện chữ [MAIL] khi đang gửi mail
    u8g2.setCursor(35, 62);
    if (isSendingGoogle) u8g2.print("[LOG]"); // Hiện chữ [LOG] khi đang gửi Google

    u8g2.sendBuffer();
    vTaskDelay(50 / portTICK_PERIOD_MS); 
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  
  // OLED & WiFi Init
  Wire.begin(I2C_SDA, I2C_SCL);
  u8g2.begin();
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(" OK!");

  // ESP-NOW Init
  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW Error"); return; }
  esp_now_register_recv_cb(OnDataRecv);

  // Tạo Queues
  sensorQueue = xQueueCreate(1, sizeof(struct_message));
  emailQueue = xQueueCreate(1, sizeof(struct_message));
  googleQueue = xQueueCreate(5, sizeof(struct_message)); // Hàng đợi cho Google (để 5 bản tin cho an toàn)

  // Tạo Tasks
  // Task Display: Ưu tiên cao nhì (để mượt UI)
  xTaskCreate(TaskDisplay, "HienThi", 4096, NULL, 2, NULL);

  // Task Email: Ưu tiên thấp (chạy nền)
  xTaskCreate(TaskEmail, "GuiMail", 8192, NULL, 1, NULL);

  // Task Google: Ưu tiên thấp (chạy nền), Stack 8192 vì HTTPS tốn RAM
  xTaskCreate(TaskGoogle, "Google", 8192, NULL, 1, NULL);

  Serial.println("--- SYSTEM STARTED ---");
}

void loop() {
  vTaskDelete(NULL);
}