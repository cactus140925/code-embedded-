int e1=9,e2=10;
int in1 =8, in2=7,in3=6,in4=5;
int btn=4;
int dem=0;
int pre= LOW;
void setup() {
  Serial.begin(9600);
  pinMode(in1,OUTPUT);
  pinMode(in2,OUTPUT);
  pinMode(e1,OUTPUT);
  pinMode(in3,OUTPUT);
  pinMode(in4,OUTPUT);
  pinMode(e2,OUTPUT);
  pinMode(btn,INPUT);
}
void motor1dithang()
{
  digitalWrite(in1,HIGH);
  digitalWrite(in2,LOW);
  analogWrite(e1,255);

}
void motor2dithang()
{
  digitalWrite(in3,HIGH);
  digitalWrite(in4,LOW);
  analogWrite(e2,255);
}
void motor1dilui()
{
  digitalWrite(in1,LOW);
  digitalWrite(in2,HIGH);
  analogWrite(e1,255);
}
void motor2dilui()
{
  digitalWrite(in3,LOW);
  digitalWrite(in4,HIGH);
  analogWrite(e2,255);
}
void quachtrai()
{
  motor1dithang();
  motor2dilui();

}
void quachphai()
{
  motor2dithang();
  motor1dilui();
  dem=0;
}
void loop() {
  int docnutnhan= digitalRead(btn);
  if(docnutnhan==HIGH && pre==LOW)
  {
    dem=dem+1;
    delay(100);
  }
  pre=docnutnhan;
  if(dem==1)
  {
    motor1dithang();
    motor2dithang();
  }
  if(dem==2)
  {
    motor1dilui();
    motor2dilui();

  }
  if(dem==3)
  {
    quachtrai();
  }
  if(dem==4)
  {
    quachphai();
  }
}
