#ifndef _DHT11_H_
#define _DHT11_H_
#include <reg52.h>
#include "Delay.h"

sbit DHT11_PIN = P3^0;
int I_RH, D_RH, I_Temp, D_Temp; // Bien luu ket qua

void DHT11_Start() {
    DHT11_PIN = 1; Delay_10us();
    DHT11_PIN = 0; Delay_ms(20);
    DHT11_PIN = 1;      
}

unsigned char DHT11_Check() {
    unsigned int timeout = 0;
    while (DHT11_PIN == 1) if (timeout++ > 500) return 0;
    timeout = 0;
    while (DHT11_PIN == 0) if (timeout++ > 500) return 0;
    timeout = 0;
    while (DHT11_PIN == 1) if (timeout++ > 500) return 0;
    return 1;
}

unsigned char DHT11_ReadByte() {
    unsigned char i, dat = 0;
    for (i = 0; i < 8; i++) {
        while (DHT11_PIN == 0);
        Delay_10us(); Delay_10us(); Delay_10us();
        if (DHT11_PIN == 1) {
            dat |= (1 << (7 - i));
            while (DHT11_PIN == 1);
        }
    }
    return dat;
}

void DHT11_ReadData() {
    DHT11_Start();
    if (DHT11_Check()) {
        I_RH = DHT11_ReadByte();
        D_RH = DHT11_ReadByte();
        I_Temp = DHT11_ReadByte();
        D_Temp = DHT11_ReadByte();
        DHT11_ReadByte(); // Checksum (bo qua)
    }
}
#endif