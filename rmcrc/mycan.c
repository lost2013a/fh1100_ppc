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
				usleep(1000);	//阻塞线程
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
	count=count%6;//避免越界
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
	LOG_MESSAGE log_buf;//写调试日志buf
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
		if(ret==1){	//网关路由不正确，此时IP已经被设置，需要重设到上次的状态
			SetIfAddr(bsp_net[port], Net_set->ip, Net_set->mask,Net_set->wg);
			sprintf(log_buf.name,"网口[%d]路由表不正确",port+1);
		}
		else{
			sprintf(log_buf.name,"网口[%d]设置失败,errno=%02d",port+1,ret);
		}
		add_my_log(log_buf);
	}
	else{
		sprintf(log_buf.name,"网口[%d]参数修改,IP地址:%d.%d.%d.%d",
			port+1,Net_set->ip[0],Net_set->ip[1],Net_set->ip[2],Net_set->ip[3]);
		add_my_log(log_buf);
	}
}



void *Pthread_3_canrec(void *arg)
{
	LOG_MESSAGE log_buf;		//写调试日志buf
	S_CanFrame  recvframe[6];	//多申请了1个，便于丢弃数据
	S_CanFrame  cfsendframe[16];
	int i,ret;
	unsigned int num,count;
	fd_set rfds;
	long tmp = (long)arg;//避免-W告警
	tmp=~tmp;
	FD_ZERO(&rfds); 			/*将set清零使集合中不含任何fd*/
	FD_SET(socket_fd, &rfds);	/*将fd加入set集合*/
	prctl(PR_SET_NAME, (unsigned long)"rmcrc_3canrec"); 
	
	for(;;)	//双循环,避免错误调用退出条件;
	{	
	while(1)
	{	
		ret=select(socket_fd+1, &rfds, NULL, NULL, NULL);//多路检测可用套接字,返    回满足条件的套接口的数目
		if(ret==-1){
			perror("select");
			continue;
		}
		if(FD_ISSET(socket_fd, &rfds))		//就绪条件来自	rfds
		{	
			count=can_recv(&recvframe[0],1);
			if (count<1){
				continue;
			}
			int frame_type = (recvframe[0].can_id&0xff00)>>8;	//报文类型
			int frame_RTR  = recvframe[0].can_id&CAN_RTR_FLAG;  //报文包数据帧还是远程帧
			if(frame_RTR==0)	 								//配置参数(数据帧)
			{	
				int frame_id = recvframe[0].can_id & 0x0f;	  	//报文包的帧序
				if(frame_id){								  	//不是帧头(0)
					can_recv(&recvframe[0],5);//读完剩余的并丢掉
					myprintf("CAN数据帧未收到帧头:(%d)帧,[%d.%d]\n",frame_id,sys_time.min,sys_time.sec);
					continue;
				}
				int frame_nr = (recvframe[0].can_id&0xf0)>>4;	//报文包的总帧数(已-1)
				if(frame_nr>0){									//报文还没有接收完整
					count+=can_recv(&recvframe[1],frame_nr);	//注意从帧1开始继续读取
					int order1,order2;
					for(i=0;i<frame_nr;i++){
						order1=recvframe[i+1].can_id;
						order2=recvframe[i].can_id;
						//myprintf("i:%d,oredr1:%x,oredr2:%x\n",i,order1,order2);
						if((order1-order2)<1)//必须是顺序
						{	
							myprintf("--CAN数据帧not in order\n");
							count=0;							 //
						}	
					}
					if(count<frame_nr)							 //没有收到足够的帧
						continue;
				}
				myprintf("--CAN数据帧收到:(%d/%d)帧,type:<%02d>,[%d.%d]\n",count,frame_nr+1,frame_type,sys_time.min,sys_time.sec);
				switch(frame_type)
				{
					case NETSET:
						num = recvframe[0].can_id >> 16 & 0xf ;//取十六进制第5位的值；先向左移动12位，再向右移动28位为网口号
						switch(num)
						{
							case 0x00:	//网口1	
								handle_canrec_netset(&file_param.Net_set1,&recvframe[0],0);
								break;
							case 0x01:	//网口2	
								handle_canrec_netset(&file_param.Net_set2,&recvframe[0],1);
								break;
							case 0x02:	//网口3	
								handle_canrec_netset(&file_param.Net_set3,&recvframe[0],2);
								break;
							case 0x03:	//网口4	
								handle_canrec_netset(&file_param.Net_set4,&recvframe[0],3);
								break;
							case 0x04:	//网口5	
								handle_canrec_netset(&file_param.Net_set5,&recvframe[0],4);
								break;
							case 0x05://网口6
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
						param.initial=0;	//初始化未完成
						break;
					default:
						can_recv(&recvframe[0],5);//读完剩余的并丢掉
						myprintf("类型未知不对的报文，丢掉\n");
						break;
					}
			}
			else{ 	//远程帧；
				//myprintf("--CAN远程帧收到:(%d)帧,type:<%02d>,[%d.%d]\n",count,frame_type,sys_time.min,sys_time.sec);
				switch(frame_type)
				{
					case NETSET:
						memset(cfsendframe, 0, CAN_FRAME_LENTH*5);
						num = recvframe[0].can_id >> 16 & 0xf; 
						switch(num)
						{
							case 0x00:	//网口1	
								Package_Net1set_frame(cfsendframe, file_param);
								can_send(cfsendframe,5);
								break;
							case 0x01:	//网口2				
								Package_Net2set_frame(cfsendframe, file_param);
								can_send(cfsendframe,5);
								break;
							case 0x02:	//网口3	
								Package_Net3set_frame(cfsendframe, file_param);
								can_send(cfsendframe,5);
								break;
							case 0x03:	//网口4				
								Package_Net4set_frame(cfsendframe, file_param);
								can_send(cfsendframe,5);
								break;
							case 0x04:	//网口5	
								Package_Net5set_frame(cfsendframe, file_param);
								can_send(cfsendframe,5);
								break;
							case 0x05:	//网口6	
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
						num=recvframe[0].can_id & 0xff;	//请求的起始条数(canid的定义是特殊的)
						count=param.event_count-num+1;	//回复多少条，需要+1
						if((count<1) || (num==0))		//出错
							break;
						else if(count>8)
							count=8;					//1次最多8条
						memset(cfsendframe, 0, CAN_FRAME_LENTH*count*2);	
						//myprintf("LCD请求第[%03d~%03d]条日志(%d)\n\n",num,num+count-1,count);
						int log_id,order;
						for(i = 0; i<count ; i++)
						{ 
							order   = count-i-1;	//帧的顺序，注意是每包来说是倒序
							log_id	= num+i-1;		//计算日志号，注意-1
							//myprintf("CAN帧[%02d]:日志[%03d/%03d]\n",order,log_id,param.event_count);
							Package_statelog_frame(cfsendframe,order,log_id,2*count-1);	//帧数=2*count-1
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
	sprintf(log_buf.name,"rmcrc线程3退出...");
	add_my_log(log_buf);
	save_my_log();
	disconnect(&socket_fd);		
	pthread_exit(NULL);
}


void* Pthread_4_cansend(void *arg)//实时帧发送

{	
	LOG_MESSAGE log_buf;//写调试日志buf
	S_CanFrame sendframe[SEND_DYNFRAME_COUNT];
	ToIn_Loc_Data LoData;	//装置告警(警告)状态
	long tmp = (long)arg;//避免-W告警
	tmp=~tmp;
	prctl(PR_SET_NAME, (unsigned long)"rmcrc_4cansend");
	sem_wait(&sem_can_dy_send);//保险，再加了1秒的延迟
	while(1)
	{
		memset(&LoData, 0, sizeof(LoData));
		memset(sendframe, 0, CAN_FRAME_LENTH*SEND_DYNFRAME_COUNT);
		sem_wait(&sem_can_dy_send);
		memcpy(&PTI, &FTP, sizeof(FTP));		//串口的接收的FTP	
		Package_dynsendframe_data(&LoData);		//告警解析
		Package_dynframe(sendframe,&LoData);	//动态帧组包
		#ifdef __MY_PROG_PTHREAD						
		if((sys_time.min%10==5)&&(sys_time.sec==0)){
			sprintf(log_buf.name,"Pthread_4_cansend 正常[%02d]-[%02d:%02d:%02d]",sys_time.day,sys_time.hour,
				sys_time.min,sys_time.sec);
			add_my_log(log_buf);	
		}
		#endif
		can_send(sendframe, SEND_DYNFRAME_COUNT);	
	}
	sprintf(log_buf.name,"rmcrc线程4退出...");
	add_my_log(log_buf);
	save_my_log();
	pthread_exit(NULL);
}

int set_gateway(int skfd,unsigned long gw);  

int SetIfAddr(char *ifname, unsigned char *Ipaddr, unsigned char *mask,unsigned char *gateway)
{
    int fd,i;
    unsigned int gw ;
    struct ifreq ifr; 			//网络地址结构
    struct rtentry  rt;			//网口结构
    struct sockaddr_in *sin;	//addr结构
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
    sin = (struct sockaddr_in*)&ifr.ifr_addr; 	  //把sin指向ifr中的地址结构    
    sin->sin_family = AF_INET;   
    if(inet_aton(ip_addres,&(sin->sin_addr)) < 0)//字符串IP地址转换为一个32位的网络序列IP地址  
    	return -3;     
	//设置IP  设置接口地址
    if(ioctl(fd,SIOCSIFADDR,&ifr) < 0){     	  	
	    perror("ioctl   SIOCSIFADDR   error");     
	    return -4;     
    }
    if(inet_aton(Mase,&(sin->sin_addr)) < 0)
    	return -5;     
	 //设置子网掩码
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
    //增加路径
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






