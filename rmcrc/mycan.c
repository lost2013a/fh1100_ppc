#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <error.h>
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/prctl.h>
#include "bsp.h"

extern sem_t sem_can_dy_send;

void can_send(S_CanFrame *pframe, int count)
{
	int num,err_n,ret;
	for(num=0,err_n=0;num<count;)
	{
		pframe[num].can_id = pframe[num].can_id | CAN_EFF_FLAG;
		if(param.print_can)
		{
			int i = 0;
			printf("(%d)  %#x  [%d]  ", num, pframe[num].can_id, pframe[num].can_dlc);
			for (i = 0; i < pframe[num].can_dlc; i++)
				printf("%#x ", pframe[num].data[i]);
			printf("\n");
    	}
		ret = write(socket_fd, &pframe[num], sizeof(pframe[num]));
		if(ret != sizeof(pframe[num]))
		{
			if(err_n>SEND_Retry){
				printf("Can Send Error\n");
				break;
			}
			else if(err_n>SEND_Retry-3){
				if(err_n==SEND_Retry-2)
					printf("Can Send Retry(%d) @Frame[%02d]\n",err_n,num);
				usleep(1000);	//�����߳�
			}
			err_n++;
		}
		else{
			num++;
			err_n=0;
		}
	}		
}

int can_recv(S_CanFrame *pframe,unsigned char count)
{
	int i,j=0,rec=0,err=0;
	count=count%6;//����Խ��
	for(i=0;i<count;){
		//rec+=read(socket_fd, &pframe[i], CAN_FRAME_LENTH);
		j=recv(socket_fd,&pframe[i],CAN_FRAME_LENTH,MSG_DONTWAIT);
		if(j>0){
			rec+=j;
			//myprintf("can1 delay :%d\n",err);
			if(rec%CAN_FRAME_LENTH==0)	
				i++;
			err=0;
		}
		else{
			err++;
			usleep(2);
			if(err>20){
				myprintf("can1 err:%d\n",err);
				i++;
				err=0;
			}
		}
	}
	if(rec%CAN_FRAME_LENTH)	
		rec=0;
	else
		rec/=CAN_FRAME_LENTH;
	return rec;
}




void handle_canrec_netset(NetDev_Type* Net_set,S_CanFrame* recvframe,int port)
{	
	LOG_MESSAGE log_buf;//д������־buf
	int i,ret;
	NetDev_Type last_set;
	last_set = *Net_set;
	for(i = 0; i < 4; i++)
		Net_set->ip[i] = recvframe[0].data[i];	
	for(i = 4; i < 8; i++)
		Net_set->mask[i-4] = recvframe[0].data[i];
	for(i = 0; i < 4; i++)
		Net_set->wg[i] = recvframe[1].data[i];
	for(i = 4; i < 8; i++)
		Net_set->remoteip[i-4] = recvframe[1].data[i];
	for(i = 0; i < 2; i++)
		Net_set->localport[i] = recvframe[2].data[i];
	for(i = 2; i < 4; i++)
		Net_set->remoteport[i-2] = recvframe[2].data[i];
	Net_set->ethernet_mode = recvframe[2].data[4];
	Net_set->spreadmode = recvframe[2].data[5];
	Net_set->spreadgaps = recvframe[2].data[6];
	for(i = 0; i < 6; i++)
		Net_set->mac[i] = recvframe[3].data[i];
	Net_set->master_slaverflag = recvframe[3].data[6];
	Net_set->ptpd.DOMAINNUMBER = recvframe[3].data[7];
	Net_set->ptpd.ANNOUNCE_INTERVAL = recvframe[4].data[0];
	Net_set->ptpd.ANNOUNCE_RECEIPT_TIMEOUT = recvframe[4].data[1];
	Net_set->ptpd.SYNC_INTERVAL = recvframe[4].data[2];
	Net_set->ptpd.PDELAYREQ_INTERVAL = recvframe[4].data[3];
	Net_set->ptpd.DELAYREQ_INTERVAL = recvframe[4].data[4];
	Net_set->ptpd.SLAVE_PRIORITY1 = recvframe[4].data[5];
	Net_set->ptpd.SLAVE_PRIORITY2 = recvframe[4].data[6];							
	ret=SetIfAddr(bsp_net[port], Net_set->ip, Net_set->mask,Net_set->wg);
	if(ret!=0){
		
		*Net_set = last_set;
		if(ret==1){	//����·�ɲ���ȷ����ʱIP�Ѿ������ã���Ҫ���赽�ϴε�״̬
			SetIfAddr(bsp_net[port], Net_set->ip, Net_set->mask,Net_set->wg);
			sprintf(log_buf.name,"����[%d]·�ɱ���ȷ",port+1);
		}
		else{
			sprintf(log_buf.name,"����[%d]����ʧ��,errno=%02d",port+1,ret);
		}
		add_my_log(log_buf);
	}
	else{
		sprintf(log_buf.name,"����[%d]�����޸�,IP��ַ:%d.%d.%d.%d",
			port+1,Net_set->ip[0],Net_set->ip[1],Net_set->ip[2],Net_set->ip[3]);
		add_my_log(log_buf);
	}
}



void *Pthread_3_canrec(void *arg)
{
	LOG_MESSAGE log_buf;		//д������־buf
	S_CanFrame  recvframe[6];	//��������1�������ڶ�������
	S_CanFrame  cfsendframe[16];
	int i,ret;
	unsigned int num,count;
	fd_set rfds;
	long tmp = (long)arg;//����-W�澯
	tmp=~tmp;
	FD_ZERO(&rfds); 			/*��set����ʹ�����в����κ�fd*/
	FD_SET(socket_fd, &rfds);	/*��fd����set����*/
	prctl(PR_SET_NAME, (unsigned long)"rmcrc_3canrec"); 
	
	for(;;)	//˫ѭ��,�����������˳�����;
	{	
	while(1)
	{	
		ret=select(socket_fd+1, &rfds, NULL, NULL, NULL);//��·�������׽���,��    �������������׽ӿڵ���Ŀ
		if(ret==-1){
			perror("select");
			continue;
		}
		if(FD_ISSET(socket_fd, &rfds))		//������������	rfds
		{	
			count=can_recv(&recvframe[0],1);
			if (count<1){
				continue;
			}
			int frame_type = (recvframe[0].can_id&0xff00)>>8;	//��������
			int frame_RTR  = recvframe[0].can_id&CAN_RTR_FLAG;  //���İ�����֡����Զ��֡
			if(frame_RTR==0)	 								//���ò���(����֡)
			{	
				int frame_id = recvframe[0].can_id & 0x0f;	  	//���İ���֡��
				if(frame_id){								  	//����֡ͷ(0)
					can_recv(&recvframe[0],5);//����ʣ��Ĳ�����
					myprintf("CAN����֡δ�յ�֡ͷ:(%d)֡,[%d.%d]\n",frame_id,sys_time.min,sys_time.sec);
					continue;
				}
				int frame_nr = (recvframe[0].can_id&0xf0)>>4;	//���İ�����֡��(��-1)
				if(frame_nr>0){									//���Ļ�û�н�������
					count+=can_recv(&recvframe[1],frame_nr);	//ע���֡1��ʼ������ȡ
					int order1,order2;
					for(i=0;i<frame_nr;i++){
						order1=recvframe[i+1].can_id;
						order2=recvframe[i].can_id;
						//myprintf("i:%d,oredr1:%x,oredr2:%x\n",i,order1,order2);
						if((order1-order2)<1)//������˳��
						{	
							myprintf("--CAN����֡not in order\n");
							count=0;							 //
						}	
					}
					if(count<frame_nr)							 //û���յ��㹻��֡
						continue;
				}
				myprintf("--CAN����֡�յ�:(%d/%d)֡,type:<%02d>,[%d.%d]\n",count,frame_nr+1,frame_type,sys_time.min,sys_time.sec);
				switch(frame_type)
				{
					case NETSET:
						num = recvframe[0].can_id >> 16 & 0xf ;//ȡʮ�����Ƶ�5λ��ֵ���������ƶ�12λ���������ƶ�28λΪ���ں�
						switch(num)
						{
							case 0x00:	//����1	
								handle_canrec_netset(&file_param.Net_set1,&recvframe[0],0);
								break;
							case 0x01:	//����2	
								handle_canrec_netset(&file_param.Net_set2,&recvframe[0],1);
								break;
							case 0x02:	//����3	
								handle_canrec_netset(&file_param.Net_set3,&recvframe[0],2);
								break;
							case 0x03:	//����4	
								handle_canrec_netset(&file_param.Net_set4,&recvframe[0],3);
								break;
							case 0x04:	//����5	
								handle_canrec_netset(&file_param.Net_set5,&recvframe[0],4);
								break;
							case 0x05://����6
								handle_canrec_netset(&file_param.Net_set6,&recvframe[0],5);
								break;
						}
						param.read_param = 1;
						break;
					case DELAYOFFSET:	
						for(i = 0; i < 4; i++)
						{
							file_param.Sysset.Mod_value.BDSoffsetVal[i] = recvframe[0].data[i];
						}	
						for(i = 4; i < 8; i++)	
						{
							file_param.Sysset.Mod_value.GPSoffsetVal[i-4] = recvframe[0].data[i];
						}	
						for(i = 0; i < 4; i++)
						{
							file_param.Sysset.Mod_value.B1offsetVal[i] = recvframe[1].data[i];
						}	
						for(i = 4; i < 8; i++)	
						{
							file_param.Sysset.Mod_value.B2offsetVal[i-4] = recvframe[1].data[i];
						}
						for(i = 0; i < 4; i++)
						{
							file_param.Sysset.Mod_value.PTP1offsetVal[i] = recvframe[2].data[i]; 
						}	
						for(i = 4; i < 8; i++)
						{
							file_param.Sysset.Mod_value.PTP2offsetVal[i-4] = recvframe[2].data[i];
						}	
						for(i = 0; i < 4; i++)
						{
							file_param.Sysset.Mod_value.ClockoffsetVal[i] = recvframe[3].data[i]; 
						}	
						for(i = 4; i < 8; i++)	
						{
							file_param.Sysset.Mod_value.PulseoffsetVal[i-4] = recvframe[3].data[i];
						}	
						for(i = 0; i < 4; i++)
						{
							file_param.Sysset.Mod_value.SmesoffsetVal[i] = recvframe[4].data[i];
						}	
						for(i = 4; i < 8; i++)
						{
							file_param.Sysset.Mod_value.RIGBoffsetVal[i-4] = recvframe[4].data[i];
						}	
						param.read_param = 1;
						break;
					case MASTER_SLAVERSET:	
						file_param.Sysset.Master_Slaver= recvframe[0].data[0];
						file_param.Sysset.Source_Mode= recvframe[0].data[1];
						param.read_param = 1;
						break;
					case UARTSET:	
						file_param.Sysset.uart1.Uart_baud= recvframe[0].data[0];
						file_param.Sysset.uart1.Uart_protocol= recvframe[0].data[1];
						file_param.Sysset.uart2.Uart_baud= recvframe[0].data[2];
						file_param.Sysset.uart2.Uart_protocol= recvframe[0].data[3];
						file_param.Sysset.IRIG_B_verify = recvframe[0].data[4];	
						param.read_param = 1;
						break;
					case PRIORIRITYSET:	
						file_param.Sysset.Priority[0] = recvframe[0].data[0];
						file_param.Sysset.Priority[1] = recvframe[0].data[1];
						file_param.Sysset.Priority[2] = recvframe[0].data[2];
						param.read_param = 1;	
						break;
					case PPXSET:	
						file_param.Sysset.PPXtype = recvframe[0].data[0];	
						param.read_param = 1;
						break;
					case ZONEINFO:	
						file_param.Sysset.Zoneinfo =  recvframe[0].data[0];
						file_param.Sysset.Time_offset_05hour = recvframe[0].data[1];	
						param.read_param = 1;
						break;
					case AMSET:	
						file_param.Sysset.AM_proportion = recvframe[0].data[0];	
						param.read_param = 1;
						break;
					case TIMESET:	
						file_param.Sysset.Source_outputflag = recvframe[0].data[0];
						file_param.Sysset.setSystime.year[1] = recvframe[0].data[1];
						file_param.Sysset.setSystime.year[0] = recvframe[0].data[2];
						file_param.Sysset.setSystime.month = recvframe[0].data[3];
						file_param.Sysset.setSystime.day = recvframe[0].data[4];
						file_param.Sysset.setSystime.hour = recvframe[0].data[5];
						file_param.Sysset.setSystime.min = recvframe[0].data[6];
						file_param.Sysset.setSystime.second = recvframe[0].data[7];
						param.read_param = 1;
						break;
					case BD3DSET:
						for(i = 0; i < 6; i++)
							file_param.Bd3Dset.Longitude[i] = recvframe[0].data[i];
						for(i = 0; i < 6; i++)
							file_param.Bd3Dset.Latitude[i] = recvframe[1].data[i];
						for(i = 0; i < 6; i++)
							file_param.Bd3Dset.High[i] = recvframe[2].data[i];
						param.read_param = 1;
						break;
					case SUMMERTIME:
						file_param.summerTime.flag= recvframe[0].data[0];
						int year = recvframe[0].data[1]+256*recvframe[0].data[2]; 
						file_param.summerTime.start.year = year%2000;
						file_param.summerTime.start.month= recvframe[0].data[3];
						file_param.summerTime.start.day= recvframe[0].data[4];
						file_param.summerTime.start.hour= recvframe[0].data[5];
						file_param.summerTime.start.min= recvframe[0].data[6];
						file_param.summerTime.start.second= recvframe[0].data[7];
						year = recvframe[1].data[1]+256*recvframe[1].data[2];
						file_param.summerTime.end.year = year%2000;
						file_param.summerTime.end.month= recvframe[1].data[3];
						file_param.summerTime.end.day= recvframe[1].data[4];
						file_param.summerTime.end.hour= recvframe[1].data[5];
						file_param.summerTime.end.min= recvframe[1].data[6];
						file_param.summerTime.end.second= recvframe[1].data[7];
						param.read_param = 1;
						break;
					case USER_LOG:	
						param.event_param[59].now_state=recvframe[0].data[0];
						event_from_lcd(59);
						break;
					case CTRL_BYTE:	
						file_param.control.global_control1 = recvframe[0].data[0];
						file_param.control.global_control2 = recvframe[0].data[1];
						file_param.control.bds_control  = recvframe[0].data[2];
						file_param.control.gps_control  = recvframe[0].data[3];
						file_param.control.b1_control   = recvframe[0].data[4];
						file_param.control.b2_control   = recvframe[0].data[5];
						file_param.control.ptp1_control = recvframe[0].data[6];
						file_param.control.ptp2_control = recvframe[0].data[7];
						file_param.control.bak_control1    = recvframe[1].data[0];
						file_param.control.bak_control2    = recvframe[1].data[1];
						file_param.control.fault_control   = recvframe[1].data[2];
						file_param.control.warning_control = recvframe[1].data[3];
						file_param.control.bak_control3    = recvframe[1].data[4];
						file_param.control.bak_control4    = recvframe[1].data[5];
						file_param.control.bak_control5    = recvframe[1].data[6];
						file_param.control.bak_control6    = recvframe[1].data[7];
						param.read_param = 1;	
						param.initial=0;	//��ʼ��δ���
						break;
					default:
						can_recv(&recvframe[0],5);//����ʣ��Ĳ�����
						myprintf("����δ֪���Եı��ģ�����\n");
						break;
					}
			}
			else{ 	//Զ��֡��
				//myprintf("--CANԶ��֡�յ�:(%d)֡,type:<%02d>,[%d.%d]\n",count,frame_type,sys_time.min,sys_time.sec);
				switch(frame_type)
				{
					case NETSET:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*5);
						num = recvframe[0].can_id >> 16 & 0xf; 
						switch(num)
						{
							case 0x00:	//����1	
								Package_Net1set_frame(cfsendframe, file_param);
								can_send(cfsendframe,5);
								break;
							case 0x01:	//����2				
								Package_Net2set_frame(cfsendframe, file_param);
								can_send(cfsendframe,5);
								break;
							case 0x02:	//����3	
								Package_Net3set_frame(cfsendframe, file_param);
								can_send(cfsendframe,5);
								break;
							case 0x03:	//����4				
								Package_Net4set_frame(cfsendframe, file_param);
								can_send(cfsendframe,5);
								break;
							case 0x04:	//����5	
								Package_Net5set_frame(cfsendframe, file_param);
								can_send(cfsendframe,5);
								break;
							case 0x05:	//����6	
								Package_Net6set_frame(cfsendframe, file_param);
								can_send(cfsendframe,5);
								break;
						}
						break;
					case DELAYOFFSET:		
						memset(cfsendframe, 0, CAN_FRAME_LENTH*5);
						Package_delayoffset_frame(cfsendframe,file_param);
						can_send(cfsendframe,5);			
						break;
					case SLOTSTATE:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*2);
						Package_Slot_State_frame(cfsendframe);
						can_send(cfsendframe,2);
						break;
					case MASTER_SLAVERSET:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*1);
						Package_master_slaver_frame(cfsendframe,file_param);
						can_send(cfsendframe,1);
						break;
					case UARTSET:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*1);
						Package_uartset_frame(cfsendframe,file_param);
						can_send(cfsendframe,1);
						break;
					case PRIORIRITYSET:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*1);
						Package_Priority_frame(cfsendframe,file_param);
						can_send(cfsendframe,1);
						break;
					case ZONEINFO:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*1);
						Package_Zoneinfo_frame(cfsendframe,file_param);
						can_send(cfsendframe,1);
						break;
					case PPXSET:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*1);
						Package_PPXtype_frame(cfsendframe,file_param);
						can_send(cfsendframe,1);
						break;
					case AMSET:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*1);
						Package_AM_proportion_frame(cfsendframe,file_param);
						can_send(cfsendframe,1);
						break;
					case TIMESET:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*1);
						Package_settime_frame(cfsendframe,file_param);
						can_send(cfsendframe,1);
						break;
					case BD3DSET:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*3);
						Package_BD3dSET_frame(cfsendframe,file_param);
						can_send(cfsendframe,3);
						break;
					case SUMMERTIME:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*2);
						Package_summerTime_frame(cfsendframe,file_param);	
						can_send(cfsendframe,2);
						break;
					case STATELOG:
						num=recvframe[0].can_id & 0xff;	//�������ʼ����(canid�Ķ����������)
						count=param.event_count-num+1;	//�ظ�����������Ҫ+1
						if((count<1) || (num==0))		//����
							break;
						else if(count>8)
							count=8;					//1�����8��
						memset(cfsendframe, 0, CAN_FRAME_LENTH*count*2);	
						//myprintf("LCD�����[%03d~%03d]����־(%d)\n\n",num,num+count-1,count);
						int log_id,order;
						for(i = 0; i<count ; i++)
						{ 
							order   = count-i-1;	//֡��˳��ע����ÿ����˵�ǵ���
							log_id	= num+i-1;		//������־�ţ�ע��-1
							//myprintf("CAN֡[%02d]:��־[%03d/%03d]\n",order,log_id,param.event_count);
							Package_statelog_frame(cfsendframe,order,log_id,2*count-1);	//֡��=2*count-1
						}						
						can_send(cfsendframe,2*count);
						break;
					case VER_MESS:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*16);
						Package_Versionsmsg_frame(cfsendframe);
						can_send(cfsendframe,16);					
						break;
					case CTRL_BYTE:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*2);
						Package_CtrlByte_frame(cfsendframe);
						can_send(cfsendframe,2);	
						break;
					default:
						break;
				}
			}
		}		

	}
	}
	sprintf(log_buf.name,"rmcrc�߳�3�˳�...");
	add_my_log(log_buf);
	save_my_log();
	disconnect(&socket_fd);		
	pthread_exit(NULL);
}


void* Pthread_4_cansend(void *arg)//ʵʱ֡����

{	
	LOG_MESSAGE log_buf;//д������־buf
	S_CanFrame sendframe[SEND_DYNFRAME_COUNT];
	ToIn_Loc_Data LoData;	//װ�ø澯(����)״̬
	long tmp = (long)arg;//����-W�澯
	tmp=~tmp;
	prctl(PR_SET_NAME, (unsigned long)"rmcrc_4cansend");
	sem_wait(&sem_can_dy_send);//���գ��ټ���1����ӳ�
	while(1)
	{
		memset(&LoData, 0, sizeof(LoData));
		memset(sendframe, 0, CAN_FRAME_LENTH*SEND_DYNFRAME_COUNT);
		sem_wait(&sem_can_dy_send);
		memcpy(&PTI, &FTP, sizeof(FTP));		//���ڵĽ��յ�FTP	
		Package_dynsendframe_data(&LoData);		//�澯����
		Package_dynframe(sendframe,&LoData);	//��̬֡���
		#ifdef __MY_PROG_PTHREAD						
		if((sys_time.min%10==5)&&(sys_time.sec==0)){
			sprintf(log_buf.name,"Pthread_4_cansend ����[%02d]-[%02d:%02d:%02d]",sys_time.day,sys_time.hour,
				sys_time.min,sys_time.sec);
			add_my_log(log_buf);	
		}
		#endif
		can_send(sendframe, SEND_DYNFRAME_COUNT);	
	}
	sprintf(log_buf.name,"rmcrc�߳�4�˳�...");
	add_my_log(log_buf);
	save_my_log();
	pthread_exit(NULL);
}

int set_gateway(int skfd,unsigned long gw);  

int SetIfAddr(char *ifname, unsigned char *Ipaddr, unsigned char *mask,unsigned char *gateway)
{
    int fd,i;
    unsigned int gw ;
    struct ifreq ifr; 			//�����ַ�ṹ
    struct rtentry  rt;			//���ڽṹ
    struct sockaddr_in *sin;	//addr�ṹ
	char  ip_addres[20],Mase[20];
	if(Ipaddr[0]==0)
		return -1;		
	for(i=0;i<20;i++)
	{
		ip_addres[i]=0;
		Mase[i]=0;
	}
	sprintf(ip_addres,"%d.%d.%d.%d",Ipaddr[0],Ipaddr[1],Ipaddr[2],Ipaddr[3]);
	sprintf(Mase,"%d.%d.%d.%d",mask[0],mask[1],mask[2],mask[3]);
	if(0){
		myprintf("SET_IP1:%s\n",ip_addres);
		myprintf("SET_MA1:%s\n",Mase);	
	}
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
    {
        perror("socket   error");     
        return -2;     
    }
    memset(&ifr,0,sizeof(ifr)); 
    strcpy(ifr.ifr_name,ifname); 
    sin = (struct sockaddr_in*)&ifr.ifr_addr; 	  //��sinָ��ifr�еĵ�ַ�ṹ    
    sin->sin_family = AF_INET;   
    if(inet_aton(ip_addres,&(sin->sin_addr)) < 0)//�ַ���IP��ַת��Ϊһ��32λ����������IP��ַ  
    	return -3;     
	//����IP  ���ýӿڵ�ַ
    if(ioctl(fd,SIOCSIFADDR,&ifr) < 0){     	  	
	    perror("ioctl   SIOCSIFADDR   error");     
	    return -4;     
    }
    if(inet_aton(Mase,&(sin->sin_addr)) < 0)
    	return -5;     
	 //������������
    if(ioctl(fd, SIOCSIFNETMASK, &ifr) < 0){
	    perror("ioctl");
	    return -6;
    }
    memset(&rt, 0, sizeof(struct rtentry));
    memset(sin, 0, sizeof(struct sockaddr_in));
	gw=0;
	gw|=gateway[0]<<24;
	gw|=gateway[1]<<16;
	gw|=gateway[2]<<8;
	gw|=gateway[3]<<0;
	//myprintf("gw0=%d.%d.%d.%d\n",(gw&0xff000000)>>24,(gw&0xff0000)>>16,(gw&0xff00)>>8,(gw&0xff)>>0);
    //set_gateway(fd,gw); 
	rt.rt_dst.sa_family = AF_INET;    
	((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;    
	rt.rt_gateway.sa_family = AF_INET;    
	((struct sockaddr_in *)&rt.rt_gateway)->sin_addr.s_addr = gw;    
	//rt.rt_flags = RTF_UP | RTF_GATEWAY;  
	rt.rt_flags = RTF_GATEWAY;  
    //����·��
    int ret = ioctl(fd, SIOCADDRT, &rt);
    if(ret<0){
      close(fd);
      return 1;
    }
    close(fd);
    return 0;
}


int set_gateway(int skfd,unsigned long gw)    
{      
  struct rtentry rt;    
  int err;    
  
  skfd = socket(PF_INET, SOCK_DGRAM, 0);    
  if (skfd < 0)    
    return -1;    
  
  /* Delete existing defalt gateway */    
  memset(&rt, 0, sizeof(rt));    
  
  rt.rt_dst.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;    
  
  rt.rt_genmask.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = 0;    
  
  rt.rt_flags = RTF_UP;    
  
  err = ioctl(skfd, SIOCDELRT, &rt);    
  myprintf("gw1:%d\n",err);
  if ((err == 0 || errno == ESRCH) && gw) {    
  /* Set default gateway */    
  memset(&rt, 0, sizeof(rt));    
  
  rt.rt_dst.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;    
  
  rt.rt_gateway.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_gateway)->sin_addr.s_addr = gw;    
  
  rt.rt_genmask.sa_family = AF_INET;    
  ((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = 0;    
  
  rt.rt_flags = RTF_UP | RTF_GATEWAY;    
  
  err = ioctl(skfd, SIOCADDRT, &rt); 
   myprintf("gw2:%d\n",err);
 } 
	close(skfd);	
	
	return err;    
 }   






