#include<regx52.h>
#include"../lib/delay.h"
sbit RS = P2^0;
sbit E = P2^1;
#define D P3
sbit btn= P1^0;
char check =0;
void ledcontrol(unsigned char cmd)
{
	RS = 0;
	D = cmd;
	E = 1;
	E=0;
	if(cmd<=0x02)
	{
		delay(2);
	}
	else
	{
		delay(1);
	}
}
void ledin( unsigned char i)
{
	RS =1;
	D = i;
	E=1;
	E=0;
	delay(1);
}
void ledinti()
{
	ledcontrol(0x30);
	delay(3);
	ledcontrol(0x30);
	delay(1);
	ledcontrol(0x38);
	delay(1);
	ledcontrol(0x01);
	ledcontrol(0x0C);
}
void ledout(char str[])
{
	unsigned char i=0;
	while(str[i]!=0)
	{
		ledin(str[i]);
		i++;
	}
}
void buttoncontrol()
{
	if(btn==0&&check==0)// co nhan nut
	{
		ledinti();
	ledout("HELLO WORLD");
	ledcontrol(0xC0);
	ledout("DAT DZ VL");
		check=1;
		while(btn==0);
	}
	if(btn==0&&check==1)
	{
		ledcontrol(0x01);
		while(btn==0);
		check=0;
	}
	delay(20);
}
void main()
{
	
	while(1)
	{
		buttoncontrol();
	}
}
		