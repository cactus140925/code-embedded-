int i =1;
void setup()
{
  pinMode(i,OUTPUT);
}
void loop()
{
  digitalWrite(i,HIGH);
  delay(5000);
  digitalWrite(i,LOW);
  delay(5000);
}