#include <regx52.h>

sbit rs = P3^7;
sbit rw = P3^6;
sbit en = P3^5;
#define congLCD P2 

void Delay_Slow(unsigned int t) {
    // Them tu khoa "volatile" de chong trinh bien dich xoa vong lap
    volatile unsigned int x, y; 
    for (x = 0; x < t; x++) {
        for (y = 0; y < 1000; y++); 
    }
}

void main() {
    while (1) {
        // BAT LED (Gia su LED noi xuong Mass thi xuat 1 la sang)
        // Neu ban noi LED len nguon thi sua thanh 0x00
        congLCD = 0xFF; 
        rs = 1; rw = 1; en = 1;
        
        Delay_Slow(500);

        // TAT LED
        congLCD = 0x00;
        rs = 0; rw = 0; en = 0;
        
        Delay_Slow(500);
    }
}