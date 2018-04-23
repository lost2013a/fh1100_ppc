//****************************************************************************//
//* Copyright (c) 2007, 府河电气有限公司                                      
//* All rights reserved.                                                     
//* 文件 CFHProSNTPWrapper.cpp                                               
//* 日期 2015-9-22        
//* 作者 rwp                 
//* 注释 UDP接收器实现                              
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

//帧格式定义
#include "mucomdef.h"

#include <vector>
using namespace std;

#define UDP_MULTICAST				0		//组播
#define UDP_UNICAST					1		//点对点
#define UDP_BROADCAST				2		//广播

#define SWAP_32(x) ((((x) & 0xFF000000)>>24) |  (((x) & 0x00FF0000)>>8) | (((x) & 0x0000FF00)<<8 ) |  (((x) & 0x000000FF)<<24))
#define SWAP_16(x) ((((x)& 0xFF00)>>8) |  (((x) & 0x00FF)<<8))

typedef struct _UDPRECEIVER_CONF 
{
	int  netType;      //接收者网络类型
	char loacalIp[40]; //本地IP
	int  localPort;    //本地端口
	char remoteIp[40]; //远程IP
	int  remotePort;   //远程端口

	int  logLevel;     //日志级别
	int  timeOutRecv;  //接收超时
	int  timeOutSend;  //发送超时
}UDPRECEIVER_CONF;

typedef struct _NetAddr{
	char IpAddr[40]; //IP
	int  nPort;      //端口
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

//闰秒状态（SNTP/UDP(内部)）
#define SNTP_STATUS_NOLEAP   0  //非闰秒状态
#define SNTP_STATUS_PLUSLEAP 1  //正闰秒状态
#define SNTP_STATUS_NEGLEAP  2  //正闰秒状态
#define SNTP_STATUS_UKNOWN   3  //闰秒未知（警告）
#define UDP_STATUS_NOLEAP    0  //正常无闰秒状态  
#define UDP_STATUS_PLUSLEAP  2  //正闰秒状态      
#define UDP_STATUS_NEGLEAP   3  //负闰秒状态      
#define UDP_STATUS_UKNOWN    1  //闰秒未知（警告）

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
