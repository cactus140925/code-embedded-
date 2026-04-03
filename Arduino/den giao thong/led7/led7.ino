const byte led17447[4] = {13, 12, 11, 10}; 
const byte led27447[4] = {9,8,7,6}; 
char codeled[10] = {0b0000,0b0001,0b0010,0b0011,0b0100,0b0101,0b0110,0b0111,0b1000,0b1001};
#define SH 5
#define DS 4
#define ST 3
char codeden[3]={0b0001,0b0010,0b0100};
void setup() {
  Serial.begin(9600);
  
  for (int i = 0; i < 4; i++) {
    pinMode(led17447[i], OUTPUT);
     pinMode(led27447[i], OUTPUT);
  }
  pinMode(SH,OUTPUT);
  pinMode(DS,OUTPUT);
  pinMode(ST,OUTPUT);
}
 int chuc=0,dvi;
void chayled(const byte led[4],int so)
{
    for (int i = 0; i < 4; i++) {
       digitalWrite(led[i], ((codeled[so] >> i) & 0b0001) ? HIGH : LOW);
      
    }
    
  }
  void dichbit(char b) //74hc 595
  {
     for(int i=0;i<3;i++)
     {
      digitalWrite(DS,((b>>i)&0b0001)? HIGH: LOW);
      digitalWrite(SH,HIGH);
       digitalWrite(SH,LOW);
     }
     digitalWrite(ST,HIGH);
    digitalWrite(ST,LOW);
  }
void loop() {
for(int i=0;i<3;i++)
{
  chuc=0;
  while(chuc<50)
  {
  dichbit(codeden[i]);
  dvi=chuc%10;
  chuc++;
  chayled(led17447,chuc/10);
  chayled(led27447,dvi);
  delay(980);
  }
}
}
