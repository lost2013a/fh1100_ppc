//****************************************************************************//
//* Copyright (c) 2007, ���ӵ������޹�˾                                      
//* All rights reserved.                                                     
//* �ļ� CFHProSNTPWrapper.cpp                                               
//* ���� 2015-9-22        
//* ���� rwp                 
//* ע�� UDP������ʵ��                              
//****************************************************************************//
#if !defined(_UDPRECVER_H__)
#define _UDPRECVER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "string.h"

//֡��ʽ����
#include "mucomdef.h"

#include <vector>
using namespace std;

#define UDP_MULTICAST				0		//�鲥
#define UDP_UNICAST					1		//��Ե�
#define UDP_BROADCAST				2		//�㲥

#define SWAP_32(x) ((((x) & 0xFF000000)>>24) |  (((x) & 0x00FF0000)>>8) | (((x) & 0x0000FF00)<<8 ) |  (((x) & 0x000000FF)<<24))
#define SWAP_16(x) ((((x)& 0xFF00)>>8) |  (((x) & 0x00FF)<<8))

typedef struct _UDPRECEIVER_CONF 
{
	int  netType;      //��������������
	char loacalIp[40]; //����IP
	int  localPort;    //���ض˿�
	char remoteIp[40]; //Զ��IP
	int  remotePort;   //Զ�̶˿�

	int  logLevel;     //��־����
	int  timeOutRecv;  //���ճ�ʱ
	int  timeOutSend;  //���ͳ�ʱ
}UDPRECEIVER_CONF;

typedef struct _NetAddr{
	char IpAddr[40]; //IP
	int  nPort;      //�˿�
}NETADDR;


typedef int (*PFUNC_LICHANGE)( int newLi, void* pContext);
typedef unsigned char BYTE;
typedef vector<BYTE> MESSAGE;
#define FRAME_DIRECTION_RECV 0
#define FRAME_DIRECTION_SEND 1
#define FUHE_MSGHDR_LENH sizeof(COMDEF_HEAD) 
#define FUHE_ITEM_LENH sizeof(COMDEF_VAL)

#define MIN_UDP_MSGHDR_LENH (FUHE_MSGHDR_LENH+1)
#define TIME_STATUS_ON  1
#define TIME_STATUS_OFF 0

//����״̬��SNTP/UDP(�ڲ�)��
#define SNTP_STATUS_NOLEAP   0  //������״̬
#define SNTP_STATUS_PLUSLEAP 1  //������״̬
#define SNTP_STATUS_NEGLEAP  2  //������״̬
#define SNTP_STATUS_UKNOWN   3  //����δ֪�����棩
#define UDP_STATUS_NOLEAP    0  //����������״̬  
#define UDP_STATUS_PLUSLEAP  2  //������״̬      
#define UDP_STATUS_NEGLEAP   3  //������״̬      
#define UDP_STATUS_UKNOWN    1  //����δ֪�����棩

class CUdpReceiver  
{
public:
	CUdpReceiver(UDPRECEIVER_CONF* pConf);
	virtual ~CUdpReceiver();
	int  Init(void);
	void setupLiChangeCallBack(PFUNC_LICHANGE pFunc, void* pContext);
	int  Start();
	void End();
	int  RecvHandle();
private:

	void WriteLog(int iLevel,const char *format, ...);
	int  SOCKE_UDPNonBlockRead( int clientfd, unsigned char* buf, int size, struct sockaddr_in dest, unsigned int timeOut);
	void OnLi(int LiFlag);
	void MsgHandle(MESSAGE& pMsg);
	int  recvMessage(MESSAGE& pMsg);
	void LogMessage(unsigned int msgLength ,const MESSAGE& pMsg ,int pWay);
	int  SOCKE_UdpInit( unsigned char sockType, char *localIpAddr, int localPort, char *remoteIpAddr, int remotePort, struct sockaddr_in *local, struct sockaddr_in *dest);
	int  SOCKE_TCPWaitForRead( int socketfd, int msec);	
	int  SOCKE_UDPWrite( int clientfd, unsigned char* buf, int size, struct sockaddr_in dest);
	void Print(UDPRECEIVER_CONF* pInface);
	void Print(COMDEF_VAL* item);
	void Print(COMDEF_HEAD* msgHead);
	int  HookLi(int bitLi);
	short BigEndian(short intVal);
	int   BigEndian(int intVal);
	float BigEndian(float fVal);
private:
	int   m_iSocketfd;
	PFUNC_LICHANGE m_pFunc;
	void* m_BindObj;
	bool  m_bLogMessage;
	NETADDR netAddr;
	UDPRECEIVER_CONF m_Inface;
};

#endif // !defined(_UDPRECVER_H__)
