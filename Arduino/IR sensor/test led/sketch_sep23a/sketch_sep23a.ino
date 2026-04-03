#define led 8
#define cambien A0

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(led,OUTPUT);
  pinMode(cambien,INPUT);


}

void loop() {
  digitalWrite(led,LOW);
  int doc= digitalRead(cambien);
  if(doc==HIGH)
  {
    digitalWrite(led,HIGH);
  }
  if(doc==LOW)
  {
  digitalWrite(led,LOW);
  }
  Serial.print(doc);
}
