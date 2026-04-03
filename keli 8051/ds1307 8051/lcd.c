
// gui ma lenh cho lcd
// voi RW=0 noi GND


#include "port.h"
#include "../lib/delay.h"

// --- HÀM T?O XUNG ENABLE (Chu?n hóa) ---
// LCD ch?t d? li?u khi chân E chuy?n t? Cao xu?ng Th?p
void delayus(int t)
{
    while(t--);
}
void LCD_Pulse_Enable()
{
    E = 1;
    delayus(10); // Gi? m?c cao m?t chút
    E = 0;       // Ch?t d? li?u
}

// --- G?I L?NH ÐI?U KHI?N ---
void lcdcontrol(unsigned char c)
{
    RS = 0;      // Ch? d? g?i l?nh
    lcd = c;     // Ð?t d? li?u lên Port
    LCD_Pulse_Enable();
    
    // L?nh xóa màn hình (0x01) c?n th?i gian dài hon (kho?ng 2ms)
    if(c == 0x01) delay(2); 
    else delayus(50); // Các l?nh khác ch? c?n delay ng?n (us)
}

// --- KH?I T?O LCD ---
void lcdinti()
{
    delay(20);         // Ch? ngu?n ?n d?nh
    lcdcontrol(0x30);
    delay(5);
    lcdcontrol(0x30);
    delayus(100);
    lcdcontrol(0x38);  // Giao ti?p 8 bit, 2 dòng, phông 5x7
    lcdcontrol(0x0C);  // B?t màn hình, t?t con tr?
    lcdcontrol(0x06);  // T? d?ng tang con tr?
    lcdcontrol(0x01);  // Xóa màn hình
    delay(2);
}

// --- IN 1 KÝ T? ---
void lcdprichar(char c)
{
    RS = 1;      // Ch? d? g?i d? li?u
    lcd = c;
    LCD_Pulse_Enable();
    delayus(50); // Delay ng?n d? vi?t nhanh hon
}

// --- IN CHU?I KÝ T? (Ð?i tên thành lcdputs cho chu?n) ---
void lcdputs(char* str)
{
    unsigned char i = 0;
    while(str[i] != '\0')
    {
        lcdprichar(str[i]);
        i++;
    }
}

// --- DI CHUY?N CON TR? VÀ IN KÝ T? ---
void outcharset(char c, char col, char row)
{
    unsigned char vitri = (row == 1 ? 0x80 : 0xC0);
    vitri += (col - 1);
    lcdcontrol(vitri);
    lcdprichar(c);
}

// --- DI CHUY?N CON TR? VÀ IN CHU?I ---
void outstringset(char * str, char col, char row)
{
    unsigned char vitri = (row == 1 ? 0x80 : 0xC0);
    vitri += (col - 1);
    lcdcontrol(vitri);
    lcdputs(str); // G?i hàm lcdputs
}

