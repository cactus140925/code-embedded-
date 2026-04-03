#include <Servo.h>
#include <LiquidCrystal.h>
int ir =3;
int ir1=9;
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);
Servo sv;
  int count = 0;
  long long time =0;
void setup() {
  lcd.begin(16,2);
  sv.attach(A0);
  pinMode(ir,INPUT);
  pinMode(ir1,INPUT);
  sv.write(0);
  lcd.clear();
  lcd.print("so nguoi:");
  lcd.print(count);
  lcd.setCursor(0,1);
  lcd.print("cua dong");
}
 const long check = 800;
 int check1 =0;
 const long timemo= 3000;
   long long timedong=0;
void loop() {
  long long current = millis();
  if(count<10){
    if(current - time > check)
    {
    if(digitalRead(ir)==1||digitalRead(ir1)==1)
    {
      sv.write(180);
      check1=1;
      timedong = millis();
        if(digitalRead(ir1)==1&&digitalRead(ir)==0)
        {
          count++;
        }
         if(digitalRead(ir1)==0&&digitalRead(ir)==1)
         {
          count--;
         }
      }
      time = millis();
    }
    if((check1==1)&&(millis()-timedong<3000))// lam cho cua mo trong 3s
    {
     lcd.setCursor(0, 0);
     lcd.print("so nguoi:      ");  
     lcd.setCursor(10, 0);          
    lcd.print(count);
    lcd.setCursor(0,1);
    lcd.print("cua mo      ");
    }else{
      sv.write(0);
     lcd.setCursor(0, 0);
     lcd.print("so nguoi:      ");  
     lcd.setCursor(10, 0);          
    lcd.print(count);
    lcd.setCursor(0,1);
    lcd.print("cua dong    ");
    check1 =0;
    }
  }else
  {
     lcd.clear();
     lcd.print("so nguoi :10(F)");
     lcd.setCursor(0,1);
     lcd.print("cua dong");
     delay(1000);
     sv.detach();
  }
}
