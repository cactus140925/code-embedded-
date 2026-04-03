#include <regx52.h>
#include "../lib/74595.h"
#include "../lib/delay.h"

char code7[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

void main() {
    while(1) {
        unsigned char x;
        char arr[2];
        for(x = 0; x < 100; x++) {  // Display from 00 to 99
            arr[0] = code7[x / 10];  // Tens digit
            arr[1] = code7[x % 10];  // Units digit
            IC74595(arr, 2);         // Send both digits to the 74595
            delay(1000);             // 1 second delay
        }
    }
}
