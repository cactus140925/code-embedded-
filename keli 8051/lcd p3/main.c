#include<regx52.h>
#include"../lib/delay.h"
sbit RS = P2^0;
sbit E = P2^1;
#define lcd P3
void lcdcontrol(unsigned char c)
{
	RS=0;
	lcd = c;
	E=1;
	E=0;
	if(c<=0x20)
	{
		delay(20);
	}else
	{
		delay(10);
	}
}
void lcdinti()
{
	lcdcontrol(0x30);
	delay(3);
	lcdcontrol(0x30);
	delay(1);
	lcdcontrol(0x30);
	lcdcontrol(0x38);
	lcdcontrol(0x01);
	delay(1);
	lcdcontrol(0x0C);
}
void lcdchar(char c)
{
	RS =1;
	lcd = c;
	E=1;
	E=0;
	delay(1);
}
void lcdstring(char* str)
{
	unsigned char i=0;
	while(str[i]!=0)
	{
		lcdchar(str[i]);
		i++;
	}
	
}
void main()
{	
	unsigned char i;
	lcdinti();
	while(1){
	char str[100]= "TOI YEU NUOC VIET NAM";
	lcdstring(str);
		delay(1000);
	for(i=0;i<7;i++)
	{
		lcdcontrol(0x1C);
		delay(300);
	}
	lcdcontrol(0x02);
}
	
}