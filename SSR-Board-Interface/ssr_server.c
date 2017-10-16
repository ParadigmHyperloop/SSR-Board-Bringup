#include "i2cfunc.h"
#include <arpa/inet.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MODE1 0x00
#define MODE2 0x01
#define SUBADR1 0x02
#define SUBADR2 0x03
#define SUBADR3 0x04
#define PRESCALE 0xFE
#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09
#define ALL_LED_ON_L 0xFA
#define ALL_LED_ON_H 0xFB
#define ALL_LED_OFF_L 0xFC
#define ALL_LED_OFF_H 0xFD

#define RESTART 0x80
#define SLEEP 0x10
#define ALLCALL 0x01
#define INVRT 0x10
#define OUTDRV 0x04

#ifndef I2CBUS
#define I2CBUS 1
#endif

#ifndef BASEADDRESS
#define BASEADDRESS 0x40
#endif

#define BUFSIZE 4096

typedef struct control_packet {
  uint8_t skateA;
  uint8_t skateB;
  uint8_t skateC;
  uint8_t skateD;

  uint8_t mpyeA;
  uint8_t mpyeB;
  uint8_t mpyeC;
  uint8_t mpyeD;

  uint8_t brakeA;
  uint8_t brakeB;

  uint8_t hpFill;
  uint8_t vent;
} control_packet_t;

void read_reg(int handle, int addr, unsigned char reg, unsigned char *buf,
              size_t len) {
  i2c_write_read(handle, addr, &reg, 1, addr, buf, len);
}

void write_reg(int handle, int addr, int reg, int val) {
  if (ioctl(handle, I2C_SLAVE, addr) < 0) {
    perror("Failed to change I2C Address: ");
    return;
  }
  printf("[0x%X] Writing 0x%X to register 0x%X\n", addr, val, reg);
  char pair[2];
  pair[0] = reg & 0xFF;
  pair[1] = val & 0xFF;
  i2c_write(handle, (unsigned char *)pair, 2);
}

void set_channel(int handle, int channel, int value) {
  int addr = BASEADDRESS + (channel > 15 ? 1 : 0);
  channel = channel % 16;
  write_reg(handle, addr, 0x06 + 4 * channel, 0);
  write_reg(handle, addr, 0x07 + 4 * channel, 0);
  write_reg(handle, addr, 0x08 + 4 * channel, 0xFF & value);
  write_reg(handle, addr, 0x09 + 4 * channel, (value >> 8) & 0xFF);
}

int start(int portno) {
  int fd;
  struct sockaddr_in self;

  // Create a socket
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Command Server socket(): ");
    return -1;
  }

  int option = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
#ifdef SO_REUSEPORT
  setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option));
#endif

  setsockopt(fd, SOL_SOCKET, SO_LINGER, &option, sizeof(option));

  struct timeval t;
  t.tv_sec = 1;
  t.tv_usec = 0;

  // Set send and recieve timeouts to reasonable numbers
  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t, sizeof(t)) < 0) {
    perror("setsockopt failed: ");
    return -1;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&t, sizeof(t)) < 0) {
    perror("setsockopt failed: ");
    return -1;
  }

  // Initialize address/port structure
  bzero(&self, sizeof(self));
  self.sin_family = AF_INET;
  self.sin_port = htons(portno);
  self.sin_addr.s_addr = INADDR_ANY;

  // Assign a port number to the socket
  if (bind(fd, (struct sockaddr *)&self, sizeof(self)) != 0) {
    perror("Command Server Socket bind(): ");
    return -1;
  }

  // Make it a "listening socket"
  if (listen(fd, 20) != 0) {
    perror("Command Server listen(): ");
    return -1;
  }

  printf("Bound port %d on fd %d\n", portno, fd);
  return fd;
}

int main(int argc, char *argv[]) {
  int handle;
  int value = 0x0;
  int channel = 0;

  int childfd;                   /* child socket */
  int portno;                    /* port to listen on */
  int clientlen;                 /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp;         /* client host info */
  char buf[BUFSIZE];             /* message buffer */
  char *hostaddrp;               /* dotted decimal host addr string */
  int optval;                    /* flag value for setsockopt */
  int n;                         /* message byte size */

  if (argc < 2) {
    printf("Usage: ssr_server <port>");
    exit(1);
  }

  handle = i2c_open(I2CBUS, BASEADDRESS);
  int addr;
  for (addr = BASEADDRESS; addr <= BASEADDRESS + 1; addr++) {
    write_reg(handle, addr, 0x01, 0x04);
    write_reg(handle, addr, 0x00, 0x01);
    int oldmode = 0;

    read_reg(handle, addr, MODE1, (unsigned char *)&oldmode, 1);

    write_reg(handle, addr, MODE1, (oldmode & 0x7F) | 0x10);
    write_reg(handle, addr, PRESCALE, 0x04);
    write_reg(handle, addr, MODE1, oldmode | 0x80);

    usleep(500);
    printf("Initialized Address 0x%X\n", addr);
  }

  portno = atoi(argv[1]);

  int serverfd = start(portno);

  /*
  * main loop: wait for a connection request, echo input line,
  * then close connection.
  */
  clientlen = sizeof(clientaddr);
  while (1) {
    printf("Waiting for connection\n");
    /*
     * accept: wait for a connection request
     */
    childfd = accept(serverfd, (struct sockaddr *)&clientaddr, &clientlen);
    if (childfd < 0) {
      if (errno == EAGAIN) {
        printf("accept(): No New Clients\n");
      } else {
        perror("ERROR on accept");
      }
      continue;
    }

    printf("Client Connected!\n");

    while (1) {
      control_packet_t packet;
      bzero(&packet, BUFSIZE);
      n = read(childfd, &packet, sizeof(control_packet_t));

      printf("read() %d bytes\n", n);
      int j;
      for (j = 0; j < n; j++) {
        uint8_t *p = &packet;
        printf("%d: %X\n", j, p[j]);
      }

      if (n < 0) {
        printf("%d\n", errno);
        if (errno == EAGAIN) {
          printf("Read Again");
        } else {
          perror("Read Failed");
          break;
        }

      } else if (n == 0) {
        break;
      } else {
        printf("-----------------------\n");
        set_channel(handle, 0, packet.skateA * 4095);
        set_channel(handle, 1, packet.skateB * 4095);
        set_channel(handle, 2, packet.skateC * 4095);
        set_channel(handle, 3, packet.skateD * 4095);

        set_channel(handle, 4, (packet.brakeA == 1) * 4095);
        set_channel(handle, 5, (packet.brakeA == 2) * 4095);
        set_channel(handle, 6, (packet.brakeB == 1) * 4095);
        set_channel(handle, 7, (packet.brakeB == 2) * 4095);

        set_channel(handle, 8, packet.hpFill * 4095);
        set_channel(handle, 9, packet.vent * 4095);

//        set_channel(handle, 13, 2000 + (packet.mpyeA * 12));
//        set_channel(handle, 14, 2000 + (packet.mpyeB * 12));
//        set_channel(handle, 30, 2000 + (packet.mpyeA * 12));
//        set_channel(handle, 31, 2000 + (packet.mpyeB * 12));
        printf("----------------------\n");
      }
    }

    printf("Client Disconnect!\n");

    /*
     * read: read input string from the client
     */

    close(childfd);
  }

  i2c_close(handle);
}

