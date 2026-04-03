#include "ds1307.h"
#include "i2c.h" 


unsigned char Ds1307_Read(unsigned char addr)
{
    unsigned char dat;
    
    
    i2c_star();          
    i2cwrite(0xD0);      
    i2cwrite(addr);     
    
   
    i2c_star();          
    i2cwrite(0xD1);      
    
   
    dat = i2cread(0);    
    
    i2c_end();           
    return dat;
}

void ds1307_Write(unsigned char addr, unsigned char dat)
{
    i2c_star();          
    i2cwrite(0xD0);      
    i2cwrite(addr);      
    i2cwrite(dat);       
    i2c_end();           
}

void ds1307_inti()
{
    unsigned char tmp;
    
    tmp = Ds1307_Read(0x00); 
    
    
    if (tmp & 0x80) 
    {
        tmp &= 0x7F;         
        ds1307_Write(0x00, tmp); 
    }
}

void ds1307_write_time(unsigned char sec, unsigned char min, unsigned char hour, unsigned char mode, unsigned char apm)
{
    
    sec = ((sec / 10) << 4) | (sec % 10);
    min = ((min / 10) << 4) | (min % 10);
    hour = ((hour / 10) << 4) | (hour % 10);

    if (mode == 12)
    {
        hour |= 0x40; 
        if (apm)      
        {
            hour |= 0x20; 
        }
    }
    
    i2c_star();     
    i2cwrite(0xD0); 
    i2cwrite(0x00);  
    i2cwrite(sec);   
    i2cwrite(min);   
    i2cwrite(hour);  
    i2c_end();       
}

bit ds1307_read_time(unsigned char *sec, unsigned char *min, unsigned char *hour, unsigned char *mode)
{
    bit amp = 0; // Kh?i t?o m?c d?nh là AM (0) d? tránh l?i
    unsigned char s_tmp, m_tmp, h_tmp;

    // Ğ?t con tr? v? 0x00
    i2c_star();      
    i2cwrite(0xD0);  
    i2cwrite(0x00);  
    
    i2c_end();       

    
    i2c_star();      
    i2cwrite(0xD1);  
    s_tmp = i2cread(1); // ACK (Ğ?c ti?p)
    m_tmp = i2cread(1); // ACK
    h_tmp = i2cread(0); // NACK (D?ng)
    i2c_end();

    s_tmp &= 0x7F;   // Xóa bit CH
    // m_tmp không c?n mask vì bit 7 luôn là 0

    // Chuy?n BCD sang DEC
    *sec = ((s_tmp >> 4) * 10) + (s_tmp & 0x0F);
    *min = ((m_tmp >> 4) * 10) + (m_tmp & 0x0F);

    if (h_tmp & 0x40) // Mode 12h
    {
        *mode = 12;
        if (h_tmp & 0x20) amp = 1; // PM
        else amp = 0;              // AM
        h_tmp &= 0x1F; // L?y 5 bit giá tr? gi?
    }
    else // Mode 24h
    {
        *mode = 24;
        h_tmp &= 0x3F; // L?y 6 bit giá tr? gi?
    }
    *hour = ((h_tmp >> 4) * 10) + (h_tmp & 0x0F);

    return amp; 
}

void Ds1307_write_date(unsigned char day, unsigned char month, unsigned char year, unsigned char date)
{
   
    date = ((date / 10) << 4) | (date % 10);
    month = ((month / 10) << 4) | (month % 10);
    year = ((year / 10) << 4) | (year % 10);
    day &= 0x07; 

    i2c_star();    
    i2cwrite(0xD0); 
    i2cwrite(0x03); // B?t d?u t? thanh ghi Th? (Day of week)
    i2cwrite(day);  
    i2cwrite(date); // Ngày (Date)
    i2cwrite(month); 
    i2cwrite(year);  
    i2c_end();    
}

void ds1307_read_date(unsigned char *day, unsigned char *date, unsigned char *month, unsigned char *year)
{
    // Ğ?t con tr? v? 0x03
    i2c_star();      
    i2cwrite(0xD0);  
    i2cwrite(0x03);  
    i2c_end();       

    // Ğ?c liên ti?p 4 byte
    i2c_star();      
    i2cwrite(0xD1);  
    *day = i2cread(1);   
    *date = i2cread(1);   
    *month = i2cread(1);  
    *year = i2cread(0);   
    i2c_end();       

    // Chuy?n BCD sang DEC
    *day &= 0x07;    
    
    *date &= 0x3F;   
    *date = ((*date >> 4) * 10) + (*date & 0x0F); 
    
    *month &= 0x1F;  
    *month = ((*month >> 4) * 10) + (*month & 0x0F); 
    
    *year = ((*year >> 4) * 10) + (*year & 0x0F); 
}