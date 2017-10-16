//
//  ssr.c
//  SSR-Board-Interface
//
//  Created by Eddie Hurtig on 10/16/17.
//  Copyright © 2017 Eddie Hurtig. All rights reserved.
//

#include "ssr.h"


int init_ssr_board(int addr) {
  int handle = i2c_open(I2CBUS, addr);
  
  if (handle < 0) {
    return -1;
  }
  
  int rc = 0;
  
  // Set Modes
  (i2c_write_read(handle, addr, MODE2, 1, 0x04) != 0) && (rc = -1);
  (i2c_write_read(handle, addr, MODE1, 1, 0x01) != 0) && (rc = -1);
  
  int oldmode = 0;
  // Read in the operating mode of the chip
  (i2c_write_read(handle, addr, MODE1, (unsigned char*)&oldmode, 1) != 0) && (rc = -1);
  
  // Put the Chip into sleep mode
  (i2c_write_read(handle, addr, MODE1, (oldmode & 0x7F) | 0x10) != 0) && (rc = -1);
  (i2c_write_read(handle, addr, PRESCALE, 0x04) != 0) && (rc = -1);
  (i2c_write_read(handle, addr, MODE1, oldmode | 0x80) != 0) && (rc = -1);
  
  // Wait for Chip to wake
  usleep(500);
  
  // Close Handle
  (i2c_close(handle) != 0) && (rc = -1);
  return rc;
}

void set_channel(int address, int channel, int value) {
  int handle;
  handle = i2c_open(I2CBUS, address);
  channel = channel % 16;
  
  i2c_write_read(handle, address, 0x06 + 4 * channel, 0);
  i2c_write_read(handle, address, 0x07 + 4 * channel, 0);
  i2c_write_read(handle, address, 0x08 + 4 * channel, 0xFF & value);
  i2c_write_read(handle, address, 0x09 + 4 * channel, (value >> 8) & 0xFF);
  
  i2c_close(handle);
}
