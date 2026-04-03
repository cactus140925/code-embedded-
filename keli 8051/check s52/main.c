#include <reg52.h>

// --- Khai báo k?t n?i ph?n c?ng ---
#define LCD_DATA P2     // Port d? li?u D0-D7 n?i vào Port 2

sbit LCD_RS = P3^7;     // Chân RS n?i P3.7
sbit LCD_RW = P3^6;     // Chân RW n?i P3.6
sbit LCD_EN = P3^5;     // Chân E n?i P3.5

// --- Hàm t?o d? tr? ---
void delay_ms(unsigned int t) {
    unsigned int x, y;
    for (x = 0; x < t; x++) {
        for (y = 0; y < 123; y++); // Ch?nh s?a s? này tùy th?ch anh
    }
}

// --- G?i l?nh di?u khi?n xu?ng LCD ---
void LCD_Cmd(unsigned char cmd) {
    LCD_DATA = cmd;     // Ð?t l?nh lên du?ng truy?n d? li?u
    LCD_RS = 0;         // RS = 0 d? ch?n thanh ghi l?nh
    LCD_RW = 0;         // RW = 0 d? ghi
    
    // T?o xung Enable d? ch?t d? li?u
    LCD_EN = 1;
    delay_ms(1);        // Ð?i m?t chút
    LCD_EN = 0;
    delay_ms(2);        // Ð?i LCD x? lý l?nh
}

// --- G?i ký t? hi?n th? lên LCD ---
void LCD_Char(unsigned char dat) {
    LCD_DATA = dat;     // Ð?t ký t? lên du?ng truy?n
    LCD_RS = 1;         // RS = 1 d? ch?n thanh ghi d? li?u
    LCD_RW = 0;         // RW = 0 d? ghi
    
    // T?o xung Enable
    LCD_EN = 1;
    delay_ms(1);
    LCD_EN = 0;
    delay_ms(1);
}

// --- G?i m?t chu?i ký t? ---
void LCD_String(char *str) {
    while (*str) {
        LCD_Char(*str++);
    }
}

// --- Kh?i t?o LCD (Ch? d? 8-bit) ---
void LCD_Init() {
    delay_ms(20);       // Ð?i LCD kh?i d?ng xong di?n áp
    LCD_Cmd(0x38);      // Ch? d? 8-bit, 2 dòng, font 5x7
    LCD_Cmd(0x0C);      // B?t hi?n th?, t?t con tr? (Cursor off)
    LCD_Cmd(0x06);      // T? d?ng tang con tr?
    LCD_Cmd(0x01);      // Xóa màn hình
    delay_ms(5);        // L?nh xóa màn hình c?n delay lâu hon
}

// --- Chuong trình chính ---
void main() {
    LCD_Init();         // Kh?i t?o màn hình
    
    // Di chuy?n con tr? d?n d?u dòng 1 (v? trí 0x80)
    LCD_Cmd(0x80);      
    
    // In ch? "check"
    LCD_String("check");

    while(1) {
        // Vòng l?p vô t?n
    }
}