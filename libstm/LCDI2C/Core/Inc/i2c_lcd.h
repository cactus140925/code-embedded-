#include "stm32f1xx_hal.h" // Thay d?i tùy dòng chip (f1, f4, l4...)

void lcd_init (void);   // Kh?i t?o LCD
void lcd_send_cmd (char cmd);  // G?i l?nh
void lcd_send_data (char data);  // G?i d? li?u
void lcd_send_string (char *str);  // G?i chu?i
void lcd_put_cur(int row, int col);  // Ð?nh v? con tr?
void lcd_clear (void); // Xóa màn hình