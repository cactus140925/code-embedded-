#ifndef _DS1307_H_
#define _DS1307_H_

// Khai báo nguyên m?u hàm
void ds1307_inti();
void ds1307_write_time(unsigned char sec, unsigned char min, unsigned char hour, unsigned char mode, unsigned char apm);
bit ds1307_read_time(unsigned char *sec, unsigned char *min, unsigned char *hour, unsigned char *mode);
void Ds1307_write_date(unsigned char day, unsigned char month, unsigned char year, unsigned char date);
void ds1307_read_date(unsigned char *day, unsigned char *date, unsigned char *month, unsigned char *year);

#endif