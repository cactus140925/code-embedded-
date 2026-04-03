#include<regx52.h>
unsigned char codeled7[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
#define CL1 P1_0
#define CL2 P3_1
#include<../lib/delay.h>
void display(number)
{ CL1=0;
	P2=codeled7[number/10];
  delay(2);
	CL1=1;
	CL2=0;
	P2=codeled7[number%10];
	delay(2);
	CL2=1;
	}
void main()
{
	unsigned char number, high, low;
	TMOD&=0Xf0;
	TMOD|=0X05;
	TR0=1;
	while(1)
	{
	high=TH0;
	low=TL0;
	number = high;
	number<<8;
	number=number|low;
	display(number);
	}
}
