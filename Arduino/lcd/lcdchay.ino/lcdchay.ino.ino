#include <LiquidCrystal.h>
LiquidCrystal lcd(12,11,7,6,5,4);
int i;
#define VO A1
void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);

}

void loop() {
  // put your main code here, to run repeatedly:
  for(i=15;i>(-15);i--)
  {
  lcd.setCursor(i,0);
  lcd.print("Dat dz vl");
  lcd.setCursor(i,1);
  lcd.print("YEu em");
  delay(300);
  lcd.clear();
  }
}
