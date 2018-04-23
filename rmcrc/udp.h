#ifndef UDP_H
#define UDP_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "gongyong.h"

#ifdef __cplusplus
#define BEGIN_C_DECLS  extern "C" {
#define END_C_DECLS      } 
#else
#define BEGIN_C_DECLS
#define END_C_DECLS 
#endif

#if defined(WIN32) 
#define COM_DATA_EXPORT __declspec(dllexport)
#else
#define COM_DATA_EXPORT
#endif

#define BAUDRATE		 9600
#define CAN_PORT 		 1
#define CAN_BITRATE 	 1000000

#define N 64
#define UDP_MULTICAST				0		//组播
#define UDP_UNICAST					1		//点对点
#define UDP_BROADCAST				2		//广播

#define  COMDEF_INTER_UDP_COM_RECEIVE_TIMEOUT  5000      //内部UDP接收超时时间
#define  COMDEF_INTER_MAX_BUFFER_LENGTH        5000
#define  COMDEF_CAST_INFO     0xFFFF

//命令结果值
#define  COMDEF_CMD_RESULT_FALSE          0
#define  COMDEF_CMD_RESULT_SUCCESS        1 

//命令类型
#define  COMDEF_CMD_RESULT                1  //命令结果响应
#define  COMDEF_CMD_VAL                   2  //发送值 
#define  COMDEF_CMD_EVENT                 3  //事件信息  

//保存数据结构

#pragma pack (push, 1)
//命令头文件
typedef struct tagcomdef_head
{
	unsigned short  from;       //程序来自哪儿
    unsigned short  to;         //报文发往哪儿
	unsigned int    equipid;    //装置类型
	unsigned short  comtype;    //命令的类型

}COMDEF_HEAD;

typedef struct tagcomdef_val
{
   unsigned int  infoindex;	//节点号
   unsigned char datatype;	//节点类型
   unsigned int  value; 	//值(遥测/遥信值)
   //unsigned long long value;

} COMDEF_VAL;

typedef struct tagcomdef_event 
{
 
   unsigned int   infoindex;                                //信息索引
   char           time[32];
   unsigned char  datatype;                                 //值得了类型
   int            value;   //值
}COMDEF_EVENT;




#pragma pack(pop)







typedef struct sockaddr SA;

//struct sockaddr_in udp_send_init(int fd,char* ip, char *port);
int udp_send_init(char* ip, char *port);
void send_udp_state(void);
void send_udp_event(void);
int init_udp(void);
void add_event_udp(int id,Time serialTime);


#endif