#include<regx52.h>
#include"../lib/delay.h"
sbit sh = P2^0;
sbit ds = P2^1;
sbit st = P2^2;
void test(unsigned char b)
{
	char i;
	for(i=0;i<8;i++)
	{
		ds = b & (0x80>>i);
		// tao xung nhan dl
		sh = 0;
		sh=1;
	}
	//tao xung xuat dl
	st=0;
	st=1;
}
void main()
{
	unsigned char a = 0x01;
	while(1)
	{
		
		test(a);
		delay(500);
		a=a<<1;
		if(a==0x00)
		{
			a=0x01;
		}
	}9
		
	}
	
		
	