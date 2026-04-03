#include <regx52.h>
#include "../lib/delay.h"
#include "port.h"
#include "i2c.h"      
#include "ds1307.h"   
#include "lcd.h"     // Ð?m b?o tên file thu vi?n dúng là lcd1.h

// Ki?m tra l?i chân BUZZER
sbit BUZZER = P2^0; 

unsigned char sec, min, hour, mode_12h, date, day, month, year;
unsigned char alarm_hour = 6, alarm_min = 30; // M?c d?nh 6:30
bit is_alarm_on = 1; 
unsigned char mode = 0; 

void lcdprichar1(unsigned char dat)
{
    if (dat > 99) dat = 99; 
    lcdprichar((dat / 10) + '0');
    lcdprichar((dat % 10) + '0'); 
}

// --- KI?M TRA T? H?P PHÍM (UP + DOWN) ---
void check_combo()
{
    if(UP == 0 && DOWN == 0)
    {
        delay(20); // Ch?ng rung
        if(UP == 0 && DOWN == 0)
        {
            if(mode < 3) 
            {
                mode = 3; 
                lcdcontrol(0x01); // Xóa màn hình
                delay(2);
            }
            while(UP == 0 || DOWN == 0); // Ch? nh?
        }
    }
}

// --- NÚT SET ---
void checkbutton_set()
{
    if(SET == 0)
    {
        delay(20); // Ch?ng rung
        if(SET == 0)
        {
            // Logic chuy?n mode
            if(mode == 0 || mode == 1 || mode == 2) 
            {
                mode++;
                if(mode > 2) mode = 0;
            }
            else if (mode == 3 || mode == 4)
            {
                mode++;
                if(mode > 4) mode = 0;
            }

            lcdcontrol(0x01); // Xóa màn hình ngay khi chuy?n
            delay(2);
            while(SET == 0); // Ch? nh? nút m?i ch?y ti?p
        }
    }
}

// --- NÚT UP ---
void checkbutton_up()
{
    if(DOWN == 0) return; 
    if(UP == 0)
    {
        delay(20);
        if(UP == 0 && DOWN == 1)
        {
            switch(mode)
            {
                case 1: alarm_hour++; if(alarm_hour > 23) alarm_hour = 0; break;
                case 2: alarm_min++; if(alarm_min > 59) alarm_min = 0; break;
                case 3: 
                    hour++; if(hour > 23) hour = 0; 
                    ds1307_write_time(sec, min, hour, mode_12h, 0); 
                    break;
                case 4:
                    min++; if(min > 59) min = 0;
                    ds1307_write_time(0, min, hour, mode_12h, 0); 
                    break;
            }
            
            // Delay nh? d? nút tang t? t? n?u nh?n gi?, ho?c dùng while d? b?t nh?n t?ng cái
            delay(150); 
        }
    }
}

// --- NÚT DOWN ---
void checkbutton_down()
{
    if(UP == 0) return; 
    if(DOWN == 0)
    {
        delay(20);
        if(DOWN == 0 && UP == 1)
        {
            switch(mode)
            {
                case 1: if(alarm_hour == 0) alarm_hour = 23; else alarm_hour--; break;
                case 2: if(alarm_min == 0) alarm_min = 59; else alarm_min--; break;
                case 3: 
                    if(hour == 0) hour = 23; else hour--; 
                    ds1307_write_time(sec, min, hour, mode_12h, 0); 
                    break;
                case 4: 
                    if(min == 0) min = 59; else min--;
                    ds1307_write_time(0, min, hour, mode_12h, 0); 
                    break;
            }
            delay(150);
        }
    }
}

// --- BÁO TH?C ---
void check_alarm()
{
    if(is_alarm_on && hour == alarm_hour && min == alarm_min && sec < 2)
    {
        BUZZER = 0; 
    }
    else
    {
        BUZZER = 1; 
    }
}

void main()
{
    bit amp_dummy; 
    i2cinti();      
    ds1307_inti();  
    lcdinti();      
    BUZZER = 1;     

    while (1)
    {
        // 1. CH? Ð?C DS1307 KHI C?N THI?T
        // N?u dang ch?nh báo th?c (Mode 1,2) thì không c?n d?c DS1307 liên t?c
        // giúp vòng l?p nhanh hon, nút b?m nh?y hon.
        if(mode == 0 || mode == 3 || mode == 4) 
        {
            amp_dummy = ds1307_read_time(&sec, &min, &hour, &mode_12h);
            ds1307_read_date(&day, &date, &month, &year);
        }

        // 2. QUÉT NÚT NH?N (Uu tiên quét tru?c khi hi?n th?)
        check_combo();       
        checkbutton_set();   
        checkbutton_up();    
        checkbutton_down();  

        // 3. HI?N TH? LCD
        if (mode == 0) // Màn hình chính
        {
            lcdcontrol(0x80); 
            lcdprichar1(hour); lcdprichar(':');
            lcdprichar1(min); lcdprichar(':');
            lcdprichar1(sec);
            
            lcdcontrol(0xC0); 
            // Dùng lcdputs (ho?c lcdpristring tùy tên trong lcd1.h)
            switch (day) {
                case 1: lcdputs("Sun "); break; 
                case 2: lcdputs("Mon "); break;
                case 3: lcdputs("Tue "); break;
                case 4: lcdputs("Wed "); break;
                case 5: lcdputs("Thu "); break;
                case 6: lcdputs("Fri "); break;
                case 7: lcdputs("Sat "); break;
                default: lcdputs("Err "); break;
            }
            lcdprichar1(date); lcdprichar('/');
            lcdprichar1(month); lcdprichar('/');
            lcdprichar1(year);

            check_alarm(); 
        }
        else if (mode == 1 || mode == 2) // Ch?nh báo th?c
        {
            lcdcontrol(0x80);
            lcdputs("SET ALARM:      ");
            
            lcdcontrol(0xC0);
            lcdprichar1(alarm_hour);
            lcdprichar(':');
            lcdprichar1(alarm_min);
            
            if(mode == 1) lcdputs(" <Hour  ");
            if(mode == 2) lcdputs(" <Min   ");
            
            BUZZER = 1; 
        }
        else if (mode == 3 || mode == 4) // Ch?nh gi? th?c
        {
            lcdcontrol(0x80);
            lcdputs("SET REAL TIME:  ");
            
            lcdcontrol(0xC0);
            lcdprichar1(hour);
            lcdprichar(':');
            lcdprichar1(min);
            
            if(mode == 3) lcdputs(" <Hour  ");
            if(mode == 4) lcdputs(" <Min   ");
            
            BUZZER = 1;
        }

        // 4. DELAY C?C NG?N
        // Ch? delay 10ms ho?c 20ms d? h? th?ng nh?y
        delay(10); 
    }
}