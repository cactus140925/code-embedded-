char keyp()
{
	char key =0;
	C1=0;
	if(R1==0) key=1;
	if(R2==0) key=2;
	if(R3==0) key=3;
	if(R4==0) key=4;
	C1=1;
	C2=0;
	if(R1==0) key=5;
	if(R2==0) key=6;
	if(R3==0) key=7;
	if(R4==0) key=8;
	C2=1;
	C3=0;
	if(R1==0) key=9;
	if(R2==0) key=10;
	if(R3==0) key=11;
	if(R4==0) key=12;
	C3=1;
	C4=0;
	if(R1==0) key=13;
	if(R2==0) key=14;
	if(R3==0) key=15;
	if(R4==0) key=16;
  C4=1;
	return key;
}