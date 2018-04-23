//****************************************************************************//
//* Copyright (c) 2007, ���ӵ������޹�˾                                      
//* All rights reserved.                                                     
//* �ļ� CFHProSNTPWrapper.cpp                                               
//* ���� 2015-9-22        
//* ���� rwp                 
//* ע�� UDP������ʵ��                              
//****************************************************************************//
#include "CUdpReceiver.h"
#include <string>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <linux/types.h>



void CUdpReceiver::Print(UDPRECEIVER_CONF* pInface)
{
	char strLog[256]="";
	string strtemp("");
	sprintf( strLog, "pInface->netType     :%d\n", pInface->netType      ); strtemp += strLog; //��������������
	sprintf( strLog, "pInface->loacalIp[40]:%s\n", pInface->loacalIp     ); strtemp += strLog; //����IP
	sprintf( strLog, "pInface->localPort   :%d\n", pInface->localPort    ); strtemp += strLog; //���ض˿�
	sprintf( strLog, "pInface->remoteIp[40]:%s\n", pInface->remoteIp     ); strtemp += strLog; //Զ��IP
	sprintf( strLog, "pInface->remotePort  :%d\n", pInface->remotePort   ); strtemp += strLog; //Զ�̶˿�
	sprintf( strLog, "pInface->logLevel    :%d\n", pInface->logLevel     ); strtemp += strLog; //��־����
	sprintf( strLog, "pInface->timeOutRecv :%d\n", pInface->timeOutRecv  ); strtemp += strLog; //���ճ�ʱ
	sprintf( strLog, "pInface->timeOutSend :%d\n", pInface->timeOutSend  ); strtemp += strLog; //���ͳ�ʱ
	WriteLog(1,(char*)strtemp.c_str());
}


CUdpReceiver::CUdpReceiver(UDPRECEIVER_CONF* pConf)
	:m_iSocketfd(-1),m_pFunc(NULL),m_BindObj(NULL),m_bLogMessage(false)
{
	memset(&netAddr,0,sizeof(NETADDR));
	if (pConf)	{
			m_Inface = *pConf;
			Print(&m_Inface);
			m_bLogMessage = (m_Inface.logLevel>0);
	}
}


CUdpReceiver::~CUdpReceiver()
{

}

void CUdpReceiver::WriteLog(int iLevel,const char *format, ...)
{
	if (iLevel>m_Inface.logLevel){
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
		printf("[CUdpReceiver] %s %s\n",time_prt,buffer);	
    }
	else{
		printf("[CUdpReceiver] %s\n",buffer);	
	}
}



int CUdpReceiver::SOCKE_UdpInit( unsigned char sockType, 
	char *localIpAddr, int localPort, char *remoteIpAddr, int remotePort, 
	struct sockaddr_in *local,struct sockaddr_in *dest)
{
	struct ip_mreq 	mreq;
	int	i = 1;
	if (m_iSocketfd>0){
		close(m_iSocketfd);
		m_iSocketfd = -1;
	}
	//����SOCKET
	m_iSocketfd = socket(PF_INET, SOCK_DGRAM,0);
	if( m_iSocketfd < 0)
		return -2;		
	//���ؽṹ
	//bzero(( void*)local,sizeof( struct sockaddr_in));
	memset((void*)local,0,sizeof(struct sockaddr_in));
	local->sin_family = PF_INET;

	if(sockType == UDP_MULTICAST) {
		local->sin_addr.s_addr	= inet_addr(remoteIpAddr);//inet_addrת���ַ�������;����ipv4��IPת��
	} else {
		local->sin_addr.s_addr	= inet_addr(localIpAddr);
	}
	local->sin_port	= htons(localPort);
	//SO_REUSEADDR  �׽��ֿ��԰������õĵ�ַ��
	if(setsockopt(m_iSocketfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof( i)) < 0){
		return -3;
	}
	//SO_SNDBUF ָ����������С
	int nSendBuf=32*1024;
	if(setsockopt( m_iSocketfd, SOL_SOCKET, SO_SNDBUF, (char *)&nSendBuf, sizeof(nSendBuf)) < 0)
	{
		printf("send buffer false\n");		 
	}
	//����·��ַ����SOCKET��
	if(bind( m_iSocketfd, ( struct sockaddr *)local, sizeof( struct sockaddr_in)) < 0 ) {
		return -4;
	}

	//�鲥
	if( sockType == UDP_MULTICAST){		
		bzero(&mreq, sizeof( mreq));			
		mreq.imr_multiaddr.s_addr = inet_addr(remoteIpAddr);
		mreq.imr_interface.s_addr = inet_addr(localIpAddr);
		//���뵽�鲥����
		if(setsockopt(m_iSocketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof( mreq)) < 0) {
			return -8;	
		}			
	}

	//�㲥
	if( sockType == UDP_BROADCAST){
		if(setsockopt(m_iSocketfd, SOL_SOCKET, SO_BROADCAST, &i, sizeof( i)) < 0)
			return -9;
	}

	//Ŀ��ṹ	
	//bzero( ( void*)dest, 0,sizeof( dest));
	memset((void*)dest,0,sizeof( dest));
	dest->sin_family		= PF_INET;
	dest->sin_addr.s_addr	= inet_addr( remoteIpAddr);
	dest->sin_port			= htons( remotePort);
	
	//char strLog[256]="";
	//sprintf(strLog,"����UDP����ɹ���m_iSocketfd:%d",m_iSocketfd);
	WriteLog(1,"����UDP����ɹ���m_iSocketfd:%d",m_iSocketfd);
	return 1;
}

int CUdpReceiver::SOCKE_UDPWrite( int clientfd, unsigned char* buf, int size, struct sockaddr_in dest)
{
	int len= 0;	
	len = sendto( clientfd, buf, size, 0, ( struct sockaddr *)&dest, sizeof(struct sockaddr_in)); 
	return len;
}

int CUdpReceiver::SOCKE_TCPWaitForRead( int m_iSocketfd, int msec)
{
	struct	timeval tv;
	fd_set	readfds;

	tv.tv_sec  = msec /1000;
	tv.tv_usec = ( msec % 1000) * 1000;

	FD_ZERO(&readfds);
	FD_SET(m_iSocketfd, &readfds);

	switch(select( m_iSocketfd + 1, &readfds, NULL, NULL, &tv)){
		case -1:
			return -1;		//����			
		case 0:
			return 0;		//��ʱ
		default:
			if( FD_ISSET( m_iSocketfd, &readfds))
				return 1;
	}	
	return 0;		
}


int CUdpReceiver::SOCKE_UDPNonBlockRead( int clientfd, unsigned char* buf, int size, struct sockaddr_in dest, unsigned int timeOut)
{
	int	opts,ret;
	unsigned int	destlen;
	opts = fcntl(clientfd, F_GETFL);
	opts = (opts | O_NONBLOCK);	
	fcntl(clientfd, F_SETFL, opts);
	switch(SOCKE_TCPWaitForRead(clientfd, timeOut)){
		case -1:
			return -1;
		case  0:
			return 0;
		case  1:
			destlen = sizeof(struct sockaddr_in);
			ret = recvfrom( clientfd, buf, size, 0, (struct sockaddr *)&dest, &destlen);
			if (ret>0){
				sprintf( netAddr.IpAddr,"%s",inet_ntoa(dest.sin_addr));
				netAddr.nPort = dest.sin_port;
			}
			return ret;
		default:
			return -1;
	}
	//return -1;
}

int CUdpReceiver::Init(void)
{
	int ret;
	struct sockaddr_in local;
	struct sockaddr_in dest;
	ret = SOCKE_UdpInit( m_Inface.netType, m_Inface.loacalIp, 
		m_Inface.localPort, m_Inface.remoteIp, m_Inface.remotePort, &local, &dest);
	return (ret==1)?0:-1;

}

void CUdpReceiver::setupLiChangeCallBack(PFUNC_LICHANGE pFunc, void* pContext )
{
	m_pFunc   = pFunc;
	m_BindObj = pContext;
}

void CUdpReceiver::OnLi(int LiFlag)
{
	//printf("OnLi: m_pFunc:%x,m_BindObj:%x\n",(unsigned int)m_pFunc,(unsigned int)m_BindObj);
	if (m_pFunc){
		m_pFunc(LiFlag,m_BindObj);
	}
}


void CUdpReceiver::Print(COMDEF_VAL* item)
{
	char strLog[256]="";
	string strtemp("");
	sprintf( strLog, "item->infoindex: %d\n", item->infoindex  ); strtemp += strLog;   
	sprintf( strLog, "item->datatype : %d\n", item->datatype   ); strtemp += strLog;   
	sprintf( strLog, "item->value    : %d\n", item->value      ); strtemp += strLog;   
	WriteLog(3,(char*)strtemp.c_str());
}

void CUdpReceiver::Print(COMDEF_HEAD* msgHead)
{
	char strLog[256]="";
	string strtemp("");
	sprintf( strLog, "msgHead.from   : %d\n", msgHead->from     ); strtemp += strLog;
	sprintf( strLog, "msgHead.to     : %d\n", msgHead->to       ); strtemp += strLog;
	sprintf( strLog, "msgHead.equipid: %d\n", msgHead->equipid  ); strtemp += strLog;
	sprintf( strLog, "msgHead.comtype: %d\n", msgHead->comtype  ); strtemp += strLog;
	WriteLog(3,(char*)strtemp.c_str());
}

void CUdpReceiver::MsgHandle(MESSAGE& pMsg)
{
	COMDEF_HEAD msgHead;
	memcpy(&msgHead,&pMsg[0], FUHE_MSGHDR_LENH);
	if(msgHead.comtype == COMDEF_CMD_VAL ){
		
		int datNum = pMsg[FUHE_MSGHDR_LENH];	//�ó������
		unsigned int index(FUHE_MSGHDR_LENH+1);	//�ƶ�λ�õ�������
		int bit32(-1);
		int bit37(-1);
		for(int i=0; i<datNum; i++){
			if (index + FUHE_ITEM_LENH > pMsg.size()){
				WriteLog(1,"���������Ȳ���(index:%d,msg_len:%d)����������",
					(index + FUHE_ITEM_LENH),pMsg.size());
				break;
			}
			COMDEF_VAL item;
			memset(&item,0,sizeof(item));
			memcpy(&item,&pMsg[index],FUHE_ITEM_LENH); 
			index+= FUHE_ITEM_LENH;
			switch (item.infoindex){	//ֻ��������е�3����
				case 19:
					Print(&item);
					if (item.value==TIME_STATUS_ON){	//ʱ��δ��ʼ��
						OnLi(0xff);		//��������δ֪�������ö�̬������봦��ӿ�
						return;			//�����жϺ�����
					}
					break;
				case 32:
					Print(&item);
					bit32 = (bool)item.value; //[32]����Ԥ��
					break;
				case 37:
					Print(&item);
					bit37 = (bool)item.value; //[37]�����־
					if((bit32 != -1)&&(bit37 != -1)){
						int bitLi;
						if (bit32 ==0){ //��[32]����Ԥ���Ϊ0������������
							bitLi = UDP_STATUS_NOLEAP;
							WriteLog(3,"����ʱ��״̬1(bit32:%d,bit37:%d, bitLi:%d)",bit32,bit37,bitLi);
						}else{
							bitLi = ((bit32<<1) + bit37);;
							WriteLog(1,"����ʱ��״̬1(bit32:%d,bit37:%d, bitLi:%d)",bit32,bit37,bitLi);
						}
						HookLi(bitLi); 
					}
					break;
				default:
					break;
			}		
		}
	}
}


int CUdpReceiver::HookLi(int bitLi)
{
	int Li;
	switch (bitLi){
		case UDP_STATUS_NOLEAP:
			Li = SNTP_STATUS_NOLEAP;
			OnLi(Li);
			break;
		case UDP_STATUS_PLUSLEAP:
			Li = SNTP_STATUS_PLUSLEAP;
			OnLi(Li);
			break;
		case UDP_STATUS_NEGLEAP:
			Li = SNTP_STATUS_NEGLEAP;
			OnLi(Li);
			break;
		default:
			WriteLog(1,"Li��ϲ�֧�ֵ�ֵ(%d)�����Դ�״̬",bitLi);
			break;
	}
	return 0;
}

int CUdpReceiver::Start()
{
	WriteLog(1,"***UDP�����߳�����***");
	return 0;
}

void CUdpReceiver::End()
{
   WriteLog(1,"***UDP�����߳�ֹͣ***");
   if (m_iSocketfd>0)   {
	   close(m_iSocketfd);
	   m_iSocketfd = -1;
   }
}

int CUdpReceiver::RecvHandle()
{
	MESSAGE pMsg;
	int reclen;
	reclen = recvMessage(pMsg);
	if (reclen <= 0){
		return -1;
	}
	if(m_Inface.logLevel==3){
		LogMessage(reclen,pMsg,FRAME_DIRECTION_RECV);
	}
	MsgHandle(pMsg);
	return 0;
}

void CUdpReceiver::LogMessage(unsigned int msgLength ,const MESSAGE& pMsg ,int pWay)
{
	if((!m_bLogMessage) )
		return ;
	string strLog = "";
	int    nHeadLen  = 0;
	unsigned int iMsgLength = (msgLength < pMsg.size())?msgLength:pMsg.size();//ȡ����ȷ�ĳ���
	strLog.resize(iMsgLength*3+50);//����string�Ļ����С��
	char *pchar = (char *)&strLog[0];
	// print log head
	switch(pWay)
	{
		case FRAME_DIRECTION_RECV:
			sprintf(pchar,"recv from %s(%d) \r\n",
				netAddr.IpAddr,netAddr.nPort);

			nHeadLen += strlen(pchar);
			pchar    += strlen(pchar);
			break;
		case FRAME_DIRECTION_SEND:
			sprintf(pchar,"send to %s(%d) \r\n",
				netAddr.IpAddr,netAddr.nPort);

			nHeadLen += strlen(pchar);
			pchar    += strlen(pchar);

			break;
		default:
			break;
	}


	// printf log body
	try{
		// get msg 
		char temp[3] = "";
		for (unsigned int i=0;i<iMsgLength;++i)
		{
			static char hex[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

			temp[2] = 0;
			BYTE msgChar = pMsg[i];
			temp[1]=hex[(msgChar&0x0f)];//����λ
			temp[0]=hex[(msgChar&0xf0)>>4];//����λ 

			memcpy(pchar,temp,2);
			pchar += 2;
			memcpy(pchar," ",1);
			pchar += 1;
		}

		// add enter
		memcpy(pchar,"\r\n",2);

		WriteLog(3,(char*)strLog.c_str());
	}
	catch(...)
	{
		WriteLog(3,"PrintfMsg() exception");
		return;
	}			
}

short CUdpReceiver::BigEndian(short intVal)
{
// #ifdef POWER_PC
// 	return intVal;
// #else
// 	return SWAP_16(intVal);
// #endif

	return intVal;
}

int CUdpReceiver::BigEndian(int intVal)
{
// #ifdef POWER_PC
// 	return intVal;
// #else
// 	return SWAP_32(intVal);
// #endif
	return intVal;
}

float CUdpReceiver::BigEndian(float fVal)
{
// #ifdef POWER_PC
// 	return fVal;
// #else
// 	return SWAP_32(fVal);
// #endif
	return fVal;
}


int CUdpReceiver::recvMessage(MESSAGE& pMsg)
{
	if (m_iSocketfd<=0)	{
		WriteLog(1,"ͨѶ���(%d)��Ч�������½���",m_iSocketfd);
		return -1;
	}

	BYTE buff[COMDEF_INTER_MAX_BUFFER_LENGTH]="";
	COMDEF_HEAD msgHead;
	
	struct sockaddr_in recver_addr;
	recver_addr.sin_family = AF_INET;         
	recver_addr.sin_port = htons(m_Inface.localPort);//�������˶˿�  
	recver_addr.sin_addr.s_addr = INADDR_ANY; 

	unsigned int readLen = SOCKE_UDPNonBlockRead(m_iSocketfd,buff,COMDEF_INTER_MAX_BUFFER_LENGTH,recver_addr,m_Inface.timeOutRecv);
	WriteLog(1,"���հ����ȣ�%d",readLen);
	if (readLen < MIN_UDP_MSGHDR_LENH)
	{
		if (readLen == 0){
			WriteLog(1,"������ʱ�����س�ʱ");
			return -1;
		}
		else{
			WriteLog(1,"������ʱ�����س���(%d)С�ڹ�Լ��Сͷ����(%d),�Ƿ�����",
				readLen,MIN_UDP_MSGHDR_LENH);
			return -1;
		}
	}

	memcpy(&msgHead,buff,sizeof(COMDEF_HEAD));
	int dataNum = buff[FUHE_MSGHDR_LENH];
	if (dataNum ==0)	{
		pMsg.resize(MIN_UDP_MSGHDR_LENH);
		memcpy(&pMsg[0],buff,readLen);
		return MIN_UDP_MSGHDR_LENH;
	}

	int dataLength =0;
	switch (msgHead.comtype)
	{
	case COMDEF_CMD_EVENT:
		{
			dataLength = dataNum * sizeof(COMDEF_EVENT);
		}
		break;
	case COMDEF_CMD_VAL:
		{
			dataLength = dataNum * sizeof(COMDEF_VAL);
		}
		break;
	default:
		{
			
		}
	}

	if ((MIN_UDP_MSGHDR_LENH+dataLength)>COMDEF_INTER_MAX_BUFFER_LENGTH)
	{
		//����һ֡��󳤶ȷ���ʧ��
		return -1;
	}

	pMsg.resize(MIN_UDP_MSGHDR_LENH+dataLength);
	memcpy(&pMsg[0],buff,MIN_UDP_MSGHDR_LENH+dataLength);

	/*
	 *	������ΪTCP�����á�����������ʽ�����ٲ��á���������ʽ
	 */
// 	int readLen = SOCKE_UDPNonBlockRead(m_iSocketfd,buff,MIN_UDP_MSGHDR_LENH,recver_addr,m_Inface.timeOutRecv);
// 	sprintf(strLog,"����Header���ȣ�%d",readLen);
// 	WriteLog(strLog, 1);
// 	if (readLen < MIN_UDP_MSGHDR_LENH)
// 	{
// 		if (readLen == 0)
// 		{
// 			sprintf(strLog, "������ͷʱ�����س�ʱ");
// 			WriteLog(strLog, 1);
// 			return -1;
// 		}else
// 		{
// 			sprintf(strLog, "������ͷʱ�����س���(%d)С�ڹ�Լ��Сͷ����(%d),�Ƿ�����",
// 				readLen,MIN_UDP_MSGHDR_LENH);
// 			WriteLog(strLog, 1);
// 			return -1;
// 		}
// 
// 	}
// 
// 	pMsg.resize(MIN_UDP_MSGHDR_LENH);
// 	memcpy(&pMsg[0],buff,readLen);
// 
// 	int dataNum = buff[FUHE_MSGHDR_LENH];
// 	if (dataNum ==0)	{
// 		return MIN_UDP_MSGHDR_LENH;
// 	}
// 
// 	int dataLength = dataNum * sizeof(COMDEF_VAL);
// 	readLen = SOCKE_UDPNonBlockRead(m_iSocketfd,buff,dataLength,recver_addr,m_Inface.timeOutRecv);
// 	sprintf(strLog,"����Data���ȣ�%d",readLen);
// 	WriteLog(strLog, 1);
// 
// 	if (readLen < dataLength)
// 	{
// 		if (readLen == 0)
// 		{
// 			sprintf(strLog, "������ͷʱ�����س�ʱ");
// 			WriteLog(strLog, 1);
// 			return -1;
// 		}
// 		sprintf(strLog, "��������ʱ�����س���(%d)С�����ݳ���(%d),�Ƿ�����",
// 			readLen,dataLength);
// 		WriteLog(strLog, 1);
// 		return -1;
// 	}
// 	pMsg.resize(MIN_UDP_MSGHDR_LENH+dataLength);
// 	memcpy(&pMsg[MIN_UDP_MSGHDR_LENH],buff,dataLength);

	return (MIN_UDP_MSGHDR_LENH + dataLength);
}
