#include"lcd.h"
void Enable(void)
	{
		HAL_GPIO_WritePin(LCD_PORT,E,GPIO_PIN_SET);
		HAL_Delay(1);
		HAL_GPIO_WritePin(LCD_PORT,E,GPIO_PIN_RESET);
			HAL_Delay(1);
		}
void Send4bit(uint8_t nibble)
	{
		HAL_GPIO_WritePin(LCD_PORT, D4, (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_PORT, D5, ((nibble >> 1) & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_PORT, D6, ((nibble >> 2) & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_PORT, D7, ((nibble >> 3) & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
void LCD_sendcomman(uint8_t cmd)
	{
		HAL_GPIO_WritePin(LCD_PORT,RS,GPIO_PIN_RESET);
		Send4bit(cmd>>4);
		Enable();
		Send4bit(cmd&0x0F);
		Enable();
		}
void LCD_sendData(uint8_t data)
	{
		HAL_GPIO_WritePin(LCD_PORT,RS,GPIO_PIN_SET);
		Send4bit(data>>4);
		Enable();
		Send4bit(data&0x0F);
		Enable();
		
		}
void LCD_init()
	{
		HAL_Delay(50);
		HAL_GPIO_WritePin(LCD_PORT,RS,GPIO_PIN_RESET);
		HAL_Delay(5);
		Send4bit(0x03);
		Enable();
		HAL_Delay(1);
		Send4bit(0x03);
		Enable();
		HAL_Delay(1);
		Send4bit(0x03);
		Enable();
		HAL_Delay(5);
		Send4bit(0x02);
		Enable();
		HAL_Delay(1);
		LCD_sendcomman(0x28);
		LCD_sendcomman(0X08);
		LCD_sendcomman(0x01);//xoa man;
		HAL_Delay(1);
		LCD_sendcomman(0x06);
		LCD_sendcomman(0x0C);
	}
	void LCD_String(char* str)
	{
		while(*str)
		{
			LCD_sendData(*str++);
		}
	}
	void LCD_Cursor(uint8_t row, uint8_t col)
	{
		uint8_t addr;
		if(row==0)
		{
			addr = 0x80+ col;
		}else
		{
			addr= 0xC0+col;
		}
		LCD_sendcomman(addr);
	}
	void LCD_Clear()
	{
		LCD_sendcomman(0x01);
		HAL_Delay(2);
	}