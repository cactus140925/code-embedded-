#include <Wire.h>
#include <LiquidCrystal.h>
#include "RTClib.h"
RTC_DS1307 rtc;
LiquidCrystal lcd(12,11,7,6,5,4);
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
void setup ()
{
Serial.begin(9600);
 lcd.begin(16,2);
 if (! rtc.begin())
 {
  lcd.setCursor(4,0);
   lcd.print("Couldn't find RTC");
   while (1);
 }
  if (! rtc.isrunning())
 {
  lcd.setCursor(4,0);
   lcd.print("RTC is NOT running!");
   lcd.println();
 }
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
}
void chill ()
{
   DateTime now = rtc.now(); 
   lcd.setCursor(0,0);
   if(now.hour()<=9)
   {
     lcd.print("0");
     lcd.print(now.hour());
   }
   else {
    lcd.print(now.hour());
   }
   lcd.print(':');
   if(now.minute()<=9)
   {
     lcd.print("0");
     lcd.print(now.minute());
   }
   else {
   lcd.print(now.minute());
   }
  lcd.print(':');
   if(now.second()<=9)
   {
     lcd.print("0");
     lcd.print(now.second());
   }
   else {
    lcd.print(now.second());
   }
  lcd.setCursor(0,1);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(",");
   if(now.day()<=9)
   {
    lcd.print("0");
    lcd.print(now.day());
   }
   else {
   lcd.print(now.day());
   }
  lcd.print('/');
   if(now.month()<=9)
   {
    lcd.print("0");
    lcd.print(now.month());
   }
   else {
   lcd.print(now.month());
   }
  lcd.print('/');
   if(now.year()<=9)
   {
    lcd.print("0");
    lcd.print(now.year());
   }
   else {
   lcd.print(now.year());
   } 
  lcd.println();
   delay(1000);
}
void loop()
{

}