#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define X_PIN A1
#define S0 5  
#define S1 7  
#define S2 6  

const int muxMap[8] = {2,  1,  0,  3,  5,  7,  6,  4};

int sensorValues[8];
int minValues[8];    
int maxValues[8];    
int thresholds[8];   

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  
  for (int i = 0; i < 8; i++) {
    minValues[i] = 1023; 
    maxValues[i] = 0;    
  }
  
  lcd.setCursor(0, 0);
  lcd.print("Quet xe qua line");
  lcd.setCursor(0, 1);
  lcd.print("Trong 5 giay...");
  
  unsigned long startTime = millis();
  
  // --- QUÁ TRÌNH CALIB ---
  while (millis() - startTime < 5000) {
    for (int i = 0; i < 8; i++) {
      int channel = muxMap[i]; 
      
      digitalWrite(S0, bitRead(channel, 0));
      digitalWrite(S1, bitRead(channel, 1));
      digitalWrite(S2, bitRead(channel, 2));
      delayMicroseconds(50); 
      
      int val = analogRead(X_PIN);
      
      if (val < minValues[i]) minValues[i] = val;
      if (val > maxValues[i]) maxValues[i] = val;
    }
  }

  for (int i = 0; i < 8; i++) {
    thresholds[i] = (minValues[i] + maxValues[i]) / 2;
  }
  
  lcd.clear();
  lcd.print("Calib Xong!");
  delay(1000);
  lcd.clear();
}

void loop() {
  int activeSensors = 0; 
  float weightedSum = 0; 

  // --- QUÁ TRÌNH ĐỌC CẢM BIẾN ---
  for (int i = 0; i < 8; i++) {
    int channel = muxMap[i]; 
    
    digitalWrite(S0, bitRead(channel, 0));
    digitalWrite(S1, bitRead(channel, 1));
    digitalWrite(S2, bitRead(channel, 2));
    delayMicroseconds(50); 
    
    sensorValues[i] = analogRead(X_PIN);
    
    if (sensorValues[i] > thresholds[i]) {
      activeSensors++;
      weightedSum += i;
    }
  }

  // --- HIỂN THỊ LCD ---
  lcd.setCursor(0, 0);
  lcd.print("Line: [");
  for (int i = 0; i < 8; i++) {
    if (sensorValues[i] > thresholds[i]) lcd.print("X"); 
    else lcd.print("_");                             
  }
  lcd.print("]");

  // --- XỬ LÝ LOGIC HƯỚNG ĐI ---
  lcd.setCursor(0, 1);
  
  if (activeSensors == 0) {
    lcd.print(">> MAT LINE <<  "); 
  } 
  else if (activeSensors >= 6) {
    lcd.print("Huong: NGA BA   "); 
  }
  else {
    float position = weightedSum / activeSensors;
    if (position < 3.0) {
      lcd.print("Huong: TRAI     "); 
    } 
    else if (position > 4.0) {
      lcd.print("Huong: PHAI     "); 
    } 
    else {
      lcd.print("Huong: THANG    "); 
    }
  }
  
  delay(100); 
}
