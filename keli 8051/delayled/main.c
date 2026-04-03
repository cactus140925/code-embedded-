#include <REGX52.H>
void delay(int n)
{
    int i, j;
    for(i = 0; i < n; i++)
    {
        for(j = 0; j < 123; j++);
    }
}

void main()
{
    while(1)
    {
        P2=0x00;
        delay(500);
			  delay(500);
			 P2=0xFF;
			 delay(500);
    }
}
