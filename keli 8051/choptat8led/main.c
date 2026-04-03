#include<REGX52.H>
#include"..\lib\delay.h"
sbit BTN= P1^0;
#define LED P2
bit PressedFlag = 0;

unsigned char Delay500msVaKiemTraPhim()
{
 	unsigned char i;
	for(i=0;i<50;i++)
	{
	 	delay(10);
		if(BTN==0)
		{
			if(PressedFlag == 0)
			{
				PressedFlag = 1;
			 	return 1;	
			}
		}
		else
		{
		 	PressedFlag = 0;
		}	
	}
	return 0;
}

void main()
{
	unsigned char i;

	while(1)
	{
		LED = 0x00;
		for(i=0;i<20;i++)
		{
		 	if(Delay500msVaKiemTraPhim()) break;
			LED = ~LED;
		}

		LED = 0x55;er
		for(i=0;i<20;i++)
		{
			if(Delay500msVaKiemTraPhim()) break;
			LED = ~LED;
		}

		LED = 0x0F;
		for(i=0;i<20;i++)
		{
		 	if(Delay500msVaKiemTraPhim()) break;
			LED = ~LED;
		}
	}
}