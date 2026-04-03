#include <reg52.h>
#include <stdio.h>
#include <intrins.h>

// --- KET NOI PHAN CUNG ---
sbit DHT11 = P1^5;      
sbit RS = P3^7;
sbit RW = P3^6;
sbit E  = P3^5;

// I2C Pin
sbit sda = P1^1;
sbit scl = P1^0;

// Buttons
sbit BTN_MENU = P3^0;
sbit BTN_UP   = P3^1;
sbit BTN_DOWN = P3^2;

// RELAY & BUZZER
sbit RELAY  = P0^6;
sbit BUZZER = P1^2; 

#define LCD_DATA P2

// --- BIEN TOAN CUC ---
unsigned char sec, min, hour, day, date, month, year;

// Bien bao thuc
unsigned char a_hour, a_min, a_date, a_month, a_on; 
unsigned char is_ringing = 0; 

// [MOI] Bien co bao dong nhiet do (0: chua bao, 1: da bao)
bit temp_warned = 0; 

int I_RH, D_RH, I_Temp, D_Temp, CheckSum; 

// Mode: 0-6 (Gio he thong), 7-11 (Bao thuc)
unsigned char mode = 0; 

// --- HAM DELAY ---
void Delay_ms(unsigned int t) {
    unsigned int x, y;
    for (x = 0; x < t; x++) for (y = 0; y < 112; y++);
}
void Delay_10us() {
    _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); 
    _nop_(); _nop_(); _nop_(); _nop_(); _nop_();
}

// --- HAM LCD ---
void LCD_Enable() { E = 1; Delay_10us(); E = 0; Delay_10us(); }
void LCD_Cmd(unsigned char cmd) { LCD_DATA = cmd; RS = 0; RW = 0; LCD_Enable(); Delay_ms(2); }
void LCD_Char(unsigned char dat) { LCD_DATA = dat; RS = 1; RW = 0; LCD_Enable(); Delay_ms(1); }
void LCD_Init() { LCD_Cmd(0x38); LCD_Cmd(0x0C); LCD_Cmd(0x01); LCD_Cmd(0x06); }
void LCD_String(char *str) { while (*str) LCD_Char(*str++); }
void LCD_Set_XY(unsigned char row, unsigned char col) {
    if (row == 0) LCD_Cmd(0x80 + col); else LCD_Cmd(0xC0 + col);
}

// --- GIAO TIEP I2C ---
void i2c_start() { sda = 1; scl = 1; _nop_(); _nop_(); sda = 0; scl = 0; }
void i2c_stop() { sda = 0; scl = 1; _nop_(); _nop_(); sda = 1; scl = 0; }
void sent8bit(unsigned char x) {
    unsigned char i;
    for(i=0; i<8; i++) { sda = (x & 0x80) ? 1 : 0; scl = 1; _nop_(); _nop_(); scl = 0; x <<= 1; }
    sda = 1; scl = 1; _nop_(); _nop_(); scl = 0;
}
unsigned char docdata(unsigned char ack) {
    unsigned char Dat = 0, i;
    sda = 1;
    for(i=0; i<8; i++) { scl = 1; Dat <<= 1; if(sda) Dat |= 1; scl = 0; }
    if(ack) sda = 0; else sda = 1;
    scl = 1; _nop_(); scl = 0;
    return Dat;
}

// --- HAM DOC/GHI RAM DS1307 ---
unsigned char DS1307_Read_Byte(unsigned char addr) {
    unsigned char dat;
    i2c_start(); sent8bit(0xD0); sent8bit(addr); i2c_stop();
    i2c_start(); sent8bit(0xD1); dat = docdata(0); i2c_stop();
    return dat;
}

void DS1307_Write_Byte(unsigned char addr, unsigned char dat) {
    i2c_start(); sent8bit(0xD0); sent8bit(addr); sent8bit(dat); i2c_stop();
}

void Load_Alarm() {
    a_hour  = DS1307_Read_Byte(0x08);
    a_min   = DS1307_Read_Byte(0x09);
    a_on    = DS1307_Read_Byte(0x0A);
    a_date  = DS1307_Read_Byte(0x0B);
    a_month = DS1307_Read_Byte(0x0C);
    
    if(a_hour > 23) a_hour = 0;
    if(a_min > 59) a_min = 0;
    if(a_on > 1) a_on = 0;
    if(a_date > 31) a_date = 0;   
    if(a_month > 12) a_month = 0; 
}

void Save_Alarm() {
    DS1307_Write_Byte(0x08, a_hour);
    DS1307_Write_Byte(0x09, a_min);
    DS1307_Write_Byte(0x0A, a_on);
    DS1307_Write_Byte(0x0B, a_date);
    DS1307_Write_Byte(0x0C, a_month);
}

// --- DS1307 TIME ---
unsigned char BCD2Dec(unsigned char bcd) { return ((bcd>>4)*10) + (bcd&0x0F); }
unsigned char Dec2BCD(unsigned char dec) { return ((dec/10)<<4) + (dec%10); }

void DS1307_Read_Time() {
    i2c_start(); sent8bit(0xD0); sent8bit(0x00); i2c_stop();
    i2c_start(); sent8bit(0xD1);
    sec   = BCD2Dec(docdata(1)); min   = BCD2Dec(docdata(1)); hour  = BCD2Dec(docdata(1));
    day   = BCD2Dec(docdata(1)); date  = BCD2Dec(docdata(1)); month = BCD2Dec(docdata(1));
    year  = BCD2Dec(docdata(0)); i2c_stop();
}

void DS1307_Write_Time() {
    i2c_start(); sent8bit(0xD0); sent8bit(0x00);
    sent8bit(Dec2BCD(sec)); sent8bit(Dec2BCD(min)); sent8bit(Dec2BCD(hour));
    sent8bit(Dec2BCD(day)); sent8bit(Dec2BCD(date)); sent8bit(Dec2BCD(month));
    sent8bit(Dec2BCD(year)); i2c_stop();
}

// --- DHT11 ---
void Request() { DHT11 = 1; Delay_10us(); DHT11 = 0; Delay_ms(20); DHT11 = 1; }
unsigned char Response() {
    unsigned int timeout = 0;
    while (DHT11 == 1) if (timeout++ > 500) return 0; timeout = 0;
    while (DHT11 == 0) if (timeout++ > 500) return 0; timeout = 0;
    while (DHT11 == 1) if (timeout++ > 500) return 0; return 1;
}
unsigned char Read_Byte() {
    unsigned char i, dat = 0;
    for (i = 0; i < 8; i++) {
        while (DHT11 == 0); Delay_10us(); Delay_10us(); Delay_10us();
        if (DHT11 == 1) { dat |= (1 << (7 - i)); while (DHT11 == 1); }
    }
    return dat;
}
void Read_DHT11() {
    Request();
    if (Response()) {
        I_RH = Read_Byte(); D_RH = Read_Byte(); I_Temp = Read_Byte(); D_Temp = Read_Byte(); CheckSum = Read_Byte();
    }
}

// --- HAM KIEM TRA NHIET DO (DA SUA DOI) ---
void checkTemp() {
    if (I_Temp <= 20) {
        // 1. Bat Relay (Gia su muc 1 la BAT)
        RELAY = 1; 

        // 2. Xu ly coi keu (Chi keu neu chua bao dong lan nao)
        if (temp_warned == 0) {
            unsigned char i;
            
            // Hien thi canh bao len LCD
            LCD_Set_XY(0, 0); LCD_String("!CANH BAO LANH!");
            LCD_Set_XY(1, 0); LCD_String(" BAM NUT DE TAT ");
            
            // Vong lap 50 lan * 100ms = 5000ms = 5 giay
            for (i = 0; i < 50; i++) {
                BUZZER = 0; Delay_ms(50); // Keu 50ms
                BUZZER = 1; Delay_ms(50); // Tat 50ms (Tao tieng tit tit)
                
                // Kiem tra nut nhan TRONG KHI dang keu
                if (BTN_MENU == 0 || BTN_UP == 0 || BTN_DOWN == 0) {
                    // Neu bam nut thi thoat ngay lap tuc
                    while(BTN_MENU==0 || BTN_UP==0 || BTN_DOWN==0); // Cho tha nut
                    break; 
                }
            }
            
            // Sau khi keu xong 5s hoac bam nut
            BUZZER = 1;      // Tat han coi
            temp_warned = 1; // Danh dau la da bao dong roi (de ko bao lai ngay)
            LCD_Init();      // Xoa thong bao tren LCD
        }
    } else {
        // Neu nhiet do > 20
        RELAY = 0;       // Tat Relay
        temp_warned = 0; // Reset co bao dong de lan sau lanh lai keu tiep
    }
}

// --- HIEN THI ---
void Display_Time_LCD() {
    char buffer[17];
    if (is_ringing) {
        LCD_Set_XY(0, 0); LCD_String("!!! BAO THUC !!!");
        LCD_Set_XY(1, 0); LCD_String("  BAM NUT TAT   ");
        return;
    }
    sprintf(buffer, "%02d:%02d:%02d T:%dC ", (int)hour, (int)min, (int)sec, I_Temp);
    LCD_Set_XY(0, 0); LCD_String(buffer);
    
    sprintf(buffer, "%02d/%02d/%02d H:%d%%%c", (int)date, (int)month, (int)year, I_RH, (a_on ? 'A' : ' '));
    LCD_Set_XY(1, 0); LCD_String(buffer);
}

void Display_Setting_Mode() {
    char buffer[17];
    LCD_Set_XY(0, 0); LCD_String("CHE DO CAI DAT: ");
    LCD_Set_XY(1, 0);
    switch(mode) {
        case 1: sprintf(buffer, "GIO: %02d         ", (int)hour); break;
        case 2: sprintf(buffer, "PHUT: %02d        ", (int)min); break;
        case 3: sprintf(buffer, "GIAY: %02d        ", (int)sec); break;
        case 4: sprintf(buffer, "NGAY: %02d        ", (int)date); break;
        case 5: sprintf(buffer, "THANG: %02d       ", (int)month); break;
        case 6: sprintf(buffer, "NAM: 20%02d       ", (int)year); break;
        case 7: sprintf(buffer, "BAO GIO: %02d      ", (int)a_hour); break;
        case 8: sprintf(buffer, "BAO PHUT: %02d     ", (int)a_min); break;
        case 9: sprintf(buffer, "BAO NGAY: %02d(0=All)", (int)a_date); break;
        case 10:sprintf(buffer, "BAO THANG:%02d     ", (int)a_month); break;
        case 11:sprintf(buffer, "BAO THUC: %s   ", (a_on ? "BAT" : "TAT")); break;
    }
    LCD_String(buffer);
}

// --- XU LY NUT NHAN ---
void Check_Buttons() {
    if (BTN_UP == 0 && BTN_DOWN == 0) {
        Delay_ms(1000); 
        if (BTN_UP == 0 && BTN_DOWN == 0) {
            if (mode == 0) { mode = 7; LCD_Init(); }
            while(BTN_UP == 0 || BTN_DOWN == 0); 
        }
    }
    
    if (BTN_MENU == 0) {
        Delay_ms(20);
        if (BTN_MENU == 0) {
            if (is_ringing) { is_ringing = 0; BUZZER = 1; while(BTN_MENU == 0); return; }
            mode++;
            if (mode > 11) { 
                if (mode > 6) Save_Alarm(); 
                else DS1307_Write_Time();
                mode = 0; LCD_Init(); 
            }
            while(BTN_MENU == 0); 
        }
    }

    if (BTN_UP == 0 && mode != 0) {
        if (BTN_UP == 0) {
            switch(mode) {
                case 1: hour++; if(hour>23) hour=0; break;
                case 2: min++;  if(min>59) min=0; break;
                case 3: sec = 0; break;
                case 4: date++; if(date>31) date=1; break;
                case 5: month++; if(month>12) month=1; break;
                case 6: year++; if(year>99) year=0; break;
                case 7: a_hour++; if(a_hour>23) a_hour=0; break;
                case 8: a_min++; if(a_min>59) a_min=0; break;
                case 9: a_date++; if(a_date>31) a_date=0; break; 
                case 10: a_month++; if(a_month>12) a_month=0; break; 
                case 11: a_on = !a_on; break; 
            }
            Delay_ms(200); 
        }
    }

    if (BTN_DOWN == 0 && mode != 0) {
        if (BTN_DOWN == 0) {
            switch(mode) {
                case 1: if(hour==0) hour=23; else hour--; break;
                case 2: if(min==0) min=59; else min--; break;
                case 3: sec = 0; break;
                case 4: if(date==1) date=31; else date--; break;
                case 5: if(month==1) month=12; else month--; break;
                case 6: if(year==0) year=99; else year--; break;
                case 7: if(a_hour==0) a_hour=23; else a_hour--; break;
                case 8: if(a_min==0) a_min=59; else a_min--; break;
                case 9: if(a_date==0) a_date=31; else a_date--; break;
                case 10: if(a_month==0) a_month=12; else a_month--; break;
                case 11: a_on = !a_on; break;
            }
            Delay_ms(200); 
        }
    }
}

void Check_Alarm_Logic() {
    if (mode == 0 && a_on == 1) {
        if (hour == a_hour && min == a_min && sec == 0) {
            unsigned char match_date = 0;
            if (a_date == 0 || (a_date == date && (a_month == 0 || a_month == month))) {
                match_date = 1;
            }
            if (match_date) { is_ringing = 1; }
        }
    }
    
    if (is_ringing) {
        BUZZER = 0; Delay_ms(100); 
        BUZZER = 1; Delay_ms(100);
        if (BTN_MENU==0 || BTN_UP==0 || BTN_DOWN==0) {
            is_ringing = 0; BUZZER = 1; LCD_Init();
            while(BTN_MENU==0 || BTN_UP==0 || BTN_DOWN==0); 
        }
    } else {
        if(mode == 0 && I_Temp > 20) BUZZER = 1; // Chi tat neu khong phai canh bao nhiet do
    }
}

// --- MAIN ---
void main() {
    unsigned int timer_dht = 0;
    
    BUZZER = 1; RELAY = 0;
    LCD_Init();
    LCD_String("Khoi dong...");
    Load_Alarm(); 
    Delay_ms(1000);
    LCD_Cmd(0x01); 

    while(1) {
        if (mode == 0) {
            DS1307_Read_Time(); 
            timer_dht++;
            if(timer_dht > 20) {
                Read_DHT11();
                timer_dht = 0;
            }
            
            checkTemp(); // KIEM TRA NHIET DO TRUOC
            
            // Chi hien thi gio neu khong dang trong trang thai canh bao 5s 
            // (Thuc te checkTemp la blocking 5s nen dong duoi se chay sau khi keu xong)
            Display_Time_LCD();
            
            Check_Alarm_Logic();
        } else {
            Display_Setting_Mode();
        }

        Check_Buttons(); 
        if(BTN_UP != 0 && BTN_DOWN != 0) Delay_ms(100); 
    }
}