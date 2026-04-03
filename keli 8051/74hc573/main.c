#include<regx52.h>
#include"../lib/delay.h"
sbit LE1= P2^0;
sbit LE2= P2^1;
char code7[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
void main()
{
	int i;
	while(1)
	{
		LE1=LE2=0;
		for(i=0;i<=99;i++)
		{
			P0 = code7[i/10];
			LE1=1;
			LE1=0;
			P0 = code7[i%10];
			LE2=1;
			LE2=0;
			delay(1000);
		}
	}
}