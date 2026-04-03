#ifndef _LCD_H_
#define _LCD_H_
#define lcdputs lcdpristring
// Khai báo các hàm (Prototypes)
void lcdcontrol(unsigned char c);
void lcdinti();
void lcdprichar(char c);
void lcdpristring(char* str);
void outcharset(char c, char col, char row);
void outstringset(char * str, char col, char row);
void delayus(int t);

#endif