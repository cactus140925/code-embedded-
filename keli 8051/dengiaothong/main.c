#include <regx52.h>
#include "..\lib\delay.h"


sbit led_do = P0^0;
sbit led_xanh = P0^1;
sbit led_vang = P0^2;

#define Led_chuc P2
#define Led_Dvi P3

char code7[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

void main() {
    int i;
    while(1) {
        led_vang = 1;
        led_do = 0; // LED d? b?t
        for(i = 20; i > 0; i--) {
            Led_chuc = code7[i / 10];
            Led_Dvi = code7[i % 10];
            delay(1000);
        }
        led_do = 1;
        led_xanh = 0; // LED xanh b?t
        for(i = 20; i > 0; i--) {
            Led_chuc = code7[i / 10];
            Led_Dvi = code7[i % 10];
            delay(1000);
        }
        led_xanh = 1;
        led_vang = 0; // LED vàng b?t
        for(i = 20; i > 0; i--) {
            Led_chuc = code7[i / 10];
            Led_Dvi = code7[i % 10];
            delay(1000);
        }
    }
	}