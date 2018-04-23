//****************************************************************************//
//* Copyright (c) 2007, 府河电气有限公司                                      
//* All rights reserved.                                                     
//* 文件 SNTPMainFlow.h                                               
//* 日期 2015-8-31        
//* 作者 rwp                 
//* 注释 规约主流程                                 
//****************************************************************************//

#include <stdlib.h>
#include <string.h>
#ifdef OS_LINUX
#include <syslog.h>
#include <unistd.h>

typedef int                     SOCKET;
#define INVALID_SOCKET          -1              //(~0)
#define SOCKET_ERROR            -1              //(-1) 
#include <sys/types.h>  
#include <netinet/in.h>  
#include <sys/socket.h>  
#include <sys/wait.h>  
#include <arpa/inet.h>       
#include <unistd.h>         
#include <sys/select.h>
#include <sys/time.h>
//#include <sys/ioctl.h> 
#include <termios.h>
#include <netdb.h>
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/un.h>
//#include <sys/unp.h>
//#include <sys/socket.h>
#include <netinet/in.h>

#else
#include <winsock.h>
#include <stdio.h>
#endif

#include "sntpMsgAttach.h"
#include "../common/LogFile.h"
#include "../common/platform.h"
#include "FHSNTPInface.h"

#define BUFFER_MAX 2048
//转化编译时间
#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')

#define BUILD_M0 \
((BUILD_MONTH_IS_OCT || BUILD_MONTH_IS_NOV || BUILD_MONTH_IS_DEC) ? '1' : '0')
#define BUILD_M1 \
( \
(BUILD_MONTH_IS_JAN) ? '1' : \
(BUILD_MONTH_IS_FEB) ? '2' : \
(BUILD_MONTH_IS_MAR) ? '3' : \
(BUILD_MONTH_IS_APR) ? '4' : \
(BUILD_MONTH_IS_MAY) ? '5' : \
(BUILD_MONTH_IS_JUN) ? '6' : \
(BUILD_MONTH_IS_JUL) ? '7' : \
(BUILD_MONTH_IS_AUG) ? '8' : \
(BUILD_MONTH_IS_SEP) ? '9' : \
(BUILD_MONTH_IS_OCT) ? '0' : \
(BUILD_MONTH_IS_NOV) ? '1' : \
(BUILD_MONTH_IS_DEC) ? '2' : \
/* error default */ '?' \
)
#define BUILD_Y0 (__DATE__[ 7])
#define BUILD_Y1 (__DATE__[ 8])
#define BUILD_Y2 (__DATE__[ 9])
#define BUILD_Y3 (__DATE__[10])

#define BUILD_D0 ((__DATE__[4] >= '0') ? (__DATE__[4]) : '0')
#define BUILD_D1 (__DATE__[ 5])

class CsntpMainFlow 
{
public:
	CsntpMainFlow ();
	virtual ~CsntpMainFlow();
	int Start(SNTP_CONFIG& pConfig);
	int End();
	int LeapSec(char li);
protected:
	bool m_bStarted;
	bool m_bExit;
	int  recvSend(int socket);
	int  netRecv(int netsock, char *pbuf, int buflen,sockaddr_in* fromAddr, struct timeval *tv);
	int  netSend(int netsock,char *pbuf, int buflen, sockaddr_in* to_addr);

private:
	THREAD_ID		m_RecvHandleThreadId;		//接收处理线程Id
	THREAD_HANDLE	m_hRecvHandleThreadHandle;	//接收处理线程句柄

	SOCKET  m_udpSocket;
	int  Init(SNTP_CONFIG& pConfig);
	bool StartRecvHandleThread(void);
	static THREAD_FUNC WINAPI Thread_Sntp_Recv(LPVOID pParam );
	bool Sntp_Recv_HandleLoop();
	bool IfExit();
	void SetRecvHandleThreadHandle( THREAD_HANDLE pHandle );
	void EndRecvHandleThread( void );
	char LI();

	int  	m_iSNTP_MOD;	//SNTP服务类型
	int  	m_iDelayUS;		//延时配置，补偿时间
	int  	m_iPort;		//网络端口
	char 	m_cLeapFalg;	//闰秒标志

	CLogFile m_LogFile;		//日志
	bool m_bDebug ;			//调试标志,=1的时候才打印
	int  m_logLevel;		//打印级别
	int  InitLogFile();		//设置打印级别=m_logLevel
	void WriteLog(int iLevel,const char *format, ...);
	//void WriteLog(char* pLog,int iLevel=CLogFile::trace);
};
