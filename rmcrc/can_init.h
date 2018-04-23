#ifndef __CAN_INIT_H
#define __CAN_INIT_H

#include <errno.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <getopt.h>
#include <linux/types.h>
#include "can.h"
#define true        1
#define false       0
typedef __u8        bool;
extern int  socket_fd;

void panic(const char *msg);
//bool find_can(const int port);
//void set_bitrate(const int port, const int bitrate);
void open_can(const int port, const int bitrate);
void close_can(const int port);
void close_socket(const int sockfd);
int  socket_listen(const int port);
int  socket_connect(const int port);
void disconnect(int *sockfd);

#endif