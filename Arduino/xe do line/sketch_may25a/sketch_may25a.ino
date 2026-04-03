int matdoc[5]={A0,A1,A2,A3,A4};
void setup() {
  Serial.begin(9600);
 for(int i=0;i<5;i++)
 {
  pinMode(matdoc[i],INPUT);
 }

}

void loop() {
  int mat[5];
  for(int i=0;i<5;i++)
  {
   mat[i]=analogRead(matdoc[i]);
  }
   Serial.print("gia tri mat doc: ");
  for(int i=0;i<5;i++)
  {
  Serial.print(mat[i]);
  Serial.print(' ');
  }
  Serial.println();
  delay(1000);
}
