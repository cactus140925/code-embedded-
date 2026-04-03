#include<regx52.h>
#include"../lib/delay.h"
unsigned char quet[]={0xF8,0XF9,0XFA,0XFB,0XFC,0XFD,0XFE,0XFF};
void main()
{
	int i;
	while(1)
	{
	for(i=0;i<8;i++)
	{
		P2 = quet[i];
	delay(1000);
	}
}
}