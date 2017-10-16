#include <stdint.h>

#ifndef I2CFUNC_H
#define I2CFUNC_H

int i2c_open(unsigned char bus, unsigned char addr);

int i2c_write(int handle, unsigned char *buf, unsigned int length);

int i2c_write_byte(int handle, unsigned char val);

int i2c_read(int handle, unsigned char *buf, unsigned int length);

int i2c_read_byte(int handle, unsigned char *val);

int i2c_close(int handle);

int i2c_write_read(int handle, unsigned char addr, unsigned char *buf_w,
                   unsigned int len_w, unsigned char *buf_r,
                   unsigned int len_r);

int read_reg(int handle, int addr, unsigned char reg, unsigned char *buf,
              size_t len);

int write_reg(int handle, int addr, int reg, int val);
#endif

