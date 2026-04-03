#include <regx52.h>
#include "../lib/delay.h"
sbit E = P2^0;
sbit RS = P2^1;
#define lcd P3
sbit btn = P1	^0;
unsigned char check=0;
void lcdcontrol(unsigned char c)
{
    RS = 0;
    lcd = c;
    E=1;
	E=0;
   if(c<=0x02)
	{
		delay(2);
	}
	else
	{
		delay(1);
	}
}

void lcdinti()
{
    lcdcontrol(0x30);
    delay(1);
    lcdcontrol(0x30);
    delay(3);
    lcdcontrol(0x38);
    delay(1);
	lcdcontrol(0x01);
	lcdcontrol(0x0C);
}

void lcdoutchar(unsigned char c)
{
    RS = 1;
    lcd = c;
    E = 1;
    E = 0;
    delay(1);
}
void lcdstring(char* str)
{
	int i=0;
	while(str[i]!=0)
	{
		lcdoutchar(str[i]);
		i++;
	}
}
void lcdout(unsigned char row, unsigned char col, unsigned char c)
{
    unsigned char cmd;
    cmd = (row == 1 ? 0x80 : 0xC0) + col - 1;
    lcdcontrol(cmd);
    lcdoutchar(c);
}
void lcdoutstr(unsigned char row, unsigned char col, char* str)
{
	 unsigned char cmd;
    cmd = (row == 1 ? 0x80 : 0xC0) + col - 1;
    lcdcontrol(cmd);
	   lcdstring(str);
}
	void btncontrol(unsigned char row, unsigned char col, char* str)
	{
		if(btn==0&&check==0)
		{
			lcdinti();
			lcdoutstr(row,col,str);
			while(btn==0);
			check=1;
		}
		if(btn==0&&check==1)
		{
			lcdcontrol(0x01);
			while(btn==0);
			check=0;
		}
		delay(20);
	}
void main()
{
    while (1) {
			btncontrol(2,4,"dat dz vl");
    }
}
