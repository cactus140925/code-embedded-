#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
RTC_DS1307 rtc;

// LCD: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(12,11,10,9,8,7);

// Nút nhấn
int chedo = A0;
int up     = 6;
int enter  = 5;
int down   = A1;
int buzz   = 2;

#define Mode1   0
#define Modeset 1

const char* daysOfWeek[] = {"CN","T2","T3","T4","T5","T6","T7"};

int mode = Mode1;
int setIndex = 0;   // 0=day,1=hour,2=min

int alarmDay = 1;
int alarmHour = 6;
int alarmMinute = 0;

long timeRef = 0;
long refreshTime = 300; // LCD update mỗi 300ms

// -------------------------------
bool checkbutton(int pin) {
  if (digitalRead(pin) == HIGH) {   // dùng INPUT_PULLUP
    delay(20);
    while (digitalRead(pin) == HIGH);
    return true;
  }
  return false;
}
// ===== Lưu báo thức vào EEPROM =====
void saveAlarmToEEPROM() {
  EEPROM.write(0, alarmDay);
  EEPROM.write(1, alarmHour);
  EEPROM.write(2, alarmMinute);
}

// ===== Đọc báo thức từ EEPROM =====
void loadAlarmFromEEPROM() {
  alarmDay    = EEPROM.read(0);
  alarmHour   = EEPROM.read(1);
  alarmMinute = EEPROM.read(2);

  // Kiểm tra lỗi và gán giá trị mặc định
  if (alarmDay < 1 || alarmDay > 31)   alarmDay = 1;
  if (alarmHour > 23)                  alarmHour = 6;
  if (alarmMinute > 59)                alarmMinute = 0;
}
// -------------------------------
void doctime() {
  DateTime now = rtc.now();

  lcd.setCursor(0,0);
  lcd.print(daysOfWeek[now.dayOfTheWeek()]);
  lcd.print(" ");

  if (now.hour() < 10) lcd.print("0");
  lcd.print(now.hour()); lcd.print(":");

  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute()); lcd.print(":");

  if (now.second() < 10) lcd.print("0");
  lcd.print(now.second());

  lcd.setCursor(0,1);
  if(now.day() < 10) lcd.print("0");
  lcd.print(now.day()); lcd.print("/");

  if(now.month() < 10) lcd.print("0");
  lcd.print(now.month()); lcd.print("/");

  lcd.print(now.year());

  // Kiểm tra báo thức
  if (now.day() == alarmDay &&
      now.hour() == alarmHour &&
      now.minute() == alarmMinute) 
  {
    digitalWrite(buzz, HIGH);
  } 
  else {
    digitalWrite(buzz, LOW);
  }
}

// -------------------------------
void setclock() {
  lcd.setCursor(0,0);
  lcd.print("SET ALARM       ");

  lcd.setCursor(0,1);

  // Day
  if(setIndex==0) lcd.print(">Day");
  else lcd.print("Day");
  lcd.print(alarmDay);
  // Hour
  if(setIndex==1) lcd.print(">Hr ");
  else lcd.print(" Hr");
  lcd.print(alarmHour);

  // Minute
  if(setIndex==2) lcd.print(">Min");
  else lcd.print(" Mi");
  lcd.print(alarmMinute);
  lcd.print("---");

  // =========== UP ===========
  if (checkbutton(up)) {
    if (setIndex == 0) {
      alarmDay++;
      if (alarmDay > 31) alarmDay = 1;
    }
    else if (setIndex == 1) {
      alarmHour++;
      if (alarmHour > 23) alarmHour = 0;
    }
    else if (setIndex == 2) {
      alarmMinute++;
      if (alarmMinute > 59) alarmMinute = 0;
    }
  }

  // =========== DOWN ===========
  if (checkbutton(down)) {
    if (setIndex == 0) {
      alarmDay--;
      if (alarmDay < 1) alarmDay = 31;
    }
    else if (setIndex == 1) {
      alarmHour--;
      if (alarmHour < 0) alarmHour = 23;
    }
    else if (setIndex == 2) {
      alarmMinute--;
      if (alarmMinute < 0) alarmMinute = 59;
    }
  }

  // =========== ENTER (Next) ===========
  if (checkbutton(enter)) {
    setIndex++;
    if (setIndex > 2) setIndex = 0;
  }

  // =========== CHẾ ĐỘ (Lưu & Thoát) ===========
  if (checkbutton(chedo)) {
     saveAlarmToEEPROM();   // <<< LƯU VÀO EEPROM
     lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Alarm Saved!");
    delay(100);
    mode = Mode1;
    lcd.clear();
    delay(150);
  }
}

// -------------------------------
void setup() {
  Wire.begin();
  lcd.begin(16,2);

  pinMode(chedo, INPUT);
  pinMode(up, INPUT);
  pinMode(down, INPUT);
  pinMode(enter, INPUT);
  pinMode(buzz, OUTPUT);

  if (!rtc.begin()) {
    lcd.print("RTC ERROR");
    while(1);
  }
 loadAlarmFromEEPROM();
  delay(1000);
  lcd.clear();
}

// -------------------------------
void loop() {

  if (mode == Mode1) {
    if (millis() - timeRef > refreshTime) {
      doctime();
      timeRef = millis();
    }

    if (checkbutton(chedo)) {
      mode = Modeset;
      setIndex = 0;
      lcd.clear();
    }
  }

  else if (mode == Modeset) {
    setclock();
  }

}
