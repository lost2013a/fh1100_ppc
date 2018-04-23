//****************************************************************************//
//* Copyright (c) 2007, ���ӵ������޹�˾                                      
//* All rights reserved.                                                     
//* �ļ� SNTPMainFlow.h                                               
//* ���� 2015-8-31        
//* ���� rwp                 
//* ע�� ��Լ������                                 
//****************************************************************************//


#include "SNTPMainFlow.h"
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <linux/types.h>


CsntpMainFlow::CsntpMainFlow()
	:m_bExit(false),m_bStarted(false),m_hRecvHandleThreadHandle(NULL),m_RecvHandleThreadId(0),
	m_udpSocket(INVALID_SOCKET), m_iDelayUS(0),m_cLeapFalg(0),
	m_iPort(SNTP_PORT),m_iSNTP_MOD(SNTPROLE_DFR_DEVICE)
{
	//Ĭ��ֻ��ӡ���漶������
	m_logLevel=CLogFile::warning;
	m_bDebug=1;

#ifdef _WIN32
	WSADATA  wsaData; 
	if ((WSAStartup(MAKEWORD(1,1),&wsaData))!=0)
	{
		WriteLog(1,"WSAStartup faild!\n");
		//return 1;
	}
#endif
}

CsntpMainFlow::~CsntpMainFlow()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

int CsntpMainFlow::Start(SNTP_CONFIG& pConfig)
{
	char    PROGRAM_DATA[40];	//����汾����
	sprintf(PROGRAM_DATA,"SNTP��̬��汾����:%c%c%c%c-%c%c-%c%c "__TIME__"",BUILD_Y0,BUILD_Y1,BUILD_Y2,BUILD_Y3,
			BUILD_M0,BUILD_M1,BUILD_D0,BUILD_D1);
	WriteLog(1,PROGRAM_DATA);
	if (m_bStarted){
		WriteLog(1,"SNTP��̬���Ѿ�����\r\n");
		return 0;
	}

	if (0 != Init(pConfig)){
		WriteLog(1,"SNTP��̬���ʼ��ʧ��\r\n");
		return -1;
	}

	if(!StartRecvHandleThread()){
		WriteLog(1,"SNTP��̬������߳�����ʧ��\r\n");
		return -1;
	}
	m_bStarted = true;
	WriteLog(1,"SNTP��̬�������ɹ�\r\n");
	return 0;	
}


int CsntpMainFlow::End()
{
	m_bExit = true;

	//�������
	if (m_udpSocket != INVALID_SOCKET)  {
#ifdef OS_WINDOWS
		::closesocket (m_udpSocket);
#endif 

#ifdef OS_LINUX
		::close(m_udpSocket);
#endif
	}
	sleep(1);

	EndRecvHandleThread();

	return -1;
}

int CsntpMainFlow::LeapSec(char li)
{
	//WriteLog(1,"�ⲿ��������λ[%d]",li);
	m_cLeapFalg = li;
	return m_bStarted?0:-1;
}



int CsntpMainFlow::Init(SNTP_CONFIG& pConfig)
{

	InitLogFile();	//�����ô�ӡ����
	char strLog[256]="";
	m_iPort 	= pConfig.sntpPort;
	m_iSNTP_MOD = pConfig.role;
	m_iDelayUS 	= pConfig.offsetUS;
	m_bDebug    = pConfig.debug;
	WriteLog(1,"SNTP��̬������:����˿�(%d) ��ʱ����(%02d)us ��������(%d)",
		m_iPort,m_iDelayUS,m_iSNTP_MOD);
#ifdef WIN32
	struct sockaddr_in sockaddr;                                        
	int     sockopt_on = 1;
	if ((m_udpSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==INVALID_SOCKET)
	{
		WriteLog(1,"udp socket faild!\r\n");
		return 1;
	}
	
	sockaddr.sin_port = htons(m_iPort);                      
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	setsockopt (m_udpSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &sockopt_on,sizeof(int));

	if (bind(m_udpSocket,(struct sockaddr*)&sockaddr,sizeof(sockaddr))==SOCKET_ERROR)
	{
		WriteLog(1,"udp bind faild!\r\n");
		return 1;
	}
#else 
	int sockopt_on = 1;
	struct sockaddr_in sockaddr;                                        
	struct sockaddr_un servaddr;
	m_udpSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if (m_udpSocket < 0) {
		WriteLog(1,"udp socket failed\r\n");
		return -1;
	}
	sockaddr.sin_port = htons(m_iPort);                       
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	setsockopt (m_udpSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &sockopt_on,sizeof(int));
	if (bind(m_udpSocket,(struct sockaddr*)&sockaddr,sizeof(sockaddr))==SOCKET_ERROR)
	{
		WriteLog(1,"udp bind faild!\n");
		return 1;
	}

	int enable = 1;
	if (setsockopt(m_udpSocket,SOL_SOCKET,SO_TIMESTAMP,&enable,sizeof(enable))<0)
	{
		close(m_udpSocket);
		WriteLog(1,"error: udp setsockopt\n");
		return -1;
	}
#endif
	WriteLog(1,"SNTP Socket set OK");
	return 0;
}





bool CsntpMainFlow::StartRecvHandleThread( void )
{
	int nRet =0;
	nRet = rw_thread_create(&m_hRecvHandleThreadHandle,
		&m_RecvHandleThreadId,
		Thread_Sntp_Recv,	//����Thread_Sntp_Recv�߳�
		this);				//thisָ�뱻���뵽Thread_Sntp_Recv�߳�
	if(nRet !=0){
		WriteLog(1,"SNTP����RecvHandle�߳�ʧ�ܣ�ԭ��Ϊ:%s",strerror(errno));
		return false;
	}
	WriteLog(1,"SNTP����RecvHandle�̳߳ɹ�");
	return true;		
}

THREAD_FUNC WINAPI CsntpMainFlow::Thread_Sntp_Recv(LPVOID pParam)
{

#define SET_TO_SCHED_RR 	1
#define PRIORITY_SCHED_RR 	20 //0-99��0=��ͨ����
	CsntpMainFlow* pThis = (CsntpMainFlow*)pParam;//�������thisָ��
#ifdef SET_TO_SCHED_RR 
	struct sched_param sched; 
	sched_getparam(0, &sched);				//0 = the calling process 
	pThis->WriteLog(1,"SCHED_RR: %d -> %d\n",sched.sched_priority,PRIORITY_SCHED_RR);
	sched.sched_priority = PRIORITY_SCHED_RR; 
	sched_setscheduler(0, SCHED_RR, &sched);//���ó�ʵʱ����ģʽ
#endif 
	int  nRet =0;
	try{
		nRet=pThis->Sntp_Recv_HandleLoop();
	}
	catch(...){	//ʧ�ܺ�����
		pThis->WriteLog(1,"RecvHandleThreadFunc() exception,error code=%d",errno);
		pThis->SetRecvHandleThreadHandle(INVALID_THREAD);
		return THREAD_RETURN;
	}
	pThis->WriteLog(1,"RecvHandleThreadFunc() exit with ret code =%d",nRet);
	pThis->SetRecvHandleThreadHandle(INVALID_THREAD);
	return THREAD_RETURN;
}


bool CsntpMainFlow::Sntp_Recv_HandleLoop()
{
	WriteLog(1,"����Sntp_Recv_HandleLoop!");
	while (!IfExit()){
		try{
			if (recvSend(m_udpSocket) < 0){
				WriteLog(1,"respond erro!");
			}
			
		}
		catch(...)
		{
			WriteLog(1,"���մ������쳣");
		}
	}
	WriteLog(1,"�˳�Sntp_Recv_HandleLoop!",CLogFile::trace);
	return 0;	
}


bool CsntpMainFlow::IfExit()
{
	return m_bExit;
}

void CsntpMainFlow::SetRecvHandleThreadHandle( THREAD_HANDLE pHandle )
{
	m_hRecvHandleThreadHandle = pHandle;
}

void CsntpMainFlow::EndRecvHandleThread( void )
{
	//ֹͣ�����߳�
#ifdef OS_LINUX
	if(m_RecvHandleThreadId != 0){
		int nRet=rw_thread_join(m_RecvHandleThreadId,NULL);
		if(nRet != 0){
			WriteLog(1,"�˳����մ����߳��쳣��ԭ��Ϊ:%s",strerror(errno));
			return;
		}
	}

#endif

#ifdef WIN32
	time_t tBegin;
	time_t tNow;
	time(&tBegin);
	do 
	{
		if (m_hRecvHandleThreadHandle == INVALID_THREAD)
		{
			break;
		}
		sleep(0.05);
		time(&tNow);
	} while ((tNow-tBegin)<30);

	if (m_hRecvHandleThreadHandle != INVALID_THREAD)
	{
		rw_thread_detach(m_hRecvHandleThreadHandle);
	}

#endif
	WriteLog(1,"�˳����մ����̳߳ɹ�");		
}



int CsntpMainFlow::recvSend(int socket)
{
	int ret(-1);
	struct timeval tv;
	struct timeval tvNow;
	unsigned char buf[SNTP_HEADER_SIZE];
	sntp   sntpMsg;
#ifdef OS_LINUX
	struct sockaddr_storage ss;
	socklen_t size = sizeof(ss);
#else
	struct sockaddr ss;
	int size = sizeof(ss);
#endif

	CSNTPMsgAttach msgAttach;	//����sntp����
	//sntp.reference = msgAttach.sntp_now(); modify by rwp 20150831


#ifdef OS_LINUX
	struct sockaddr_in fromAddr;//����Դip
	//����netRecv�������ؽ���ʱ��ʱ���
	size = netRecv(socket, (char*)buf, sizeof(buf),&fromAddr, &tv);
	if (size <= 0)	{
		return -1;
	}
#else
	char strLog[256]="";
	if (recvfrom(socket, (char*)buf, sizeof(buf), 0, (struct sockaddr *) &ss, &size) < 0)
		return -1;
#endif
	unsigned char oritime[8]="";
	char identifierName[5]="";
	memcpy(oritime,(char*)&(buf[40]),8);	//��ȡ��oritime������ת���о��ȶ�ʧ�������ֿͻ����жԸ�ֵ���м��
	msgAttach.sntp_unpack(&sntpMsg, buf);	//�������ȡ��sntpMsg
	if(m_bDebug==3){
 		//WriteLog(1,(char*)msgAttach.sntp_inspect(&sntpMsg));//��ӡ���ձ���
	}
	switch (m_iSNTP_MOD){
		case SNTPROLE_DFR_DEVICE: /*1  //¼����                 */
		case SNTPROLE_TMS_DEVICE: /*3  //TSM�նˣ�ʱ�����ϵͳ��*/
			//���˷Ƿ���ʶ��
			
			memcpy(identifierName,sntpMsg.identifier,4);
			if (0 != strcmp("TSSM",identifierName)){
				WriteLog(1,"��ʶ��Ϊ[%s],���Ϸ���ʱ��ϵͳ��ʶ",identifierName);
				return 0;
			}
			break;
		
		case SNTPROLE_UKWN_DEVICE: /*0  //δ֪                   */
		case SNTPROLE_GPS_DEVICE:  /*2  //GPSװ��                */
		default:	
			//��ʱ��װ�÷���ˣ�δ��ʱ״̬�£������ظ�
			if (LI() == 255){
				WriteLog(1,"GPS�豸ʱ�ⲿ��δ��ʼ����255��״̬,��������");
				return 0;
			}
			
			break;
	}
	
	sntpMsg.li = LI();
	sntpMsg.vn = 4;	//�̶�4
	sntpMsg.mode = (sntpMsg.mode == SNTP_MOD_CLIENT) ? SNTP_MOD_SERVER : SNTP_MOD_PAS;
	sntpMsg.stratum = 2;
	sntpMsg.poll = 0x06;
	sntpMsg.precision = /*-6*/0xec;
	sntpMsg.root_delay = 0.0;
	sntpMsg.root_dispersion = 0.0;

	if ((m_iSNTP_MOD == SNTPROLE_DFR_DEVICE)||(m_iSNTP_MOD == SNTPROLE_TMS_DEVICE))
	{
		strncpy((char *) sntpMsg.identifier, "TSSM", sizeof(sntpMsg.identifier));
	}else
	{
		sntpMsg.identifier[0]=0x47;
		sntpMsg.identifier[1]=0x50;
		sntpMsg.identifier[2]=0x53;
		sntpMsg.identifier[3]=0x00;		
	}
	sntpMsg.originate/*t2*/ = sntpMsg.transmit/*�ͻ���t4*/;	 
#ifdef OS_LINUX
	sntpMsg.receive  /*t3*/ = DIFF19001970 + tv.tv_sec+(double)tv.tv_usec/1000000;
#else
	sntpMsg.receive  /*t3*/ = sntpMsg.reference;
#endif
	
	sntpMsg.transmit /*t4*/ = msgAttach.sntp_now(m_iDelayUS);//ȡϵͳʱ���Ϊ����ʱ���
	sntpMsg.reference/*t1*/ = sntpMsg.transmit/*t4*/;
	msgAttach.sntp_pack(buf, &sntpMsg);

	//
// 	printf("time segmetn2: %02x%02x%02x%02x%02x%02x%02x%02x\n",
// 		oritime[0],
// 		oritime[1],
// 		oritime[2],
// 		oritime[3],
// 		oritime[4],
// 		oritime[5],
// 		oritime[6],
// 		oritime[7]
// 		);
	memcpy(&buf[24],oritime,8);

	//��ӡ���ͱ���
	if(m_bDebug==3)
	{
		//WriteLog(1,(char*)msgAttach.sntp_inspect(&sntpMsg));
	}
#ifdef WIN32
	ret = sendto(socket, (char*)buf, sizeof(buf), 0, (struct sockaddr *) &ss, size);
#else
	ret = netSend(socket,(char*)buf, SNTP_HEADER_SIZE,&fromAddr);
#endif
	return ret;
}


int CsntpMainFlow::netRecv(int netsock, char *pbuf, int buflen,sockaddr_in* fromAddr, struct timeval *tv)
{
#ifndef WIN32
	int ret(-1);
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct iovec iov;
	struct sockaddr_in from_addr;
	char ctrl[CMSG_SPACE(sizeof(struct timeval))];

	memset(&msg, 0, sizeof(msg));
	memset(&from_addr, 0, sizeof(from_addr));
	memset(&ctrl, 0, sizeof(ctrl));

	iov.iov_base= pbuf;	//IO����(ԭ�Ӳ���)
	iov.iov_len = buflen;

	msg.msg_name = &from_addr;                         
	msg.msg_namelen = sizeof(from_addr);

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	msg.msg_control = (caddr_t)ctrl;
	msg.msg_controllen = sizeof(ctrl);

//	sprintf( strLog, "****fromaddr.sin_family      :%d\n", from_addr.sin_family                   );printf(strLog);
//	sprintf( strLog, "****fromaddr.sin_port        :%d\n", from_addr.sin_port                     );printf(strLog);
//	sprintf( strLog, "****fromaddr.in_addr sin_addr:%s\n", inet_ntoa(from_addr.sin_addr)          );printf(strLog);
//	sprintf( strLog, "****fromaddr.sin_zero[8]     :%s\n", from_addr.sin_zero                     );printf(strLog);

	ret = recvmsg(netsock, &msg, 0);

	if (ret <= 0)
		return ret;

//	sprintf( strLog, "fromaddr.sin_family      :%d\n", from_addr.sin_family                   );printf(strLog);
//	sprintf( strLog, "fromaddr.sin_port        :%d\n", from_addr.sin_port                     );printf(strLog);
//	sprintf( strLog, "fromaddr.in_addr sin_addr:%s\n", inet_ntoa(from_addr.sin_addr)          );printf(strLog);
//	sprintf( strLog, "fromaddr.sin_zero[8]     :%s\n", from_addr.sin_zero                     );printf(strLog);
	*fromAddr = from_addr; //��ȡ���ԶԷ���IP��ַ
	//����msg�е���Чcmsghdr����ȡ���е�ʱ�����Ϣ
	//CMSG_FIRSTHDR 	���ص�һ��cmsghdr��ָ��
	//CMSG_NXTHDR   	������һ����Ч��cmsghdr
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)){
		if ((cmsg->cmsg_level == SOL_SOCKET) && (cmsg->cmsg_type == SO_TIMESTAMP)){
			WriteLog(1,"���յ�һ֡��ʱ�걨��ret:%d, ip:(%s)",ret,inet_ntoa(from_addr.sin_addr));
			memcpy(tv, (struct timeval *)CMSG_DATA(cmsg), sizeof(struct timeval));
			break;
		}
	}

	return ret;
#else
	{
		return -1;
	}
#endif
}

int CsntpMainFlow::netSend(int netsock,char *pbuf, int buflen,sockaddr_in* fromAddr)
{
#ifndef WIN32
	{
		char strLog[256]="";
		int clifd;
		int ret=0;
		struct msghdr msg;
		struct iovec iov[1];
		char buf[100];
		union {
			struct cmsghdr cm;
			char control[CMSG_SPACE(sizeof(struct timeval))];
		} control_un;
		struct cmsghdr *pcmsg;
		int fd;

		//  	clifd = socket(AF_UNIX, SOCK_STREAM, 0);
		//  	if (clifd < 0) {
		//  		printf("socket failed.\n");
		//  		return -1;
		//  	}
		// 	fd = open(OPEN_FILE, O_CREAT| O_RDWR, 0777);
		// 	if (fd < 0) {
		// 		printf("open test failed.\n");
		// 		return -1;
		// 	}

		// 	bzero(&servaddr, sizeof(servaddr));
		// 	servaddr.sun_family = AF_UNIX;
		// 	strcpy(servaddr.sun_path, UNIXSTR_PATH);
		// 
		// 	ret = connect(clifd,(SA *)&servaddr, sizeof(servaddr));
		// 	if (ret < 0) {
		// 		printf("connect failed.\n");
		// 		return 0;
		// 	}


		/*-*/
		// 			struct sockaddr_in from_addr;
		// memset(&from_addr, 0, sizeof(from_addr));
		// memset(&ctrl, 0, sizeof(ctrl));
		// 
		// iov.iov_base= pbuf;
		// iov.iov_len = buflen;
		// 
		// msg.msg_name = &from_addr;                         
		// msg.msg_namelen = sizeof(from_addr); 
		/*-*/
		struct sockaddr_in to_addr;
		memset(&to_addr, 0, sizeof(to_addr));
		to_addr.sin_port = htons(/*SNTP_PORT*//*port*/fromAddr->sin_port);                        //�˿ں�
		to_addr.sin_family = AF_INET;
		//to_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		//to_addr.sin_addr.s_addr = inet_addr("192.168.7.12"); //��д��Դ��IP
		to_addr.sin_addr.s_addr = fromAddr->sin_addr.s_addr; //��д��Դ��IP

//		sprintf( strLog, "to_addr->sin_family      :%d\n", to_addr.sin_family                   );printf(strLog);
//		sprintf( strLog, "to_addr->sin_port        :%d\n", to_addr.sin_port                     );printf(strLog);
//		sprintf( strLog, "to_addr->in_addr sin_addr:%s\n", inet_ntoa(to_addr.sin_addr)          );printf(strLog);
//		sprintf( strLog, "to_addr->sin_zero[8]     :%s\n", to_addr.sin_zero                     );printf(strLog);

		msg.msg_name = &to_addr;                         
		msg.msg_namelen = sizeof(sockaddr_in); 
		iov[0].iov_base = pbuf;
		iov[0].iov_len = buflen;
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;
		msg.msg_control = control_un.control;
		msg.msg_controllen = sizeof(control_un.control);

		struct timeval ktv;
		pcmsg = CMSG_FIRSTHDR(&msg);
		pcmsg->cmsg_len = CMSG_LEN(sizeof(struct timeval));
		pcmsg->cmsg_level = SOL_SOCKET;
		pcmsg->cmsg_type = SCM_RIGHTS;
		*((struct timeval *)CMSG_DATA(pcmsg)) =ktv;

		ret = sendmsg(netsock, &msg, 0);
//		printf("ret = %d\n", ret);
		return ret;
	}
#else
	return -1;
#endif
}



char CsntpMainFlow::LI()
{
	return m_cLeapFalg;
}


int CsntpMainFlow::InitLogFile()
{
	m_LogFile.SetLogLevel(m_logLevel);
	return 0;
}


void CsntpMainFlow::WriteLog(int iLevel,const char *format, ...)
{
	if (iLevel>m_logLevel){
		return;
	}	
#define LOGLENGTH 1024
	char buffer[LOGLENGTH];
  	va_list ap;
	va_start(ap,format);
	vsnprintf(buffer,LOGLENGTH,format,ap);//vsprintf�ĳ������ư�
	va_end(ap);
    if(1){	//����ʱ�����������CPU����
		struct timeval  tv;
	    struct timezone tz;
		gettimeofday(&tv,&tz);
		struct tm *tm = localtime(&tv.tv_sec);
		char time_prt[50]="";
		sprintf(time_prt,"%.4d-%.2d-%.2d %.2d:%.2d:%.2d.%03ld ",
			tm->tm_year + 1900, tm->tm_mon + 1,tm->tm_mday, tm->tm_hour,
			tm->tm_min, tm->tm_sec,tv.tv_usec/1000);
		printf("[SNTPMainFlow] %s %s\n",time_prt,buffer);	
    }
	else{
		printf("[SNTPMainFlow] %s\n",buffer);	
	}
}






