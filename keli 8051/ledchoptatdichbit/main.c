#include<regx51.h>
void delay(int n)
{
	int i,j;
	for(i=0;i<n;i++)
	{
		for(j=0;j<123;j++);
	}
}
void main()
{
	P2=0x80;
	while(1)
	{
		P2=P2>>1;
		delay(1000);
	}
}