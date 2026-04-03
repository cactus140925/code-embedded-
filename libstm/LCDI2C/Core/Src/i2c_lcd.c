#include "i2c_lcd.h"
extern I2C_HandleTypeDef hi2c1;  // Khai báo bi?n i2c t? main.c

#define SLAVE_ADDRESS_LCD (0x27 << 1) // Ð?a ch? I2C c?a PCF8574

void lcd_send_cmd (char cmd) {
    char data_u, data_l;
    uint8_t data_t[4];
    data_u = (cmd & 0xf0);
    data_l = ((cmd << 4) & 0xf0);
    data_t[0] = data_u|0x0C;  // en=1, rs=0
    data_t[1] = data_u|0x08;  // en=0, rs=0
    data_t[2] = data_l|0x0C;  // en=1, rs=0
    data_t[3] = data_l|0x08;  // en=0, rs=0
    HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data) {
    char data_u, data_l;
    uint8_t data_t[4];
    data_u = (data & 0xf0);
    data_l = ((data << 4) & 0xf0);
    data_t[0] = data_u|0x0D;  // en=1, rs=1
    data_t[1] = data_u|0x09;  // en=0, rs=1
    data_t[2] = data_l|0x0D;  // en=1, rs=1
    data_t[3] = data_l|0x09;  // en=0, rs=1
    HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t *) data_t, 4, 100);
}

void lcd_clear (void) {
    lcd_send_cmd (0x01);
    HAL_Delay(2);
}

void lcd_put_cur(int row, int col) {
    switch (row) {
        case 0: col |= 0x80; break;
        case 1: col |= 0xC0; break;
    }
    lcd_send_cmd (col);
}

void lcd_init (void) {
    HAL_Delay(50);
    lcd_send_cmd (0x30);
    HAL_Delay(5);
    lcd_send_cmd (0x30);
    HAL_Delay(1);
    lcd_send_cmd (0x30);
    HAL_Delay(10);
    lcd_send_cmd (0x20);  // Ch? d? 4-bit
    HAL_Delay(10);
    lcd_send_cmd (0x28);  // 2 hàng, font 5x8
    HAL_Delay(1);
    lcd_send_cmd (0x08);  // T?t hi?n th?
    HAL_Delay(1);
    lcd_send_cmd (0x01);  // Xóa màn hình
    HAL_Delay(1);
    lcd_send_cmd (0x06);  // T? d?ng tang con tr?
    HAL_Delay(1);
    lcd_send_cmd (0x0C);  // B?t hi?n th?, t?t con tr?
}

void lcd_send_string (char *str) {
    while (*str) lcd_send_data (*str++);
}