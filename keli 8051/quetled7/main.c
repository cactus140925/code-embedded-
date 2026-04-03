#include <regx52.h>
#include "../lib/delay.h"

sbit led1 = P0^0;     // LED cho ch? s? hàng ch?c
sbit led2 = P0^1;     // LED cho ch? s? hàng don v?
#define LED7 P3       // LED 7 do?n

char code7[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

void main() {
    unsigned int i;
    unsigned char dem = 0, chuc, dvi;

    while(1) {
        for(dem = 0; dem < 100; dem++) {
            chuc = dem / 10;    // Tính ch? s? hàng ch?c
            dvi = dem % 10;     // Tính ch? s? hàng don v?

            for(i = 0; i < 500; i++) {   // Duy?t 500 l?n d? gi? hi?n th?
                // Hi?n th? hàng ch?c
                LED7 = code7[chuc];
                led1 = 0;   // B?t LED cho hàng ch?c
                delay(1);   // Gi? LED trong 1ms
                led1 = 1;   // T?t LED cho hàng ch?c

                // Hi?n th? hàng don v?
                LED7 = code7[dvi];
                led2 = 0;   // B?t LED cho hàng don v?
                delay(1);   // Gi? LED trong 1ms
                led2 = 1;   // T?t LED cho hàng don v?
            }
        }
    }
}
