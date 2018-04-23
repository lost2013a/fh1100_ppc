#ifndef __SERIAL_H
#define __SERIAL_H	 			
#include "msg_fpga.h"
#include "msg_lcd.h"
#define FALSE  -1
#define TRUE   0
/*serial_init.c*/
int UART0_Open(char* port);
void UART0_Close(int fd);
int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
int UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity);
int UART0_Send(int fd, void *send_buf,int data_len);
int UART0_Recv(int fd, void *recv_buf, int data_len);
/*serial.c*/
void accept_serail();
int  deal_config_fpga(void);
void send_serial_fpga();
int SetSystemTime();	
void handleap(struct timeval *tv);

int myfilter(int *off,int d,char *b,char *i);
#endif