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
#define UDP_MULTICAST				0		//�鲥
#define UDP_UNICAST					1		//��Ե�
#define UDP_BROADCAST				2		//�㲥

#define  COMDEF_INTER_UDP_COM_RECEIVE_TIMEOUT  5000      //�ڲ�UDP���ճ�ʱʱ��
#define  COMDEF_INTER_MAX_BUFFER_LENGTH        5000
#define  COMDEF_CAST_INFO     0xFFFF

//������ֵ
#define  COMDEF_CMD_RESULT_FALSE          0
#define  COMDEF_CMD_RESULT_SUCCESS        1 

//��������
#define  COMDEF_CMD_RESULT                1  //��������Ӧ
#define  COMDEF_CMD_VAL                   2  //����ֵ 
#define  COMDEF_CMD_EVENT                 3  //�¼���Ϣ  

//�������ݽṹ

#pragma pack (push, 1)
//����ͷ�ļ�
typedef struct tagcomdef_head
{
	unsigned short  from;       //���������Ķ�
    unsigned short  to;         //���ķ����Ķ�
	unsigned int    equipid;    //װ������
	unsigned short  comtype;    //���������

}COMDEF_HEAD;

typedef struct tagcomdef_val
{
   unsigned int  infoindex;	//�ڵ��
   unsigned char datatype;	//�ڵ�����
   unsigned int  value; 	//ֵ(ң��/ң��ֵ)
   //unsigned long long value;

} COMDEF_VAL;

typedef struct tagcomdef_event 
{
 
   unsigned int   infoindex;                                //��Ϣ����
   char           time[32];
   unsigned char  datatype;                                 //ֵ��������
   int            value;   //ֵ
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