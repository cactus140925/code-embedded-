#include <regx52.h>
#include "../lib/delay.h"

sbit btn = P1^0;
sbit led = P2^0;

void main()
{
    while(1)
    {
        if(btn == 0) // N?u nút nh?n du?c nh?n (m?c logic th?p)
        {
            led = 1; // B?t dèn LED
        }
        else
        {
            led = 0; // T?t dèn LED
        }
    }
}
