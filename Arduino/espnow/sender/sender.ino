#include <esp_now.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- THAY MAC CỦA S3 VÀO ĐÂY ---
uint8_t broadcastAddress[] = {0xDC, 0xB4, 0xD9, 0x05, 0xC2, 0x0C}; 

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

bool deviceConnected = false;
BLECharacteristic *pTxCharacteristic;

// --- BIẾN CỜ HIỆU ĐỂ TRÁNH XUNG ĐỘT ---
bool newCommand = false;     // Cờ báo có lệnh mới
bool targetLedState = false; // Trạng thái muốn gửi

typedef struct struct_message {
  bool ledStatus;
} struct_message;
struct_message myData;
esp_now_peer_info_t peerInfo;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      BLEDevice::startAdvertising();
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        Serial.print("Bluetooth nhận: ");
        Serial.println(value[0]);

        // Thay vì gửi ngay, ta chỉ lưu trạng thái và bật cờ
        if (value[0] == '1') {
          targetLedState = true;
          newCommand = true; // Báo hiệu cho loop() biết
        } else if (value[0] == '0') {
          targetLedState = false;
          newCommand = true; // Báo hiệu cho loop() biết
        }
      }
    }
};

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Hàm này giờ sẽ chạy ổn định hơn
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println(">> Gửi OK: S3 đã nhận!");
  } else {
    Serial.println(">> Gửi LỖI: S3 không phản hồi.");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Lỗi ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
  
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Lỗi thêm Peer");
    return;
  }

  // BLE Setup
  BLEDevice::init("ESP32-C3-Final");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                            CHARACTERISTIC_UUID_RX,
                                            BLECharacteristic::PROPERTY_WRITE
                                          );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  
  BLEDevice::startAdvertising();
  
  Serial.println("--- HỆ THỐNG SẴN SÀNG ---");
}

void loop() {
  // KIỂM TRA CỜ HIỆU Ở ĐÂY (Nơi an toàn để gửi WiFi)
  if (newCommand) {
    newCommand = false; // Tắt cờ
    
    // Thực hiện gửi
    myData.ledStatus = targetLedState;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    // Kiểm tra ngay kết quả lệnh gửi
    if (result == ESP_OK) {
      Serial.println("Đang gửi lệnh sang S3...");
    } else {
      Serial.print("Lỗi lệnh gửi: ");
      Serial.println(result); // In mã lỗi nếu có
    }
  }
  
  delay(10); // Nghỉ nhẹ để giữ ổn định
}