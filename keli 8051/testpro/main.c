#include<regx52.h>
#include"../lib/delay.h"

sbit E = P3^0;        // Chân E n?i v?i P3.0
sbit RS = P3^1;       // Chân RS n?i v?i P3.1
#define lcd P2        // C?ng P2 n?i v?i LCD data

sbit IC_ST = P3^4;    // Chân ST c?a IC 74HC595 n?i v?i P3.4
sbit IC_SH = P3^3;    // Chân SH c?a IC 74HC595 n?i v?i P3.3
sbit IC_DS = P3^5;    // Chân DS c?a IC 74HC595 n?i v?i P3.5

unsigned char maled[] = {0x84, 0x6C, 0x7E, 0x3F, 0x3E, 0x7C, 0x6C, 0x84};
void lcdcontrol(unsigned char cmd) {
    RS = 0;
    lcd = cmd;
    E = 0;
    E = 1;
    delay(10);
    if(cmd <= 0x20) {
        delay(20);
    } else {
        delay(10);
    }
}

void lcdinti() {
    lcdcontrol(0x30);
    delay(3);
    lcdcontrol(0x30);
    delay(1);
    lcdcontrol(0x38);
    delay(1);
    lcdcontrol(0x01);
    delay(3);
    lcdcontrol(0x0C);
}

void lcdoutchar(char c) {
    RS = 1;
    lcd = c;
    E = 0;
    E = 1;
    delay(1);
}

void lcdstring(char* str) {
    unsigned char i = 0;
    while(str[i] != 0) {
        lcdoutchar(str[i]);
        i++;
    }
}

void IC74595(unsigned char *p, unsigned char n) {
    unsigned char i, j, b;

    for(i = 0; i < n; i++) {
        b = *(p + n - i - 1);
        for(j = 0; j < 8; j++) {
            IC_DS = b & (0x80 >> j);
            IC_SH = 0;
            IC_SH = 1;
        }
        P0 ^= (1 << i); 
        IC_ST = 0;  
        IC_ST = 1;
        delay(1);
        P0 ^= (1 << i); 
    }
}

void main() {
    unsigned char x,i=30;
    lcdinti();
    while(1) {
        IC74595(maled, 8);
        lcdstring("CHUC MUNG QUOC KHANH 2/9");
        lcdcontrol(0xC0);
        lcdstring("TOI YEU VIET NAM");
        lcdcontrol(0x80);
        delay(1000);
			while(i--)
					{
				 IC74595(maled, 8);
				}
        for(x = 0; x < 9; x++) {
            lcdcontrol(0x18);
            delay(200);
        }
				 delay(4000);
				while(i--)
					{
				 IC74595(maled, 8);
				}
        delay(500);
        lcdcontrol(0x01);
    }
}
