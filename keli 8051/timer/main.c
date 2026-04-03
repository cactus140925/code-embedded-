#include<regx52.h>
#define LED P2_1
void delaytimer(int t)
{
	do{
	TL0=0x18;
	TH0=0xFC;
	TR0=1;
	while(!TF0);
	TR0=0;
	TF0=0;
	t--;
	}while(t>0);
}
void main()
{
	TMOD&=0XF0; //set 4 bit thap cho timer0 va giu nguyen 4 bit cao
	TMOD|=0X01;  // set mode 1 cho timer0 M0=1 VA M1=0
	while(1)
	{
		LED=!LED;
		delaytimer(1000);
	}
}