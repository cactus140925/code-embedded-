#include<regx52.h>
#include"../lib/delay.h"
char code7[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
sbit btn= P1^0;
#define Led7 P2
void main()
{
	int dem=0;
	while(1)
	{ 
		
		while(btn==0)
		{
			dem++;
			delay(50);
		}
			Led7 = code7[dem%9+1];
	}
}
			