/*******************************
 * i2cfunc.c v3
 * wrapper for I2C functions
 *******************************/

#include "i2cfunc.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#ifdef __linux__

#include <linux/i2c-dev.h>
#include <linux/i2c.h>

int i2c_open(unsigned char bus, unsigned char addr) {
  int file;
  char filename[16];
  snprintf(filename, sizeof(filename)/sizeof(char), "/dev/i2c-%d", bus);
  if ((file = open(filename, O_RDWR)) < 0) {
    fprintf(stderr, "i2c_open open error: %s\n", strerror(errno));
    return (file);
  }
  if (ioctl(file, I2C_SLAVE, addr) < 0) {
    fprintf(stderr, "i2c_open ioctl error: %s\n", strerror(errno));
    return (-1);
  }
  return (file);
}

int i2c_write(int handle, unsigned char *buf, unsigned int length) {
  if (write(handle, buf, length) != length) {
    fprintf(stderr, "i2c_write error: %s\n", strerror(errno));
    return (-1);
  }
  return (length);
}

int i2c_write_byte(int handle, unsigned char val) {
  if (write(handle, &val, 1) != 1) {
    fprintf(stderr, "i2c_write_byte error: %s\n", strerror(errno));
    return (-1);
  }
  return (1);
}

int i2c_read(int handle, unsigned char *buf, unsigned int length) {
  if (read(handle, buf, length) != length) {
    fprintf(stderr, "i2c_read error: %s\n", strerror(errno));
    return (-1);
  }
  return (length);
}

int i2c_read_byte(int handle, unsigned char *val) {
  if (read(handle, val, 1) != 1) {
    fprintf(stderr, "i2c_read_byte error: %s\n", strerror(errno));
    return (-1);
  }
  return (1);
}

int i2c_close(int handle) {
  if ((close(handle)) != 0) {
    fprintf(stderr, "i2c_close error: %s\n", strerror(errno));
    return (-1);
  }
  return (0);
}

int i2c_write_read(int handle, unsigned char addr, unsigned char *buf_w,
                   unsigned int len_w, unsigned char *buf_r,
                   unsigned int len_r) {
  struct i2c_rdwr_ioctl_data msgset;
  struct i2c_msg msgs[2];

  msgs[0].addr = addr;
  msgs[0].len = len_w;
  msgs[0].flags = 0;
  msgs[0].buf = buf_w;

  msgs[1].addr = addr;
  msgs[1].len = len_r;
  msgs[1].flags = 1;
  msgs[1].buf = buf_r;

  msgset.nmsgs = 2;
  msgset.msgs = msgs;

  if (ioctl(handle, I2C_RDWR, (unsigned long)&msgset) < 0) {
    fprintf(stderr, "i2c_write_read error: %s\n", strerror(errno));
    return -1;
  }
  return (len_r);
}

int write_reg(int handle, int addr, int reg, int val) {
  if (ioctl(handle,I2C_SLAVE,addr) < 0) {
    perror("Failed to change I2C Address: ");
    return;
  }
  printf("[0x%X] Writing 0x%X to register 0x%X\n", addr, val, reg);
  char pair[2];
  pair[0] = reg & 0xFF;
  pair[1] = val & 0xFF;
  return i2c_write(handle, (unsigned char *)pair, 2);
}

#else

int i2c_open(unsigned char bus, unsigned char addr) { return 0; }

int i2c_write(int handle, unsigned char *buf, unsigned int length) { return 0; }

int i2c_write_byte(int handle, unsigned char val) { return 0; }

int i2c_read(int handle, unsigned char *buf, unsigned int length) { return 0; }

int i2c_read_byte(int handle, unsigned char *val) { return 0; }

int i2c_close(int handle) { return 0; }

int i2c_write_read(int handle, unsigned char addr, unsigned char *buf_w,
                   unsigned int len_w, unsigned char *buf_r,
                   unsigned int len_r) { return 0; }

int read_reg(int handle, int addr, unsigned char reg, unsigned char *buf,
             size_t len) { return 0; };

int write_reg(int handle, int addr, int reg, int val) { return 0; };

#endif
