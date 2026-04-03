#include <DHT.h>
#include <WiFi.h>
#include <esp_now.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// --- CẤU HÌNH ---
#define DHTPIN 4       
#define SOIL_PIN 5     
#define SUN_PIN 7      
#define DHTTYPE DHT22  
#define LED_RED     15
#define LED_GREEN   16
#define LED_YELLOW  17

const char* ssid = "Dat"; 
const char* password = "123456789";

const char* mqtt_server = "8841aface2d445b79ee4622bb41afb36.s1.eu.hivemq.cloud"; 
const int mqtt_port = 8883; 
const char* mqtt_user = "dat140925"; 
const char* mqtt_pass = "Dat140925@";
const char* mqtt_topic_pub = "home/sensor/data";
const char* mqtt_topic_sub = "home/sensor/cmd"; 

// --- CẤU HÌNH THỜI GIAN (LOGIC CHỐNG LAG) ---
const unsigned long WIFI_TRY_TIMEOUT = 10000;    // Thử kết nối trong 10 giây
const unsigned long WIFI_COOLDOWN_TIME = 300000; // Nghỉ 5 phút (300.000ms)

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Biến Wifi Logic
unsigned long wifiAttemptStart = 0;
unsigned long wifiCooldownStart = 0;
bool isWifiCooldownMode = false;
bool wasConnected = false; // Biến kiểm tra để đồng bộ kênh 1 lần

// Struct Data
typedef struct mess { float nhiedo; float doam; int doat; int asang; } mess;
typedef struct cmd_struct { int color_id; } cmd_struct;
cmd_struct commandRecv;

// ĐỊA CHỈ MAC CỦA ESP32-C3 (Gửi Unicast để đảm bảo nhận được tin)
uint8_t c3tAddress[] = {0x88,0x56,0xA6,0x6F,0x1C,0x20};

unsigned long lastEspNowTime = 0;
bool stt_red=0, stt_green=0, stt_yellow=0;

// --- HÀM GỬI MQTT ---
void publishState() {
  if (client.connected()) {
    char json[200];
    snprintf(json, sizeof(json), "{\"led_r\":%d, \"led_g\":%d, \"led_y\":%d}", stt_red, stt_green, stt_yellow);
    client.publish(mqtt_topic_pub, json);
  }
}

// --- CALLBACK ---
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) message += (char)payload[i];
  if (message == "RED") { stt_red = !stt_red; digitalWrite(LED_RED, stt_red); } 
  else if (message == "GREEN") { stt_green = !stt_green; digitalWrite(LED_GREEN, stt_green); }
  else if (message == "YELLOW") { stt_yellow = !stt_yellow; digitalWrite(LED_YELLOW, stt_yellow); }
  publishState();
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&commandRecv, incomingData, sizeof(commandRecv));
  // Xử lý ngay lập tức
  switch (commandRecv.color_id) {
    case 1: stt_red = !stt_red; digitalWrite(LED_RED, stt_red); break;
    case 2: stt_green = !stt_green; digitalWrite(LED_GREEN, stt_green); break;
    case 3: stt_yellow = !stt_yellow; digitalWrite(LED_YELLOW, stt_yellow); break;
  }
  publishState();
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {}

void reconnectMQTT() {
  if (!client.connected()) {
    String clientId = "S3-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      client.subscribe(mqtt_topic_sub);
      publishState();
    }
  }
}

// --- TASK CHÍNH ---
void TaskReadAndSend(void *pvParameters) {
  mess dataToSend;
  char mqtt_full_payload[256]; 
  unsigned long lastMqttSensorTime = 0;

  // Khởi động: Bắt đầu thử kết nối Wifi ngay
  WiFi.begin(ssid, password);
  wifiAttemptStart = millis();
  isWifiCooldownMode = false;

  for (;;) {
    unsigned long currentMillis = millis();

    // --- 1. QUẢN LÝ WIFI THÔNG MINH (ANTI-LAG & SYNC CHANNEL) ---
    if (WiFi.status() == WL_CONNECTED) {
        // Có mạng: Reset trạng thái chờ
        isWifiCooldownMode = false;
        
        // [QUAN TRỌNG] Nếu vừa kết nối lại -> Cập nhật kênh cho ESP-NOW
        if (!wasConnected) {
            wasConnected = true;
            Serial.printf("Wifi OK! Channel: %d. Syncing ESP-NOW...\n", WiFi.channel());
            
            esp_now_peer_info_t peerInfo;
            memset(&peerInfo, 0, sizeof(peerInfo));
            memcpy(peerInfo.peer_addr, c3tAddress, 6); // Dùng địa chỉ C3
            peerInfo.channel = WiFi.channel();         // Kênh theo Router
            peerInfo.encrypt = false;
            peerInfo.ifidx = WIFI_IF_AP;
            
            esp_now_mod_peer(&peerInfo); // Cập nhật lại Peer
        }
        
        // Chạy MQTT
        reconnectMQTT();
        client.loop();
    } 
    else {
        // MẤT MẠNG
        wasConnected = false;
        
        if (!isWifiCooldownMode) {
            // GIAI ĐOẠN 1: ĐANG CỐ KẾT NỐI (Trong 10 giây đầu)
            if (currentMillis - wifiAttemptStart > WIFI_TRY_TIMEOUT) {
                Serial.println("Qua 10s khong duoc -> Ngat Wifi!");
                WiFi.disconnect(); // Ngắt để nhả Radio
                
                isWifiCooldownMode = true;
                wifiCooldownStart = currentMillis;
            }
        } 
        else {
            // GIAI ĐOẠN 2: ĐANG NGHỈ (Trong 5 phút)
            if (currentMillis - wifiCooldownStart > WIFI_COOLDOWN_TIME) {
                Serial.println("Het 5p -> Thu ket noi lai...");
                isWifiCooldownMode = false;
                wifiAttemptStart = currentMillis;
                WiFi.reconnect(); 
            }
        }
    }

    // --- 2. CÁC TÁC VỤ KHÁC (LUÔN CHẠY) ---
    // Gửi ESP-NOW (Unicast cho C3)
    if (currentMillis - lastEspNowTime > 500) {
        lastEspNowTime = currentMillis;
        float h = dht.readHumidity(); float t = dht.readTemperature();
        if (isnan(h)) {h=0;t=0;}
        int sun = map(analogRead(SUN_PIN),0,4095,0,100);
        int soil = map(analogRead(SOIL_PIN),0,4095,0,100);

        dataToSend.nhiedo = t; dataToSend.doam = h; 
        dataToSend.doat = soil; dataToSend.asang = sun;
        
        // [SỬA] Gửi thẳng cho c3tAddress thay vì broadcast
        esp_now_send(c3tAddress, (uint8_t *) &dataToSend, sizeof(dataToSend));
    }

    // Gửi Sensor MQTT (Chỉ khi có mạng)
    if (WiFi.status() == WL_CONNECTED && client.connected()) {
        if (currentMillis - lastMqttSensorTime > 2000) {
            lastMqttSensorTime = currentMillis;
            float h = dht.readHumidity(); float t = dht.readTemperature();
            if (isnan(h)) {h=0;t=0;}
            int sun = map(analogRead(SUN_PIN),0,4095,0,100);
            int soil = map(analogRead(SOIL_PIN),0,4095,0,100);
            snprintf(mqtt_full_payload, sizeof(mqtt_full_payload), 
              "{\"temp\":%.1f, \"hum\":%.1f, \"soil\":%d, \"sun\":%d, \"led_r\":%d, \"led_g\":%d, \"led_y\":%d}", 
              t, h, soil, sun, stt_red, stt_green, stt_yellow);
            client.publish(mqtt_topic_pub, mqtt_full_payload);
        }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS); 
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_RED, OUTPUT); pinMode(LED_GREEN, OUTPUT); pinMode(LED_YELLOW, OUTPUT);
  dht.begin(); pinMode(SOIL_PIN, INPUT); pinMode(SUN_PIN, INPUT);

  WiFi.mode(WIFI_AP_STA); 
  
  if (esp_now_init() != ESP_OK) ESP.restart();
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  // [SỬA] Đăng ký Peer bằng địa chỉ C3 (c3tAddress)
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo)); 
  memcpy(peerInfo.peer_addr, c3tAddress, 6); // Dùng địa chỉ thật
  peerInfo.channel = 1; 
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_AP; 
  esp_now_add_peer(&peerInfo);

  espClient.setInsecure(); client.setBufferSize(4096); 
  client.setServer(mqtt_server, mqtt_port); client.setCallback(callback);

  xTaskCreatePinnedToCore(TaskReadAndSend, "MainTask", 10000, NULL, 1, NULL, 1);
}

void loop() { vTaskDelay(1000); }