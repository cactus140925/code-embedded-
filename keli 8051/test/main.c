#include<regx52.h>
#include"..\lib\delay.h"
int main()
{
	while(1)
	{
			P2=0x00;
		delay(1000);
		P2=0xFF;
		delay(500);
	}
}