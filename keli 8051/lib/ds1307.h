#ifndef _DS1307_H_
#define _DS1307_H_
#include <reg52.h>
#include <intrins.h>

sbit sda = P1^1;
sbit scl = P1^0;

// Bien thoi gian toan cuc de Main truy cap
unsigned char sec, min, hour, day, date, month, year;

// --- Giao tiep I2C Co ban ---
void i2c_start() {
    sda = 1; scl = 1; _nop_(); _nop_();
    sda = 0; scl = 0;
}
void i2c_stop() {
    sda = 0; scl = 1; _nop_(); _nop_();
    sda = 1; scl = 0;
}
void sent8bit(unsigned char x) {
    unsigned char i;
    for(i=0; i<8; i++) {
        sda = (x & 0x80) ? 1 : 0; scl = 1; _nop_(); _nop_(); scl = 0; x <<= 1;
    }
    sda = 1; scl = 1; _nop_(); _nop_(); scl = 0; // ACK
}
unsigned char docdata(unsigned char ack) {
    unsigned char Dat = 0, i;
    sda = 1;
    for(i=0; i<8; i++) {
        scl = 1; Dat <<= 1; if(sda) Dat |= 1; scl = 0;
    }
    if(ack) sda = 0; else sda = 1; // ACK/NACK
    scl = 1; _nop_(); scl = 0;
    return Dat;
}

// --- Xu ly DS1307 ---
unsigned char BCD2Dec(unsigned char bcd) { return ((bcd>>4)*10) + (bcd&0x0F); }
unsigned char Dec2BCD(unsigned char dec) { return ((dec/10)<<4) + (dec%10); }

void DS1307_Read() {
    i2c_start(); sent8bit(0xD0); sent8bit(0x00); i2c_stop();
    i2c_start(); sent8bit(0xD1);
    sec   = BCD2Dec(docdata(1));
    min   = BCD2Dec(docdata(1));
    hour  = BCD2Dec(docdata(1));
    day   = BCD2Dec(docdata(1));
    date  = BCD2Dec(docdata(1));
    month = BCD2Dec(docdata(1));
    year  = BCD2Dec(docdata(0));
    i2c_stop();
}

void DS1307_Write() {
    i2c_start(); sent8bit(0xD0); sent8bit(0x00);
    sent8bit(Dec2BCD(sec)); sent8bit(Dec2BCD(min)); sent8bit(Dec2BCD(hour));
    sent8bit(Dec2BCD(day)); sent8bit(Dec2BCD(date)); sent8bit(Dec2BCD(month));
    sent8bit(Dec2BCD(year));
    i2c_stop();
}
#endif