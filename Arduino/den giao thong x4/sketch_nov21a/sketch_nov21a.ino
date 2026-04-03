#define SH 12
#define DS 11
#define ST 10
#define CL1 9
#define CL2 8
#define CL11 6
#define CL22 5
#define DS1 7
#define SH1 4
#define ST1 3
#define BTN1 2
#define BTN2 0
#define BTN3 13
int t;
int Mode;
int timetruc1, timetruc2;
const byte den[6]={A0,A1,A2,A3,A4,A5};
char codeled[10] = {0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b1100110, 0b1101101, 0b1111101, 0b0000111, 0b1111111, 0b1101111};
int tx1=20;
int tx2=20;
int tv1=5;
int tv2=5;
unsigned long timedelay=millis();
   void dichbit(char b) 
  {
     for(int i=0;i<8;i++)
     {
      digitalWrite(DS,((b>>i)&0b0001)? HIGH: LOW);
      digitalWrite(SH,HIGH);
       digitalWrite(SH,LOW);
     }
     digitalWrite(ST,HIGH);
    digitalWrite(ST,LOW);
    
  }
   void dichbit2(char b) 
  {
     for(int i=0;i<8;i++)
     {
      digitalWrite(DS1,((b>>i)&0b0001)? HIGH: LOW);
      digitalWrite(SH1,HIGH);
       digitalWrite(SH1,LOW);
     }
     digitalWrite(ST1,HIGH);
    digitalWrite(ST1,LOW);
    
  }
  void controltruc1(int dvi,int chuc)
  {
     digitalWrite(CL11,HIGH);
     digitalWrite(CL22,LOW);
     dichbit2(codeled[dvi]);
     delay(2);
      dichbit2(0x00);
       digitalWrite(CL11,LOW);
     digitalWrite(CL22,HIGH);
     dichbit2(codeled[chuc]);
     delay(2);
  }
  void controltruc2(int dvi, int chuc)
  {
    digitalWrite(CL1,HIGH);
    digitalWrite(CL2,LOW);
    dichbit(codeled[dvi]);
    delay(2);
    dichbit(0x00);
     digitalWrite(CL1,LOW);
    digitalWrite(CL2,HIGH);
    dichbit(codeled[chuc]);
    delay(2);
  }
void setup() {
Serial.begin(9600);
pinMode(ST,OUTPUT);
pinMode(DS,OUTPUT);
pinMode(SH,OUTPUT);
pinMode(CL1,OUTPUT);
pinMode(CL2,OUTPUT);
pinMode(CL11,OUTPUT);
pinMode(CL22,OUTPUT);
pinMode(DS1,OUTPUT);
pinMode(ST1,OUTPUT);
pinMode(SH1,OUTPUT);
for(int i=0;i<=5;i++)
{
  pinMode(den[i],OUTPUT);
}
pinMode(BTN1,INPUT);
pinMode(BTN2,INPUT);
}
void x1sang()
{
  digitalWrite(A2,HIGH);
  digitalWrite(A0,LOW);
  digitalWrite(A1,LOW);
}
void d1sang()
{
  digitalWrite(A0,HIGH);
  digitalWrite(A2,LOW);
  digitalWrite(A1,LOW);
}
void v1sang()
{
  digitalWrite(A1,HIGH);
  digitalWrite(A0,LOW);
  digitalWrite(A2,LOW);
}
void d2sang()
{
  digitalWrite(A3,HIGH);
  digitalWrite(A4,LOW);
  digitalWrite(A5,LOW);
}
void v2sang()
{
  digitalWrite(A4,HIGH);
  digitalWrite(A3,LOW);
  digitalWrite(A5,LOW);
}
void x2sang()
{
  digitalWrite(A5,HIGH);
  digitalWrite(A4,LOW);
  digitalWrite(A3,LOW);
}
void DK_DEN()
{
  if(t<=tx1)
  {
    x1sang();
    d2sang();
  }else if(t>=tx1&&t<=tx1+tv1)
  {
    v1sang();
    d2sang();
  }else if(t>=tx1+tv1&&t<=tx1+tv1+tx2)
  {
    d1sang();
    x2sang();
  }else if(t>=tx1+tv1+tx2&&t<=tx1+tx2+tv1+tv2)
  {
    d1sang();
    v2sang();
  }
}
void DK_time()
{
  if(t==0)
  {
    timetruc1= tx1;
    timetruc2= tx1+tv1;
  } 
  if(t==tx1)
  {
    timetruc1= tv1;
    timetruc2=tv1;
  } 
  if(t==tx1+tv1)
  {
    timetruc1=tx2+tv2;
    timetruc2=tx2;
  } 
  if(t==tx1+tv1+tx2)
  {
    timetruc1= tv2;
    timetruc2=tv2;
  } 
  if(t==tx1+tx2+tv1+tv2)
  {
    t=0;
  }
}
void DK_LED(int time1, int time2)
{
  for(int i=0;i<10;i++)
  {
  controltruc1(time1%10,time1/10);
  controltruc2(time2%10,time2/10);
  }
}
void mode0()
{
  for(int i=0;i<6;i++)
  {
    digitalWrite(den[i],LOW);
  }
  digitalWrite(CL1,HIGH);
  digitalWrite(CL11,HIGH);
  digitalWrite(CL22,HIGH);
  digitalWrite(CL2,HIGH);
}
void mode1()
{
  if((unsigned long)(millis()-timedelay)>=1000)
  {
    t++;
    timetruc1--;
    timetruc2--;
    timedelay=millis();
  }
  DK_DEN();
  DK_time();
  DK_LED(timetruc1,timetruc2);
}
void LEDoff()
{
  digitalWrite(CL1,HIGH);
   digitalWrite(CL11,HIGH); 
    digitalWrite(CL22,HIGH);
     digitalWrite(CL2,HIGH);

}
void denoff()
{
  v1sang();
  v2sang();
}
void mode2()
{
  LEDoff();
  denoff();
}
bool isButtonPressed(int pin) {
  static unsigned long lastPressTime = 0;
  if (digitalRead(pin) == LOW) {
    if (millis() - lastPressTime >20) { 
      lastPressTime = millis();
      return true;
    }
  }
  return false;
}

void loop() {
 if (isButtonPressed(BTN1)) {
  Mode = 0;
}
if (isButtonPressed(BTN2)) {
  t = 0;
  Mode = 1;
}
if(isButtonPressed(BTN3))
{
  Mode=2;
}
 switch(Mode)
 {
  case 0:
  {
    mode0();
    break;
  }
  case 1:
  {
    mode1();
    break;
  }
  case 2:
  {
    mode2();
    break;
  }
  default :
  {
    mode0();
  }
 }
}
