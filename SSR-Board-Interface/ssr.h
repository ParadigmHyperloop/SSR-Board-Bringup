//
//  ssr.h
//  SSR-Board-Interface
//
//  Created by Eddie Hurtig on 10/16/17.
//  Copyright © 2017 Eddie Hurtig. All rights reserved.
//

#ifndef ssr_h
#define ssr_h
#include "i2cfunc.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MODE1              0x00
#define MODE2              0x01
#define SUBADR1            0x02
#define SUBADR2            0x03
#define SUBADR3            0x04
#define PRESCALE           0xFE
#define LED0_ON_L          0x06
#define LED0_ON_H          0x07
#define LED0_OFF_L         0x08
#define LED0_OFF_H         0x09
#define ALL_LED_ON_L       0xFA
#define ALL_LED_ON_H       0xFB
#define ALL_LED_OFF_L      0xFC
#define ALL_LED_OFF_H      0xFD

#define RESTART            0x80
#define SLEEP              0x10
#define ALLCALL            0x01
#define INVRT              0x10
#define OUTDRV             0x04

#ifndef I2CBUS
#define I2CBUS 1
#endif

#ifndef BASEADDRESS
#define BASEADDRESS 0x40
#endif

#ifndef I2C_SLAVE
#define I2C_SLAVE 1000UL
#endif

int init_ssr_board(int addr);
void set_channel(int address, int channel, int value);
  
#endif /* ssr_h */
