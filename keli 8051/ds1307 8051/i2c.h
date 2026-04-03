#ifndef I2C_H
#define I2C_H
void i2cinti();
void i2c_star();
void i2c_end();
bit i2c_get_ack();
bit i2cwrite(unsigned char a);
void i2cack();
void i2cNack();
unsigned char i2cread(bit ack);
#endif
