#include<regx52.h>
#include"../lib/delay.h"
#define LED2 P2
#define LED1 P1
#define LED0 P0
void mode0()
{
	int i;
	LED0=0xFE;
	for(i=1;i<8;i++)
	{
		LED0=0xfe<<i;
		delay(400);
	}
	LED2=0xFE;
	for(i=1;i<8;i++)
	{
		LED2=0xfe<<i;
		delay(400);
	}
	P3=0XFE;
	P3_1=0;
	delay(400);
	P3_3=0;
	delay(400);
	P3_4=0;
	delay(400);
	P3_5=0;
	delay(400);
	P3_6=0;
	delay(400);
	P3_7=0;
	delay(400);
	LED1=0xFE;
	for(i=1;i<8;i++)
	{
		LED1=0xfe<<i;
		delay(400);
	}
	LED0=LED2=0Xff;
	P3=0xff;
	LED1=0xFF;
	for(i=0;i<5;i++)
	{
		LED0=LED2=0X00;
	P3 &= (0x01 << 2);
	LED1=0X00;
		delay(3000);
	LED0=LED2=LED1=P3=0Xff;
	 delay(3000);
 }	 
}
void mode1()
{
	LED0=LED1=LED2=0X00;
	P3 &= (0x01 << 2);
	delay(3000);
	LED0=LED2=0x00;
	LED1=0xff;
	P3_1= 1;
	P3_1=1;
	P3_1=1;
	P3_1=1;
	P3_1=1;
	P3_1=1;
}
void main()
{
	EA=1;
	EX0=1;
	PX0=1;
	IT0=1;
	while(1)
	{
		mode1();
	}
}
void ngatngoai()interrupt 0
{
	while(1)
	{
		LED0=LED2=LED1=P3=0Xff;
		mode0();
		delay(500);
	}
}
	
