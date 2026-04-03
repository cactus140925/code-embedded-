#include "I2C_LCD.h"

void lcd_send_cmd(I2C_LCD_HandleTypeDef *lcd, char cmd) {
    uint8_t data_u, data_l;
    uint8_t data_t[4];
    data_u = (cmd & 0xf0);
    data_l = ((cmd << 4) & 0xf0);
    data_t[0] = data_u | 0x0C;  // en=1, rs=0
    data_t[1] = data_u | 0x08;  // en=0, rs=0
    data_t[2] = data_l | 0x0C;  // en=1, rs=0
    data_t[3] = data_l | 0x08;  // en=0, rs=0
    
    // Ép Timeout xu?ng 10ms d? không treo motor
    HAL_I2C_Master_Transmit(lcd->hi2c, lcd->address, data_t, 4, 10);
    HAL_Delay(2); // Delay này c?c k? quan tr?ng cho Proteus
}

void lcd_send_data(I2C_LCD_HandleTypeDef *lcd, char data) {
    uint8_t data_u, data_l;
    uint8_t data_t[4];
    data_u = (data & 0xf0);
    data_l = ((data << 4) & 0xf0);
    data_t[0] = data_u | 0x0D;  // en=1, rs=1
    data_t[1] = data_u | 0x09;  // en=0, rs=1
    data_t[2] = data_l | 0x0D;  // en=1, rs=1
    data_t[3] = data_l | 0x09;  // en=0, rs=1
    
    HAL_I2C_Master_Transmit(lcd->hi2c, lcd->address, data_t, 4, 10);
    HAL_Delay(2);
}

void lcd_init(I2C_LCD_HandleTypeDef *lcd) {
    HAL_Delay(100); // Ch? LCD ?n d?nh di?n áp ?o
    lcd_send_cmd(lcd, 0x33); HAL_Delay(5);
    lcd_send_cmd(lcd, 0x32); HAL_Delay(5);
    lcd_send_cmd(lcd, 0x28); HAL_Delay(5); // 2 dòng, font 5x8
    lcd_send_cmd(lcd, 0x0C); HAL_Delay(5); // B?t hi?n th?
    lcd_send_cmd(lcd, 0x01); HAL_Delay(5); // Xóa màn hình
}

void lcd_puts(I2C_LCD_HandleTypeDef *lcd, char *str) {
    while (*str) lcd_send_data(lcd, *str++);
}

void lcd_gotoxy(I2C_LCD_HandleTypeDef *lcd, int col, int row) {
    uint8_t addr = (row == 0) ? (0x80 + col) : (0xC0 + col);
    lcd_send_cmd(lcd, addr);
}