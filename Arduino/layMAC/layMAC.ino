#include "WiFi.h"

void setup() {
  WiFi.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);
  Serial.println(WiFi.macAddress());
}

void loop() {}