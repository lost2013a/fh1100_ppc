#include <netinet/in.h>
#include "bsp.h"
#include <sys/prctl.h>
#include <string.h>

int 	sock_fd;
struct sockaddr_in dstaddr_s;

int			udp_event_count;
UDP_EVENT	udp_event[EVENT_COUNT];//UDP������Ϣ����

int		 socketfd;
struct sockaddr_in dest;



void *Pthread_5_udp(void *arg)						
{
	LOG_MESSAGE log_buf;//д������־buf
	volatile int old_ms;
	int  udp_initial;
	long tmp = (long)arg;//����-W�澯
	tmp=~tmp;
	read_time();
	old_ms = sys_time.msec;
	udp_initial=init_udp();
	prctl(PR_SET_NAME, (unsigned long)"rmcrc_5udp");
  	while(1)
	{
		read_time();	//����sys_timeʱ��
		if(old_ms<600 && sys_time.msec>=600){	//ÿ���300ms������һ��
			if(udp_initial==1){
				if(param.initial>=3){
					send_udp_state();
					send_udp_event();
				}
				#ifdef __MY_PROG_PTHREAD
				if((sys_time.min%10==5)&&(sys_time.sec==0)){
					sprintf(log_buf.name,"Pthread_5_udp ����[%02d]-[%02d:%02d:%02d]",sys_time.day,sys_time.hour,
						sys_time.min,sys_time.sec);
					add_my_log(log_buf);	
				}
				#endif
			}	
			if(sys_time.hour==0&&sys_time.min==0&&sys_time.sec==0){
				create_log(1);	//��Ҫ������ļ�������
			}	
			if(prog_log_count>0){
				save_my_log();
			}
			//myprintf("sys_time=%d,leapNum=%d,leap61=%d,leap59=%d\n",sys_time.sec,leapNum,leap61,leap59);
		}
		old_ms = sys_time.msec;
		usleep(10*1000);
	}
	sprintf(log_buf.name,"rmcrc�߳�5�˳�...");
	add_my_log(log_buf);
	save_my_log();
	pthread_exit(NULL);
	return 0;
}

int udp_send_init(char* ip, char *port)
{
	if((sock_fd = socket(PF_INET,SOCK_DGRAM, 0)) == -1)
	{
		perror("fail to socket");
		return -1;
	}
	bzero(&dstaddr_s, sizeof(dstaddr_s));
	dstaddr_s.sin_family = PF_INET;
	dstaddr_s.sin_port = htons(atoi(port));				//�˿�
	dstaddr_s.sin_addr.s_addr = inet_addr(ip);			//�鲥��ַ
	return 0;
}		

int SOCKE_UdpInit( unsigned char sockType, 
					 char *localIpAddr, int localPort, 
					 char *remoteIpAddr, int remotePort, 
					 int *socketfd, 
					 struct sockaddr_in *local,
					 struct sockaddr_in *dest)
{
	struct ip_mreq 		mreq;
	int			i = 1;
	
	if( socketfd == NULL)
 		return -1;			

	//����SOCKET
	*socketfd = socket( PF_INET, SOCK_DGRAM, 0);
	if( *socketfd < 0)
 		return -2;		

	//���ؽṹ
	bzero( ( void*)local, sizeof( struct sockaddr_in));	
	local->sin_family		= PF_INET;
	
	if( sockType == UDP_MULTICAST) {
		local->sin_addr.s_addr	= inet_addr( remoteIpAddr);
	} else {
				local->sin_addr.s_addr	= inet_addr( localIpAddr);
	}
	
	local->sin_port			= htons( localPort);

	//���ö˿ڸ���
	if( setsockopt( *socketfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof( i)) < 0) {
		return -3;
	}
	//-----------------------???????
	int nSendBuf=32*1024;
	if (setsockopt( *socketfd, SOL_SOCKET, SO_SNDBUF, (char *)&nSendBuf, sizeof(nSendBuf)) < 0)
	{
		printf("send buffer false\n");		 
	}

	//���˿���SOCKET��
	if( bind( *socketfd, ( struct sockaddr *)local, sizeof( struct sockaddr_in)) < 0 ) {
 		return -4;
 	}
 	 	 	 	
	//�鲥
	if( sockType == UDP_MULTICAST){		
		bzero( &mreq, sizeof( mreq));			
		mreq.imr_multiaddr.s_addr = inet_addr( remoteIpAddr);
		mreq.imr_interface.s_addr = inet_addr( localIpAddr);

		//���뵽�鲥����
		if( setsockopt( *socketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof( mreq)) < 0) {
			return -8;	
		}			
	}

	//�㲥
	if( sockType == UDP_BROADCAST)
	{
		if( setsockopt( *socketfd, SOL_SOCKET, SO_BROADCAST, &i, sizeof( i)) < 0)
			return -9;
	}
	
	//Ŀ��ṹ				
	bzero( ( void*)dest, sizeof( dest));	
	dest->sin_family		= PF_INET;
	dest->sin_addr.s_addr	= inet_addr( remoteIpAddr);
	dest->sin_port			= htons( remotePort);
		
	return 1;
}

int SOCKE_UDPWrite( int clientfd,  char* buf, int size, struct sockaddr_in dest, unsigned int timeOut)
{
	long tmp = (long)timeOut;//����-W�澯
	tmp=~tmp;
	int len= 0;	
	len = sendto( clientfd, buf, size, 0, ( struct sockaddr *)&dest, sizeof(struct sockaddr_in)); 
	return len;
}

int SOCKE_TCPWaitForRead( int socketfd, int msec)
{
	struct		timeval tv;
	fd_set		readfds;
	
	tv.tv_sec  = msec /1000;
	tv.tv_usec = ( msec % 1000) * 1000;

	FD_ZERO( &readfds);
	FD_SET( socketfd, &readfds);

	switch( select( socketfd + 1, &readfds, NULL, NULL, &tv))
	{
	case -1:

		return -1;		//����			

	case 0:

		return 0;		//��ʱ

	default:
		
		if( FD_ISSET( socketfd, &readfds))
			return 1;
	}	
	
	return 0;		
}


int SOCKE_UDPNonBlockRead( int clientfd, unsigned char* buf, int size, struct sockaddr_in dest, unsigned int timeOut)
{
	int		opts;
	unsigned int	destlen;
	long tmp = (long)timeOut;//����-W�澯
	tmp=~tmp;
	opts = fcntl(clientfd, F_GETFL);
	opts = (opts | O_NONBLOCK);
	fcntl(clientfd, F_SETFL, opts);

	switch( SOCKE_TCPWaitForRead( clientfd, 30))
	{
	case -1:

		return -1;

	case  0:

		return 0;
	
	case  1:
		
		destlen = sizeof( struct sockaddr_in);

		return recvfrom( clientfd, buf, size, 0, ( struct sockaddr *)&dest, &destlen);
	default:
		
		return -1;
	}
}

int init_udp(void)
{
	//int		socketfd;
	int ret;
	unsigned char		sockType = 1; 
	char		localIpAddr[50];
	int		localPort;
	char		remoteIpAddr[50];
	int		remotePort; 
	struct sockaddr_in local;
	
	sockType = 2;
	
	//����IP��ַ
	sprintf( localIpAddr,  "127.0.0.1");
//	sprintf(localIpAddr,"%d.%d.%d.%d",param.IP1[0],param.IP1[1],param.IP1[2],param.IP1[3]);
//printf("����IP��ַ��%d.%d.%d.%d, %s\n",	param.IP1[0],param.IP1[1],param.IP1[2],param.IP1[3],localIpAddr);
	localPort = 8000;
	
	//Զ��IP��ַ
	sprintf( remoteIpAddr, "224.6.6.6");
	remotePort = 40000;
		
	//UDP ��ʼ��
	ret = SOCKE_UdpInit( sockType, localIpAddr, localPort, remoteIpAddr, remotePort, &socketfd, &local, &dest);
  return ret;
	
}



void send_udp_state(void)
{
	char buf[2000];
	int frmlen  = 0;
	int i;
	int ret;
	//int iTemp;
	unsigned int uiTemp;
	COMDEF_HEAD udp_head;
	COMDEF_VAL  data_record;
	//unsigned int pps_errval_sec;
	//unsigned int pps_errval_nsec;
	//long long PPS_errval;
	char neg_flag;

	udp_head.from 	= htons(100);       	//���������Ķ�
 	udp_head.to   	= htons(0xffff);     	//���ķ����Ķ�
	udp_head.equipid= htonl(3);    		//װ������
	udp_head.comtype= htons(COMDEF_CMD_VAL);    		//���������
	
	memcpy(&buf[frmlen], &udp_head, sizeof(udp_head));
	frmlen += sizeof(udp_head);
	int tmlen = frmlen;
	buf[frmlen++] = 0;
	int count = 0;
	for(i=0;i<EVENT_COUNT;i++)
	{
		data_record.infoindex=htonl(param.event_param[i].index);
		data_record.datatype =param.event_param[i].type;
		data_record.value    =htonl(param.event_param[i].now_state);
		if(data_record.infoindex==7||data_record.infoindex==8)//����״̬
		{
			if(data_record.value==2)
				data_record.value = 0;
			else
				data_record.value = 1;
		}
		if(param.event_param[i].flag==0)
			data_record.value = 0;
		memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
		frmlen += sizeof(COMDEF_VAL);
		param.udp_state[data_record.infoindex] = data_record.value;
		count ++;
		//if(data_record.infoindex==18)
	    //myprintf("����UDP״̬��index=%d,type=%d,val=%x\n",data_record.infoindex,data_record.datatype,data_record.value);
	}
	
	data_record.infoindex=htonl(39);
	data_record.datatype =2;
	if(param.event_param[38].flag)
		data_record.value    = htonl(param.PTP1_priorty);
	else	
		data_record.value    =htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	param.udp_state[data_record.infoindex] = data_record.value;
	//myprintf("����UDP״̬��index=%d,type=%d,val=%x\n",data_record.infoindex,data_record.datatype,data_record.value);

	data_record.infoindex=htonl(40);
	data_record.datatype =2;
	if(param.event_param[39].flag)
		data_record.value = htonl(param.PTP2_priorty);
	else	
		data_record.value = htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	param.udp_state[data_record.infoindex] = data_record.value;
	//myprintf("����UDP״̬��index=%d,type=%d,val=%x\n",data_record.infoindex,data_record.datatype,data_record.value);
	
	data_record.infoindex=htonl(41);
	data_record.datatype =2;
	if(param.event_param[40].flag)
		data_record.value    =htonl(PTI.BDS.satellites>>4);
	else
		data_record.value    =htonl(0);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	param.udp_state[data_record.infoindex] = data_record.value;
//printf("����UDP״̬��index=%d,type=%d,val=%x\n",data_record.infoindex,data_record.datatype,data_record.value);
	
	data_record.infoindex=htonl(42);
	data_record.datatype =2;
	if(param.event_param[41].flag)
		data_record.value    =htonl(PTI.GPS.satellites>>4);
	else	
		data_record.value    =htonl(0);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	param.udp_state[data_record.infoindex] = data_record.value;
	//myprintf("����UDP״̬��index=%d,type=%d,val=%x\n",data_record.infoindex,data_record.datatype,data_record.value);
	
	data_record.infoindex=htonl(43);
	data_record.datatype =2;
	if(param.event_param[42].flag)
		data_record.value    = htonl(param.gps_priorty);
	else	
		data_record.value    =htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	param.udp_state[data_record.infoindex] = data_record.value;
	//myprintf("����UDP״̬��index=%d,type=%d,val=%x\n",data_record.infoindex,data_record.datatype,data_record.value);
	
	data_record.infoindex=htonl(44);
	data_record.datatype =2;
	if(param.event_param[43].flag)
		data_record.value    = htonl(param.bad_priorty);
	else	
		data_record.value    =htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	param.udp_state[data_record.infoindex] = data_record.value;
	//myprintf("����UDP״̬��index=%d,type=%d,val=%x\n",data_record.infoindex,data_record.datatype,data_record.value);
	
	data_record.infoindex=htonl(45);
	data_record.datatype =2;
	if(param.event_param[44].flag)
		data_record.value    = htonl(param.b1_priorty);
	else	
		data_record.value    =htonl(0);		
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	param.udp_state[data_record.infoindex] = data_record.value;
	//myprintf("����UDP״̬��index=%d,type=%d,val=%x\n",data_record.infoindex,data_record.datatype,data_record.value);
	
	data_record.infoindex=htonl(46);
	data_record.datatype =2;
	if(param.event_param[45].flag)
		data_record.value    = htonl(param.b2_priorty);
	else	
		data_record.value    =htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	param.udp_state[data_record.infoindex] = data_record.value;
	//myprintf("����UDP״̬��index=%d,type=%d,val=%x\n",data_record.infoindex,data_record.datatype,data_record.value);
	data_record.infoindex=htonl(61);
	data_record.datatype =2;
	if(param.event_param[60].flag)
		data_record.value    = htonl(myclog.Dev_TB);
	else	
		data_record.value    =htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	//myprintf("����UDP״̬��index=%d,type=%d,val=%x\n",data_record.infoindex,data_record.datatype,data_record.value);
	
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("����UDP״̬��index=%d,type=%d,val=%x\n",data_record.infoindex,data_record.datatype,data_record.value);
	
	for(i=0;i<14;i++)																//1--14���״̬������
	{
		data_record.infoindex=htonl(2*i+OFF_E_BoardState+1);
		data_record.datatype =2;
		if(bard_state_udp[i])	//�忨����
			data_record.value    = htonl(0);
		else
			data_record.value    = htonl(1);	
		memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
		frmlen += sizeof(COMDEF_VAL);
		count ++;
//printf("��%d���״̬��%x, ",i+1,data_record.value);		
		
		data_record.infoindex=htonl(2*i+OFF_E_BoardState+2);
		data_record.datatype =2;
		data_record.value    = htonl(bard_state_udp[i]);	//�忨����
		memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
		frmlen += sizeof(COMDEF_VAL);
		count ++;
//printf("��ţ�%x\n",data_record.value);	
	}
	
	data_record.infoindex=htonl(90);								//�������״̬
	data_record.datatype =2;
	if(myclog.BDS_board_state)
		data_record.value    = htonl(1);
	else
		data_record.value    = htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("�������״̬��%x,  ",data_record.value);
	
	data_record.infoindex=htonl(91);								//�����������
	data_record.datatype =2;
	if(myclog.BDS_board_state)
		data_record.value    = htonl(0);
	else
		data_record.value    = htonl(0x0103);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("��ţ�%x\n",data_record.value);	
	
	
	
	data_record.infoindex=htonl(92);								//GPS���״̬
	data_record.datatype =2;
	if(myclog.GPS_board_state)
		data_record.value    = htonl(1);
	else
		data_record.value    = htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("GPS���״̬��%x,  ",data_record.value);	
	
	data_record.infoindex=htonl(93);								//GPS�������
	data_record.datatype =2;
	if(myclog.GPS_board_state)
		data_record.value    = htonl(0);
	else
		data_record.value    = htonl(0x0107);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("��ţ�%x\n",data_record.value);
	
	data_record.infoindex=htonl(94);								//BDS��������ֵ
	data_record.datatype =2;
	uiTemp = 256*(256*(256*file_param.Sysset.Mod_value.BDSoffsetVal[0]+file_param.Sysset.Mod_value.BDSoffsetVal[1])+file_param.Sysset.Mod_value.BDSoffsetVal[2])+file_param.Sysset.Mod_value.BDSoffsetVal[3];
	data_record.value    = htonl(uiTemp);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("BDS����ֵ��%x\n",data_record.value);	
	
	data_record.infoindex=htonl(95);								//GPS��������ֵ
	data_record.datatype =2;
	uiTemp = 256*(256*(256*file_param.Sysset.Mod_value.GPSoffsetVal[0]+file_param.Sysset.Mod_value.GPSoffsetVal[1])+file_param.Sysset.Mod_value.GPSoffsetVal[2])+file_param.Sysset.Mod_value.GPSoffsetVal[3];
	data_record.value    = htonl(uiTemp);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("GPS����ֵ��%x\n",data_record.value);	
	
	data_record.infoindex=htonl(96);								//B1��������ֵ
	data_record.datatype =2;
	uiTemp = 256*(256*(256*file_param.Sysset.Mod_value.B1offsetVal[0]+file_param.Sysset.Mod_value.B1offsetVal[1])+file_param.Sysset.Mod_value.B1offsetVal[2])+file_param.Sysset.Mod_value.B1offsetVal[3];
	data_record.value    = htonl(uiTemp);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("B1����ֵ��%x\n",data_record.value);	
	
	data_record.infoindex=htonl(97);								//B2��������ֵ
	data_record.datatype =2;
	uiTemp = 256*(256*(256*file_param.Sysset.Mod_value.B2offsetVal[0]+file_param.Sysset.Mod_value.B2offsetVal[1])+file_param.Sysset.Mod_value.B2offsetVal[2])+file_param.Sysset.Mod_value.B2offsetVal[3];
	data_record.value    = htonl(uiTemp);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("B2����ֵ��%x\n",data_record.value);
	
	data_record.infoindex=htonl(98);								//PTP1��������ֵ
	data_record.datatype =2;
	uiTemp = 256*(256*(256*file_param.Sysset.Mod_value.PTP1offsetVal[0]+file_param.Sysset.Mod_value.PTP1offsetVal[1])+file_param.Sysset.Mod_value.PTP1offsetVal[2])+file_param.Sysset.Mod_value.PTP1offsetVal[3];
	data_record.value    = htonl(uiTemp);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("PTP1����ֵ��%x\n",data_record.value);	
	
	data_record.infoindex=htonl(99);								//PTP2��������ֵ
	data_record.datatype =2;
	uiTemp = 256*(256*(256*file_param.Sysset.Mod_value.PTP2offsetVal[0]+file_param.Sysset.Mod_value.PTP2offsetVal[1])+file_param.Sysset.Mod_value.PTP2offsetVal[2])+file_param.Sysset.Mod_value.PTP2offsetVal[3];
	data_record.value    = htonl(uiTemp);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("PTP2����ֵ��%x\n",data_record.value);	
	
	data_record.infoindex=htonl(100);								//ʱ���������ֵ
	data_record.datatype =2;
	uiTemp = 256*(256*(256*file_param.Sysset.Mod_value.ClockoffsetVal[0]+file_param.Sysset.Mod_value.ClockoffsetVal[1])+file_param.Sysset.Mod_value.ClockoffsetVal[2])+file_param.Sysset.Mod_value.ClockoffsetVal[3];
	data_record.value    = htonl(uiTemp);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("clock����ֵ��%x\n",data_record.value);	
	
	data_record.infoindex=htonl(101);								//BDS�Ӳ�ֵ
	data_record.datatype =2;
	if(param.event_param[100].flag)
		data_record.value    = htonl(clock_offset.BDS.sec);
	else
		data_record.value    = htonl(0);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	
	data_record.infoindex=htonl(102);								//BDS�Ӳ�ֵ
	data_record.datatype =2;
	if(param.event_param[101].flag)
		data_record.value    = htonl(clock_offset.BDS.nsec);
	else
		data_record.value    = htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//	printf("BDS�Ӳ%x.%x \n",clock_offset.BDS.sec,clock_offset.BDS.nsec);	

	data_record.infoindex=htonl(103);								//GPS�Ӳ�ֵ
	data_record.datatype =2;
	if(param.event_param[102].flag)
		data_record.value    = htonl(clock_offset.GPS.sec);
	else
		data_record.value    = htonl(0);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	
	data_record.infoindex=htonl(104);								//GPS�Ӳ�ֵ
	data_record.datatype =2;
	if(param.event_param[103].flag)
		data_record.value    = htonl(clock_offset.GPS.nsec);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//	printf("GPS�Ӳ%x.%x\n",clock_offset.GPS.sec,clock_offset.GPS.nsec);	
	
	data_record.infoindex=htonl(105);								//b1�Ӳ�ֵ
	data_record.datatype =2;
	if(param.event_param[104].flag)
		data_record.value    = htonl(clock_offset.B1.sec);
	else
		data_record.value    = htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	
	data_record.infoindex=htonl(106);								//b1�Ӳ�ֵ
	data_record.datatype =2;
	if(param.event_param[105].flag)
		data_record.value    = htonl(clock_offset.B1.nsec);
	else
		data_record.value    = htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	//myprintf("B1�Ӳ%x.%x \n",clock_offset.B1.sec,clock_offset.B1.nsec);	
	
	data_record.infoindex=htonl(107);								//b2�Ӳ�ֵ
	data_record.datatype =2;
	if(param.event_param[106].flag)
		data_record.value    = htonl(clock_offset.B2.sec);
	else
		data_record.value    = htonl(0);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	
	data_record.infoindex=htonl(108);								//b2�Ӳ�ֵ
	data_record.datatype =2;
	if(param.event_param[107].flag)
		data_record.value    = htonl(clock_offset.B2.nsec);
	else
		data_record.value    = htonl(0);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	//myprintf("B2�Ӳ%x.%x \n",clock_offset.B2.sec,clock_offset.B2.nsec);		
	
	data_record.infoindex=htonl(109);								//ptp1�Ӳ�ֵ
	data_record.datatype =2;
	if(param.event_param[108].flag)
		data_record.value    = htonl(clock_offset.PTP1.sec);
	else
		data_record.value    = htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	
	data_record.infoindex=htonl(110);								//ptp1�Ӳ�ֵ
	data_record.datatype =2;
	if(param.event_param[109].flag)
		data_record.value    = htonl(clock_offset.PTP1.nsec);
	else
		data_record.value    = htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	//printf("PTP1�Ӳ%x.%x\n",clock_offset.PTP1.sec,clock_offset.PTP1.nsec);	

	data_record.infoindex=htonl(111);								//ptp2�Ӳ�ֵ
	data_record.datatype =2;
	if(param.event_param[110].flag)
		data_record.value    = htonl(clock_offset.PTP2.sec);
	else
		data_record.value    = htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	
	data_record.infoindex=htonl(112);								//ptp2�Ӳ�ֵ
	data_record.datatype =2;
	if(param.event_param[111].flag)
		data_record.value    = htonl(clock_offset.PTP2.nsec);
	else
		data_record.value    = htonl(0);	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	//printf("PTP2�Ӳ%x.%x \n",	clock_offset.PTP2.sec,clock_offset.PTP2.nsec);	
	
	data_record.infoindex=htonl(113);								//����Ƶ��1
	data_record.datatype =2;
	data_record.value    = htonl(myclog.freque1);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	
	data_record.infoindex=htonl(114);								//����Ƶ��2
	data_record.datatype =2;
	data_record.value    = htonl(myclog.freque2);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("����Ƶ�ʣ�%x,%x \n",	myclog.freque1,myclog.freque2);	
	
	unsigned char	longitude[6],latitude[6],high[6];
	unsigned int  uiLlngitude,uiLatitude,uiHigh;
	if(myclog.BDS_source_state==0)
	{
		for(i=0;i<6;i++)
		{
			longitude[i] = FTP.BDS.longitude[i];
			latitude[i]  = FTP.BDS.latitude[i];
			high[i] 		 = FTP.BDS.high[i];
		}
	}	
	else if(myclog.GPS_source_state==0)
	{
		for(i=0;i<6;i++)
		{
			longitude[i] = FTP.GPS.longitude[i];
			latitude[i]  = FTP.GPS.latitude[i];
			high[i] 		 = FTP.GPS.high[i];
		}
	}
	else	
	{
		for(i=0;i<6;i++)
		{
			longitude[i] = 0;
			latitude[i]  = 0;
			high[i] 	 = 0;
		}
	}

//	printf("���ȣ�%02x %02x %02x %02x %02x %02x\n",
//					longitude[0],
//					longitude[1],
//					longitude[2],
//					longitude[3],
//					longitude[4],
//					longitude[5]);
//
//printf("γ�ȣ�%02x %02x %02x %02x %02x %02x\n",
//					latitude[0],
//					latitude[1],
//					latitude[2],
//					latitude[3],
//					latitude[4],
//					latitude[5]);
//printf("�߶ȣ�%02x %02x %02x %02x %02x %02x\n",
//					high[0],
//					high[1],
//					high[2],
//					high[3],
//					high[4],
//					high[5]);					
//						
//	
	
	unsigned char uc1,uc2,uc3,uc4;
	uc1 = 100*(longitude[0]>>4)+10*(longitude[0]&0x0f)+(longitude[1]>>4);
	uc2 = 10*(longitude[1]&0x0f)+(longitude[2]>>4);
	uc3 = 10*(longitude[3]>>4)+(longitude[3]&0x0f);
	uc4 = 10*(longitude[4]>>4)+(longitude[4]&0x0f);
	uiLlngitude = ((uc1*256+uc2)*256+uc3)*256+uc4;
	
	uc1 = 10*(latitude[0]>>4)+(latitude[0]&0x0f);
	uc2 = 10*(latitude[1]>>4)+(latitude[1]&0x0f);
	uc3 = 10*(latitude[2]&0x0f)+(latitude[3]>>4);
	uc4 = 10*(latitude[3]&0x0f)+(latitude[4]>>4);
	uiLatitude = ((uc1*256+uc2)*256+uc3)*256+uc4;
	
	uiTemp=0;
	char flag=0;
	neg_flag=0;
	for(i=0;i<12;i++)
	{
		if((i&0x01)==0)
			uc1 = high[i/2]>>4;
		else
			uc1 = high[i/2]&0x0f;
		if(flag==0)
		{
			if(uc1==0x0b)
			{
				neg_flag=1;
				uiTemp=0;
			}	
			else if(uc1==0x0e)
				flag=1;
			//else if((uc1>=0)&&(uc1<=9))
			else if(uc1<=9)
			{
				uiTemp = 10*uiTemp +uc1;	
			}	
			else
			{
				//printf("�߶Ƚ���(����)���ݴ���\n");
				uiTemp=0;
				uc3=0;
				break;
			}		
		}	
		else
		{
			//if(uc1>=0 && uc1<=9)
			if(uc1<=9)
			{
				uc3=10*uc1;
				break;
			}
			else
			{
				//printf("�߶Ƚ�����С�������ݴ���\n");
				uc3=0;
				break;
			}
		}	
	}
	if(neg_flag)
		uiTemp |= 0x8000;
	uc1 = uiTemp>>8;
	uc2 = uiTemp&0xff;
	uc4 = 0;	
	uiHigh = ((uc1*256+uc2)*256+uc3)*256+uc4;
	
	
	
	data_record.infoindex=htonl(115);								//��վ���ھ���
	data_record.datatype =2;
	data_record.value    = htonl(uiLlngitude);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	
	
	data_record.infoindex=htonl(116);								//��վ����γ��
	data_record.datatype =2;
	data_record.value    = htonl(uiLatitude);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
	
	data_record.infoindex=htonl(117);								//��վ���ڸ߶�
	data_record.datatype =2;
	data_record.value    = htonl(uiHigh);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("����=%x,γ��=%x���߶�=%x \n",	uiLlngitude,uiLatitude,uiHigh);	

	data_record.infoindex=htonl(118);								//�ڳ�ʱ��
	data_record.datatype =2;
	data_record.value    = htonl(pps_second.locate_total_sec);
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("�ڳ�ʱ��=%x\n",data_record.value);	

	data_record.infoindex=htonl(119);								//BDSԴʱ��
	data_record.datatype =2;
	if((FTP.BDS.source_syn_state&SS_SYNC)==0)
		data_record.value    = htonl(pps_second.bds_total_sec+1);
	else
		data_record.value = 0;	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("BDSԴʱ��=%x\n",data_record.value);

	data_record.infoindex=htonl(120);								//BDSʧ��ʱ��
	data_record.datatype =2;
	if(lose_lock.bds_flag && myclog.Init_state==0 && param.event_param[119].flag)
		data_record.value = htonl(pps_second.locate_total_sec-lose_lock.bds_sec);//����ʧ��ʱ��
	else
		data_record.value = 0;	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("BDSʧ��ʱ��=%x\n",data_record.value);

	data_record.infoindex=htonl(121);								//GPSԴʱ��
	data_record.datatype =2;
	if((FTP.GPS.source_syn_state&SS_SYNC)==0)
		data_record.value    = htonl(pps_second.gps_total_sec+1);
	else
		data_record.value = 0;	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("GPSԴʱ��=%x\n",data_record.value);

	data_record.infoindex=htonl(122);								//GPSʧ��ʱ��
	data_record.datatype =2;
	if(lose_lock.gps_flag && myclog.Init_state==0 && param.event_param[121].flag)
		data_record.value    = htonl(pps_second.locate_total_sec-lose_lock.gps_sec);
	else
		data_record.value = 0;	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("GPSʧ��ʱ��=%x\n",data_record.value);

	data_record.infoindex=htonl(123);								//B1Դʱ��
	data_record.datatype =2;
	if((FTP.IRIG_B1.source_syn_state&SS_SYNC)==0)
		data_record.value    = htonl(pps_second.b1_total_sec+1);
	else
		data_record.value = 0;	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("B1Դʱ��=%x\n",data_record.value);

	data_record.infoindex=htonl(124);								//B1ʧ��ʱ��
	data_record.datatype =2;
	if(lose_lock.b1_flag && myclog.Init_state==0 && param.event_param[123].flag)
		data_record.value    = htonl(pps_second.locate_total_sec-lose_lock.b1_sec);
	else
		data_record.value = 0;	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("B1ʧ��ʱ��=%x\n",data_record.value);

	data_record.infoindex=htonl(125);								//B2Դʱ��
	data_record.datatype =2;
	if((FTP.IRIG_B2.source_syn_state&SS_SYNC)==0)
		data_record.value    = htonl(pps_second.b2_total_sec+1);
	else
		data_record.value = 0;	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("B2Դʱ��=%x\n",data_record.value);

	data_record.infoindex=htonl(126);								//B2ʧ��ʱ��
	data_record.datatype =2;
	if(lose_lock.b2_flag && myclog.Init_state==0 && param.event_param[125].flag)
		data_record.value    = htonl(pps_second.locate_total_sec-lose_lock.b2_sec);
	else
		data_record.value = 0;	
	memcpy(&buf[frmlen], &data_record, sizeof(COMDEF_VAL));
	frmlen += sizeof(COMDEF_VAL);
	count ++;
//printf("B2ʧ��ʱ��=%x\n",data_record.value);

	
	buf[tmlen]= count;
	ret=SOCKE_UDPWrite(socketfd, buf, frmlen, dest, 1000);
	if(ret!=frmlen)
	{
		printf("UDPʵʱ���ݷ���ʧ�ܣ�send_byte=%d,ret_val=%d\n",frmlen,ret);
	}
//	printf("����UDP���ģ���¼���� %d,���ȣ�%d,  ������������%d,GPS��������%d,	�������ȼ���%d, GPS���ȼ���%d,B1���ȼ���%d,B2���ȼ���%d\n",
//					count,frmlen,
//					PTI.BDS.satellites>>4,PTI.GPS.satellites>>4,
//					param.bad_priorty,param.gps_priorty,param.b1_priorty,param.b2_priorty);




}

void send_udp_event(void)
{
	LOG_MESSAGE log_buf;//д������־buf
	char buf[2000];
	char time_buf[30];
	int frmlen  = 0;
	int i;
	int ret;
	COMDEF_HEAD 	udp_head;
	COMDEF_EVENT	event_record;
	if(udp_event_count==0) return;
	udp_head.from 	= htons(100);       			//���������Ķ�
    udp_head.to   	= htons(0xffff);     			//���ķ����Ķ�
	udp_head.equipid= htonl(3);    					//װ������
	udp_head.comtype= htons(COMDEF_CMD_EVENT); 		//���������
	
	memcpy(&buf[frmlen], &udp_head, sizeof(udp_head));
	frmlen += sizeof(udp_head);
	buf[frmlen++] = udp_event_count;
	for(i=0;i<udp_event_count;i++)
	{
		event_record.infoindex = htonl(udp_event[i].index);
				event_record.datatype  = udp_event[i].type;
		event_record.value     = htonl(udp_event[i].val);
		if(event_record.infoindex==7 || event_record.infoindex==8)
		{
			if(event_record.value==2) 
				event_record.value=htonl(0);
			else
				event_record.value=htonl(1);	
		}
		sprintf(time_buf,"%04d-%02d-%02d %02d:%02d:%02d.%06d",
						udp_event[i].time.year,udp_event[i].time.mon,udp_event[i].time.day,
						udp_event[i].time.hour,udp_event[i].time.min,(int)udp_event[i].time.sec,(int)udp_event[i].time.usec);
						//�޸ļ�¼��2017.11.23
						//udp_event[i].time.hour,udp_event[i].time.min,udp_event[i].time.sec,udp_event[i].time.usec);
		memset(event_record.time, 0x00, sizeof(event_record.time));
		strcpy(event_record.time,time_buf);
		
		memcpy(&buf[frmlen], &event_record, sizeof(COMDEF_EVENT));
		frmlen += sizeof(COMDEF_EVENT);	
		if(__MY_PROG_UDP){
			sprintf(log_buf.name,"  2Event_UDP[%03d]: val=%08x",udp_event[i].index,event_record.value);
			add_my_log(log_buf);	
		}	
	}
	ret=SOCKE_UDPWrite(socketfd, buf, frmlen, dest, 1000);
	udp_event_count=0;
	if(ret!=frmlen)
	{
		sprintf(log_buf.name,"UDP�¼�����ʧ�ܣ�send_byte=%d,ret_val=%d\n",frmlen,ret);
		add_my_log(log_buf);
	}	
//	printf("�����¼���len=%d,ret=%d\n\n\n",frmlen,ret);
}

void add_event_udp(int id,Time serialTime)
{
	udp_event[udp_event_count].index = id+1;
	udp_event[udp_event_count].type  = param.event_param[id].type;
	udp_event[udp_event_count].val   = param.event_param[id].now_state;
	//myprintf("now(%d)id=%d\n",udp_event[udp_event_count].val,id+1);
	//61-
	if((id>=OFF_E_BoardState && id<(OFF_E_BoardState+14*2))&&(id%2)){
		if(param.event_param[id].now_state==0x1f){
			udp_event[udp_event_count].val   = 1;
		}
		else{
			udp_event[udp_event_count].val   = 0;
		}
	}
	//myprintf("now2(%d)id=%d\n",udp_event[udp_event_count].val,id+1);
//	udp_event[udp_event_count].old_val   = param.event_param[id].old_val;
	udp_event[udp_event_count].time.year  = serialTime.y.year;
	udp_event[udp_event_count].time.mon  = serialTime.mo.month;
	udp_event[udp_event_count].time.day  = serialTime.d.day;
	udp_event[udp_event_count].time.hour = serialTime.h.hour;
	udp_event[udp_event_count].time.min  = serialTime.m.min;
	udp_event[udp_event_count].time.sec  = serialTime.s.second;
	udp_event[udp_event_count].time.usec = 0;
	udp_event_count++;
}
