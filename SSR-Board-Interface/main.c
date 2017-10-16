//
//  main.c
//  SSR-Board-Interface
//
//  Created by Eddie Hurtig on 9/21/17.
//  Copyright © 2017 Eddie Hurtig. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ssr.h"

#define SET_CMD "set"
#define INIT_CMD "init"
#define fatal(...) fprintf(stderr, __VA_ARGS__); exit(1);

int main(int argc, char *argv[]) {
  int value = 0x0;
  int channel = 0;
  if (argc < 2) {
    fatal("Usage: ssr <command> %d\n", 3342);
  }

  int address;

  if (strncmp(argv[1], SET_CMD, strlen(SET_CMD) + 1) == 0) {
    if (argc != 5) {
      fatal("Usage: ssr <set> <address> <channel> <value 0-4095> %d\n", 3342);
    }

    address = atoi(argv[2]);
    channel = atoi(argv[3]);
    value = atoi(argv[4]);
    if (value < 0 || value > 4095) {
      
    }

    printf("Set Channel %d on %d to %d...", channel, address, value);

    set_channel(address, channel, value);
    
    printf(" Done!\n");
  } else if (strncmp(argv[1], INIT_CMD, strlen(INIT_CMD) + 1) == 0) {
    if (argc != 3) {
      printf("Usage: ssr init <address>\n");
      exit(1);
    }
    
    address = atoi(argv[3]);
    printf("Initializing SSR Board on /dev/i2c-%d @ %x...", I2CBUS, address);
    int rc = init_ssr_board(address);
    
    if (rc == 0) {
      printf(" Success!\n");
    } else {
      printf(" Failed!\n");
    }
  }
  
}
