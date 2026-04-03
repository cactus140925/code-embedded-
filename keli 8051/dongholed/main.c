#include <regx52.h>
#include "../lib/delay.h"

#define Led_port P2
sbit led1 = P3^0;
sbit led2 = P3^1;
sbit led3 = P3^2;
sbit led4 = P3^3;
sbit led5 = P3^4;
sbit led6 = P3^5;

char code7[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

void main() {
    unsigned int i;
    unsigned char gio, phut, giay;
    unsigned char chuc, dvi;

    gio = phut = giay = 0;

    while(1) {
        for (i = 0; i < 166; i++) {  // Ð? tr? 1 giây (166 l?n l?p nhân v?i delay)
            // Hi?n th? gi?
            chuc = gio / 10;
            dvi = gio % 10;
            Led_port = code7[chuc];
            led1 = 0;
            delay(1);
            led1 = 1;

            Led_port = code7[dvi];
            led2 = 0;
            delay(1);
            led2 = 1;

            // Hi?n th? phút
            chuc = phut / 10;
            dvi = phut % 10;
            Led_port = code7[chuc];
            led3 = 0;
            delay(1);
            led3 = 1;

            Led_port = code7[dvi];
            led4 = 0;
            delay(1);
            led4 = 1;

            // Hi?n th? giây
            chuc = giay / 10;  // S?a l?i: tính ch? s? hàng ch?c
            dvi = giay % 10;   // S?a l?i: tính ch? s? hàng don v?
            Led_port = code7[chuc];
            led5 = 0;
            delay(1);
            led5 = 1;

            Led_port = code7[dvi];
            led6 = 0;
            delay(1);
            led6 = 1;
        }

        giay++;
        if (giay == 60) {
            giay = 0;
            phut++;
            if (phut == 60) {
                phut = 0;
                gio++;
                if (gio == 24) {
                    gio = 0;
                }
            }
        }
    }
}
