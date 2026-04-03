#include<main.h>
#include"..\Objects\STARTUP.obj"
unsigned char cnt =10;
void main()
{
	// bat timer 0
	TMOD&=0Xf0;
	TMOD |= 0x01;
	// cho phep timer0 dem
	TL0=0Xb0;
	TH0=0x3c;
	//cho phep ngat
	ET0=1;
	EA=1;
	TR0=1;
	while(1)
	{
	}
}
void ngatngoaitimer(void) interrupt 1
{
	TL0=0Xb0;
	TH0=0x3c;
	cnt--;
	if(cnt==0)
	{
		cnt=10;
		P2=~P2;
	}
}