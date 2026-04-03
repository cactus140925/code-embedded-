#include <main.h> // AT89C52
#include "../lib/delay.h"

#define SH P2_0
#define DS P2_1
#define ST P2_2
#define CL1 P2_3
#define CL2 P2_4
char codeled7[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

void IC74595(unsigned char b) {
    unsigned char j;
    for (j = 0; j < 8; j++) {
        DS = (b & (0x80 >> j)) ? 1 : 0; // D?ch t?ng bit
        SH = 0;
        SH = 1;
    }
    ST = 0;
    ST = 1;
}

void displayLED(unsigned char digit1, unsigned char digit2) {
    // Hi?n th? ch? s? hàng don v?
    IC74595(codeled7[digit1]);
    CL1 = 1;
    CL2 = 0;
    delay(5);

    // Hi?n th? ch? s? hàng ch?c
    IC74595(codeled7[digit2]);
    CL1 = 0;
    CL2 = 1;
    delay(5);
}

void main() {
    unsigned char count = 0;

    while (1) {
       P2=0XC0;
}
