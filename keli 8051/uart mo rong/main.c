#include<regx52.h>
#include"..\lib\delay.h"
void uartmode0(char c) 
{

	SBUF = c;
	while(TI==0);
	TI=0;
	P3_2=0;
	P3_2=1;
}
void main()
{
	SM0= SM1=0;
	
	while(1)
	{
		uartmode0(0x00);
		delay(500);
		uartmode0(0xFF);
		delay(500);
	}
}
	