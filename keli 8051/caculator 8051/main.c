#include<regx52.h>
#include"../lib/delay.h"
#define keyport P1
unsigned char keypad[4][4] = {{'7','8','9','/'},
    {'4','5','6','x'},
    {'1','2','3','-'},
    {' ','0','=','+'} };
unsigned char colloc, rowloc;
		do
{
    keyport = 0xF0;    
    colloc = keyport;
    colloc &= 0xF0; 
} while (colloc != 0xF0);  

