#ifndef lcd_h
#define lcd_h
#include "stm32f1xx_hal.h"
#define LCD_PORT GPIOA
#define RS GPIO_PIN_1
#define E GPIO_PIN_2
#define D4 GPIO_PIN_3
#define D5 GPIO_PIN_4
#define D6 GPIO_PIN_5
#define D7 GPIO_PIN_6
void LCD_init(void);
void LCD_send(uint8_t cmd);
void LCD_Data(uint8_t data);
void LCD_String(char * str);
void LCD_Cursor(uint8_t row, uint8_t col);
void LCD_clear(void);
	
#endif
 
 