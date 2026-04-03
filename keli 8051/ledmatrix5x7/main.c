#include <regx52.h>
#include "../lib/delay.h"

char codeled[] = {0x02, 0x06, 0x0A, 0x0A, 0x0A, 0x06, 0x02}; 

void main() {
    while(1) {
        P0 = codeled[0];
        P3_0 = 0; // B?t LED 1
        delay(1);
        P3_0 = 1; // T?t LED 1

        P0 = codeled[1];
        P3_1 = 0; // B?t LED 2
        delay(1);
        P3_1 = 1; // T?t LED 2

        P0 = codeled[2];
        P3_2 = 0; // B?t LED 3
        delay(1);
        P3_2 = 1; // T?t LED 3

        P0 = codeled[3];
        P3_3 = 0; // B?t LED 4
        delay(1);
        P3_3 = 1; // T?t LED 4

        P0 = codeled[4];
        P3_4 = 0; // B?t LED 5
        delay(1);
        P3_4 = 1; // T?t LED 5

        P0 = codeled[5];
        P3_5 = 0; // B?t LED 6
        delay(1);
        P3_5 = 1; // T?t LED 6

        P0 = codeled[6];
        P3_6 = 0; // B?t LED 7
        delay(1);
        P3_6 = 1; // T?t LED 7
    }
}
