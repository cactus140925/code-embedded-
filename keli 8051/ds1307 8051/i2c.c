

#include <port.h> 
#include <intrins.h>

#define i2cdelay() {_nop_(); _nop_(); _nop_(); _nop_(); _nop_();} // Delay nh? b?ng 5 nop

void i2cinti()
{
    SDA = 1; 
    SCL = 1;
    i2cdelay();
}

void i2c_star()
{
    SCL = 0;
    i2cdelay();
    SDA = 1; 
    i2cdelay();
    SCL = 1; 
    i2cdelay();
    SDA = 0; 
    i2cdelay();
    SCL = 0; 
    i2cdelay();
}

void i2c_end()
{
    SCL = 0; 
    i2cdelay();
    SDA = 0; 
    i2cdelay();
    SCL = 1; 
    i2cdelay();
    SDA = 1; 
    i2cdelay();
}

bit i2c_get_ack()
{
    bit check;
    SDA = 1; 
    i2cdelay();
    SCL = 1; 
    i2cdelay();
    check = SDA; 
    SCL = 0; 
    i2cdelay();
    return check; 
}

bit i2cwrite(unsigned char a)
{
    unsigned char i;
    for (i = 0; i < 8; i++)
    {
        SDA = (bit)(a & 0x80);
        i2cdelay();
        SCL = 1; // 
        i2cdelay();
        SCL = 0; //
        i2cdelay();
        a <<= 1; 
    }
    return i2c_get_ack(); 
}

void i2cack()
{
    SDA = 0; 
    i2cdelay();
    SCL = 1; 
    i2cdelay();
    SCL = 0; 
    i2cdelay();
}

void i2cNack()
{
    SDA = 1; 
    i2cdelay();
    SCL = 1; 
    i2cdelay();
    SCL = 0; 
    i2cdelay();
}
unsigned char i2cread(bit ack)
{
    unsigned char dat = 0x00;
    unsigned char i;

    SDA = 1;
    for (i = 0; i < 8; i++)
    {
        i2cdelay();
        SCL = 1; 
        i2cdelay();
        dat <<= 1; 
        if (SDA) dat |= 0x01; 
        SCL = 0; 
        i2cdelay();
    }
    if (ack)
    {
        i2cack(); 
    }
    else
    {
        i2cNack(); 
    }
    return dat; 
}