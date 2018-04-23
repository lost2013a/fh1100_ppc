#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>

#include "bsp.h"

#define DEVICE_PTP0		 "/dev/ptp0"
#define DEVICE_Usart1 	 "/dev/ttyS1"

#define SERIAL_LENTH 	 267		//267+6个帧头	

//硬件
clockid_t clkid;		 //PowPC的时间(硬件)入口

/*线程分布*/
/*
线程0 Pthread_0_serail		收FPGA串口，产生Serial信号量,发送配置(串口)到FPGA
//线程2 Pthread_2_devlog		等待Serial，处理日志，并发送CAN0信号量
线程3 Pthread_3_canrec		处理从界面来的CAN信息
线程4 Pthread_4_cansend		等待CAN0信号量，发送CAN0
线程5 Pthread_5_udp         	定时往UDP SOCKET发送
*/
/*同步机制*/
//sem_t sem_serial_accept;
sem_t sem_can_dy_send;
pthread_mutex_t lock_log;		//调试日志(记录)写互斥

/*全局变量*/
SYS_TIME 	  sys_time;		//装置(系统)时间，由线程5负责更新，只用到秒级精度
Mclklog 	  myclog;		//装置状态记录
PARAM		  param;		//装置参数,包含装置的点表
LCD2PPC_MSG   file_param;   //装置的配置，有保存在文件中
PPC2FPGA_MSG  ToFpga;		//发往FPGA串口数据结构
FPGA2PPC_MSG  FTP;			//FPGA的串口数据接收缓存
FPGA2PPC_MSG  PTI;			//FPGA的串口数据(可以看作是装置的状态)
char leapNum=0,leap61=0,leap59=0;
char 	time_acquired;		//已经获得大时间
char    PROGRAM_DATA[20];	//程序版本日期
/*
*其他重要的全局变量*
RECORD mem_record[LOG_MAX_COUNT];		//装置日志(内存中的结构)			定义在log.c
RECORD temp_record[LOG_MAX_COUNT];		//装置日志的临时缓存				定义在log.c
UDP_EVENT	udp_event[EVENT_COUNT];		//UDP上送信息缓存 			定义在udp.c
*/
CLOCK_OFFSET  clock_offset;	//6个时间源计算后的钟差值
PPS_SECOND    pps_second;	//本地和6个时间源换算的秒级时间戳(相对与1970年)
LOSE_LOCK	  lose_lock;	//信号失锁的时间记录
unsigned int  bard_state_udp[16];	//装置的插槽状态(UDP用)

/*局部变量*/
static int  serial_fd = 0;	//和FPGA通信的串口文件的句柄
static void Pthread_0_serail(void);
static clockid_t get_clockid(int fd);

/*外部变量*/

//extern void *Pthread_2_adjtime(void *arg);
extern void *Pthread_3_canrec(void *arg);
extern void *Pthread_4_cansend(void *arg);
extern void *Pthread_5_udp(void *arg);	

int main(int argc,char *argv[])
{
	LOG_MESSAGE log_buf;//写调试日志buf
	int fd,err;
	int argv_tmp;
	pthread_t tid[5];
	memset(&param,0,sizeof(PARAM)); 		//结构体数据清0
	memset(&FTP,0,sizeof(FPGA2PPC_MSG)); 
	memset(&myclog, 0, sizeof(myclog));
	time_acquired=0;
	if(CreateDir()>=0)						//检查(或创建)"/mnt/HD/log"文件夹
		param.dir = 1;
	if(argc==2){
	  	argv_tmp = atoi(argv[1]);
	  	if(argv_tmp&0x01)param.print_serial		= 1;	//打印收到串口
	  	if(argv_tmp&0x02)param.print_can		= 1;	//打印发送Can
	  	if(argv_tmp&0x04)param.print_serial_tx	= 1;	//打印发送串口
	  	if(argv_tmp&0x08)param.print_prog_log	= 1;	//打印状态日志			
  	}
	create_log(0);
	main_initial(1);			//初始化参数配置，包括网口的配置参数
	dev_console_init();
	com_event.op_flag = 0;		//Comm日志的记录表标志位可用
	com_event.event_count=0;	//Comm日志的记录表记录清0
	param.writ_event_time=60;	//开机后即可写装置日志
	
  	read_log_flie();			//读取保存在文件系统的的装置日志
  	read_board_state();			//从文件读取插槽状态，只在开机的时候使用
	serial_fd = UART0_Open(DEVICE_Usart1);
	while(serial_fd==FALSE){		//打开串口
		sleep(2);
		serial_fd = UART0_Open(DEVICE_Usart1);
	}
	err = UART0_Init(serial_fd,BAUDRATE,0,8,1,'n');
	while(err==FALSE){				//配置串口
		sleep(2);
		err = UART0_Init(serial_fd,BAUDRATE,0,8,1,'n');
	}
	fd = open(DEVICE_PTP0, O_RDWR);
	while(fd<0) {					//打开硬件时间入口
		sleep(2);
		fd = open(DEVICE_PTP0, O_RDWR);
	}
	clkid = get_clockid(fd);
	while(clkid == -1){	//硬件时间不可读写
		sleep(2);
		clkid = get_clockid(fd);
	}
	close_can(CAN_PORT);			//必须先关闭CAN，才能设置CAN波特率
	open_can(CAN_PORT, CAN_BITRATE);
    socket_fd = socket_connect(CAN_PORT);
    if (socket_fd < 0){      
        disconnect(&socket_fd);
        panic("socket can ERR\n");
    }
	sprintf(PROGRAM_DATA,"%c%c%c%c-%c%c-%c%c "__TIME__"",BUILD_Y0,BUILD_Y1,BUILD_Y2,BUILD_Y3,
			BUILD_M0,BUILD_M1,BUILD_D0,BUILD_D1);
	sprintf(log_buf.name,"---CPU系统初始化完成! (程序版本日期: %s)",PROGRAM_DATA);
	add_my_log(log_buf);
	log_buf.name[255]='0';
	myprintf("%s\n",log_buf.name);
	read_time();
	save_my_log();					//BSP硬件初始化完成，写调试日志
	
	/*初始化同步信号*/
	/*
	sem_init(&sem_serial_accept, 0, 0);
	*/
	sem_init(&sem_can_dy_send, 0, 0);
	pthread_mutex_init(&lock_log, NULL);
	//pthread_create(&tid[1], NULL, Pthread_2_adjtime, NULL);
	pthread_create(&tid[2], NULL, Pthread_3_canrec, NULL);
	pthread_create(&tid[3], NULL, Pthread_4_cansend, NULL);
	pthread_create(&tid[4], NULL, Pthread_5_udp, NULL);
	
	/*主线程-串口接收数据*/
	Pthread_0_serail();
	//pthread_join(tid[1], NULL);
	pthread_join(tid[2], NULL);
	pthread_join(tid[3], NULL);
	pthread_join(tid[4], NULL);
	close(serial_fd);
	pthread_exit(NULL);
	return 0;
}

void mydebug_FTP(viod)
{
	if(0)	//调试
	{	
		static int dbgtime=0;
		if(sys_time.sec==0)
			dbgtime++;
		switch(dbgtime){
		case 0:
			break;	
		case 1:
			
			//FTP.LoTimestate|=0x80; 	//正闰秒1
			//FTP.LoTimestate&=0xbf;	//正闰秒2
			
			FTP.LoTimestate|=0xc0; 	//负闰秒
			if(sys_time.sec==59)	//
			FTP.LoTimestate&=0x7f; 	//负闰秒
			
			break;	
		case 2:
			
			//if(sys_time.sec==0){
			//	FTP.LoTimestate|=0x80; 	//正闰秒1
			//	FTP.LoTimestate&=0xbf;	//正闰秒2
			//}
			
			break;	
		case 3:	
			break;	
		default:
			break;
		}
	}
	if(0)	//调试同步
	{	
		static int dbgtime=0;
		if(sys_time.sec==0)
			dbgtime++;
		switch(dbgtime){
		case 0:
			FTP.Self_Checkstate|=0X08;
			if(sys_time.sec==3)
			myprintf("Self_Checkstate1=%02x\n",FTP.Self_Checkstate);
			break;	
		case 1:
			
			FTP.Self_Checkstate&=(~0X08);
			if(sys_time.sec==3)
			myprintf("Self_Checkstate2=%02x\n",FTP.Self_Checkstate);
			break;	
		case 2:
			break;	
		case 3:	
			break;	
		default:
			break;
		}
	}

	if(0)	//调试晶振驯服
	{	
		static int dbgtime=0;
		unsigned char dbgdata = 0x20;
		if(sys_time.sec==0)
			dbgtime++;
		switch(dbgtime){
		case 0:
			FTP.Self_Checkstate|=dbgdata;
			if(sys_time.sec==0)
			myprintf("Self_Checkstate1=%02x\n",FTP.Self_Checkstate);
			break;		
		case 1:
			FTP.Self_Checkstate|=dbgdata;
			if(sys_time.sec==0)
			myprintf("Self_Checkstate1=%02x\n",FTP.Self_Checkstate);
			break;	
		case 2:
			
			FTP.Self_Checkstate&=(~dbgdata);
			if(sys_time.sec==0)
			myprintf("Self_Checkstate2=%02x\n",FTP.Self_Checkstate);
			break;	
		case 3:
			FTP.Self_Checkstate|=dbgdata;
			if(sys_time.sec==0)
			myprintf("Self_Checkstate2=%02x\n",FTP.Self_Checkstate);
			break;	
			break;	
		case 4:	
			break;	
		default:
			break;
		}
	}
	
	if(0)	//调试模块异常
	{	
		static int dbgtime=0;
		unsigned char dbgdata = 0x20;
		if(sys_time.sec==0)
			dbgtime++;
		switch(dbgtime){
		case 0:
			FTP.Self_Checkstate|=dbgdata;
			if(sys_time.sec==0)
			myprintf("Self_Checkstate1=%02x\n",FTP.Self_Checkstate);
			break;		
		case 1:
			FTP.Self_Checkstate|=dbgdata;
			if(sys_time.sec==0)
			myprintf("Self_Checkstate1=%02x\n",FTP.Self_Checkstate);
			break;	
		case 2:
			
			FTP.Self_Checkstate&=(~dbgdata);
			if(sys_time.sec==0)
			myprintf("Self_Checkstate2=%02x\n",FTP.Self_Checkstate);
			break;	
		case 3:
			FTP.Self_Checkstate|=dbgdata;
			if(sys_time.sec==0)
			myprintf("Self_Checkstate2=%02x\n",FTP.Self_Checkstate);
			break;	
			break;	
		case 4:	
			break;	
		default:
			break;
		}
	}

	
}



void Pthread_0_serail()
{
	LOG_MESSAGE log_buf;//写调试日志buf
	__u8 buf[300] = {0};
	__u8 recy_crc = 0, accept_crc = 0;
	int datalen = 0;
	int i = 0;
	int ret = 0,serail_err=1;
	struct timeval timeover;
 	fd_set rfds;
	Time serialTime;
	FD_ZERO(&rfds);
	FD_SET(serial_fd, &rfds);
	prctl(PR_SET_NAME, (unsigned long)"rmcrc_0serail");
	while(1)
	{	
		timeover.tv_sec = 1;
		timeover.tv_usec = 500000;
		ret = select(serial_fd+1, &rfds, NULL, NULL, &timeover);
		switch(ret)
		{
			case -1:
				perror("select");
				break;
			case 0:
				printf("waitting FPGA serails time out!\n");
				break;
			default:	
				if(FD_ISSET(serial_fd, &rfds))	
				{	
					datalen = 0;	
					memset(buf, 0, 300); 
					recy_crc = 0;
					accept_crc = 0;	
					UART0_Recv(serial_fd, &buf[0], 1);
					if(buf[0] == 0xeb)
					{	
						UART0_Recv(serial_fd, buf+1, 5);	
						if(buf[1] == 0x90 && buf[2] == 0xeb && buf[3] == 0x90)
						{					
							datalen |= buf[4] << 8 ;
							datalen |= buf[5];
							__u8* data = buf + 6;
							int dusec;
							dusec=(1000*1000*datalen/BAUDRATE)*10+120000;//补偿了处理时间120ms
							usleep(dusec);
							//int nread;
							//ioctl(serial_fd,FIONREAD,&nread);//测试在fd中还有nread个字符需要读取
							//myprintf("serial_fd nread(delay%dms):%d\n\n",dusec/1000,nread);
							UART0_Recv(serial_fd, data, datalen+1);	
							recy_crc = buf[6+datalen];
							i=1;
							if(param.print_serial)
								printf("recv fpga data:\n");
							while(datalen--)	
							{
								if(param.print_serial)
								{
									printf("%02x ",*data);
									if(i==49||i==85||i==121||i==137||i==153||i==192||i==231||i==239||i==247||i==257||i==267)
										printf("\n");
									i++;	
								}	
								accept_crc ^= *data++;	
							}
							if(accept_crc == recy_crc){
								
								memset(&FTP, 0, sizeof(FTP));
								memcpy(&FTP, buf+6, SERIAL_LENTH);     //报文复制到内存的数据结构中
								mydebug_FTP();
								if(param.rx_serial<2){
									if(param.rx_serial==1){
										sprintf(log_buf.name,"FPGA串口接收正常");
										add_my_log(log_buf);
									}
									param.rx_serial++;//修改记录2017.11.20
									serail_err=0;
								}
								else{
									serialTime.y.year   = FTP.Locatetime.year[1]|(FTP.Locatetime.year[0] << 8);
									serialTime.mo.month = FTP.Locatetime.month;
									serialTime.d.day 	= FTP.Locatetime.day;
									serialTime.h.hour 	= FTP.Locatetime.hour;
									serialTime.m.min 	= FTP.Locatetime.min;
									serialTime.s.second = FTP.Locatetime.second;
									SetSystemTime();
									temp_record_count=0;
									if(param.read_param){		//在作装置逻辑前，尝试读取最新的配置，其与逻辑处理密切相关
										save_param();			//把file_param的值写入文件
										read_param();			//把配置文件的值读入到file_param和ToFpga的值
										dev_console_init();		//把file_param的值传入parm
										myprintf("参数改变，重新读取配置\n");
										param.read_param = 0;
									}
									Run_BoardState(serialTime);	//插槽状态逻辑
									Run_DevData();				//装置数据计算
									comp_clock_offset();		//计算6个源的钟差值
									Run_DevLog(serialTime);		//装置日志逻辑
									Update_DevLog();			//装置写文件(尽量减慢FLASH擦写)
									
									sem_getvalue(&sem_can_dy_send, &i);
									if(i<1)
									sem_post(&sem_can_dy_send);
									
								}
								usleep(100*1000);
								send_serial_fpga();
							}			
						}			
					}
					else{
						read(serial_fd, &buf[1], 300-2);//避免错位
						if(serail_err<3){
							serail_err++;
							sprintf(log_buf.name,"FPGA串口接收发生异常[%02d字节](连续错误%d次)",ret,serail_err);
							add_my_log(log_buf);
						}
						usleep(100*1000);					   //延迟100毫秒
					}
				}
				break;
			}		
	}	
}


void send_serial_fpga()
{
	unsigned int i;
	int send_crc = 0;
	int date_len = 0;
	__u8 *data = NULL;
	__u8 databuf[256] = {0};
	__u8 headbuf[4] = {0xeb,0x90,0xeb,0x90};
	__u8 buff[200]  = {0};
	static int fpga_initial_count=0;
	if(fpga_initial_count<5){	//初始阶段不发配置信息
		fpga_initial_count++;
		return;	
	}
	deal_config_fpga();        //数据处理
	memcpy(ToFpga.Check_head, headbuf, sizeof(headbuf));
	ToFpga.Packet_len = 159;//0xA7;  
 	/*用于计算crc*/
	date_len = (int)ToFpga.Packet_len;
	memcpy(databuf, &ToFpga,sizeof(ToFpga));
	data = databuf + 5;
	while(date_len--) 
	  send_crc ^=(*(data++));
	ToFpga.crc = send_crc;
	//int j,k;
	memcpy(buff, &ToFpga, sizeof(ToFpga));
	UART0_Send(serial_fd, &ToFpga, sizeof(ToFpga));	
	if(param.print_serial_tx)
	{
		memcpy(databuf, &ToFpga,sizeof(ToFpga));
		printf("------------------send to fpga length=%d\n",sizeof(ToFpga));
		for(i=0;i<sizeof(ToFpga);i++)
		{
			printf("%02x ",buff[i]);
			if(i==4||i==18||i==58||i==64||i==79||i==98||i==128||i==158||i==166)
				printf("\n");
		}
		printf("\n");
	}
}



/*处理发送到fpga的数据*/
int  deal_config_fpga(void)
{
	//LOG_MESSAGE log_buf;//写调试日志buf
	static char old_wang_state=0;
	//__u8	FaultLamp = 0;//故障灯
	//__u8	WarnLamp = 0;//告警灯
	//__u8 Random_Source_off =0;
	Dev_S_FPGA2MSG dyndata;
	memset(&dyndata, 0, sizeof(dyndata));
	memset(&dyndata, 0, sizeof(dyndata));
	char cTemp = (myclog.freque1/10000)%100;
	ToFpga.summerTime.end.year = (cTemp/10)*16+cTemp%10;
	cTemp = (myclog.freque1/100)%100;
	ToFpga.summerTime.end.month = (cTemp/10)*16+cTemp%10;
	cTemp = myclog.freque1%100;
	ToFpga.summerTime.end.day = (cTemp/10)*16+cTemp%10;

	cTemp = (myclog.freque2/10000)%100;
	ToFpga.summerTime.end.hour = (cTemp/10)*16+cTemp%10;
	cTemp = (myclog.freque2/100)%100;
	ToFpga.summerTime.end.min = (cTemp/10)*16+cTemp%10;
	cTemp = myclog.freque2%100;
	ToFpga.summerTime.end.second = (cTemp/10)*16+cTemp%10;
	//printf("电网频率1=%d, %x, %x, %x\n",myclog.freque1,ToFpga.summerTime.end.year,ToFpga.summerTime.end.month,ToFpga.summerTime.end.day);
	//printf("电网频率2=%d, %x, %x, %x\n",myclog.freque2,ToFpga.summerTime.end.hour,ToFpga.summerTime.end.min,ToFpga.summerTime.end.second);
	dyndata.P1010_Source_result = 0xff;
	if(param.warn)  dyndata.P1010_SelfCheck |= 0x02;
	if(param.fault) dyndata.P1010_SelfCheck |= 0x01;
	if((dyndata.P1010_SelfCheck&0x03)	!= old_wang_state)
	{
		//sprintf(log_buf.name,"装置告警(故障)状态发生变化: %d-->%d",old_wang_state,dyndata.P1010_SelfCheck&0x03);
		//add_my_log(log_buf);
		old_wang_state = dyndata.P1010_SelfCheck&0x03;
	}	
	memcpy(&(ToFpga.Dyn_FPGA_message), &dyndata,sizeof(Dev_S_FPGA2MSG));
	return 0;
}


int myfilter(int *off,int d,char *b,char *i)
{
	int ret = 0;
	int absoff ;
	absoff = abs(*off);
	if(*b < 4){
		if(*i < 4){ 			//稳态还未建立
			if(absoff < d)
				(*i)++;	
			else if((*i)>0)
				(*i)--;
		}
		else {					//稳态已经建立	
			if(absoff<(d/2)){
				if(*b>0){
					(*b)--;
				}
			}
			else {
				(*b)++; 		
				*off = 0; 		//忽略此次偶然的波动
			}
			ret=1;
		}
	}
	else{  						 //不稳定计数>5,稳态已经破坏
		*i = 0;
		*b = 0;
	}
	//myprintf("off=%d, *i=%d, *b=%d\n",*off,*i,*b);
	return ret;
}



/*校正powerpc的时间*/
int SetSystemTime()
{
	static  char b_syst=0,i_syst=0;
	static  char b_hadwaret=0,i_hadwaret=0;	
	struct tm serialtime;
	struct timeval tv,tvppc;
	struct timespec ts;
	CTime 	serialtime_t;
	int serialtime_t_year = 0;
	int ppc_off = 0;
	int res;
	memset(&serialtime_t, 0, sizeof(serialtime_t));
	memset(&serialtime, 0, sizeof(serialtime));
	memcpy(&serialtime_t, &FTP.Locatetime,sizeof(serialtime_t));
	serialtime_t_year |= serialtime_t.year[0]<< 8;
	serialtime_t_year |= serialtime_t.year[1] ;
	serialtime.tm_sec = (int)(serialtime_t.second);
	serialtime.tm_min = (int)(serialtime_t.min);
	serialtime.tm_hour = (int)(serialtime_t.hour);
	serialtime.tm_mday = (int)(serialtime_t.day);
	serialtime.tm_mon = (int)(serialtime_t.month)-1;
	serialtime.tm_year = serialtime_t_year - 1900;
	/*获取当前系统时间*/
	gettimeofday(&tvppc,NULL);
	  
	/*串口时间与power时间时差大于1s，设置系统时间*/	
	int OFFSET = 10*1000*(SERIAL_LENTH+6+1)/BAUDRATE + 120+4;//补偿了系统调用时间124ms
	int serial_sec = mktime(&serialtime);
	ppc_off = serial_sec*1000 + OFFSET - tvppc.tv_sec*1000 - tvppc.tv_usec/1000;
	//if(param.print_prog_log)
	//	myprintf("serial_time off:%d\n",ppc_off);
	res = myfilter(&ppc_off,500,&b_syst,&i_syst);
	if((res>0)&&(sys_time.year!=2010)){
		if(time_acquired == 0){
			time_acquired=1;
		}
	}
	if(1){//闰秒实现
		if((FTP.LoTimestate&0x80) >>7){	//闰秒时，不使用串口时间修改时间
			if(leapNum<5)
				leapNum++;
			else{
				if((FTP.LoTimestate&0x40) >>6 )
					leap59=1;
				else
					leap61=1;
			}	
		}
		else{
			if(leapNum<=0){
				leap61=0;
				leap59=0;
			}
			else{
				leapNum--;
			}
		}	
	}
	if(ppc_off > 500 || ppc_off < -500)
	{
		myprintf("修改系统时间：ppc_off=%d,sys_time=%d.%d,serialtime=%d\n",  
			(int)ppc_off,(int)tvppc.tv_sec,(int)tvppc.tv_usec,(int)serial_sec);		
		tv.tv_sec = serial_sec;
		tv.tv_usec = 1000*OFFSET;
		settimeofday(&tv, (struct timezone*)0);
	}
	if (clock_gettime(clkid, &ts)==0)
	{
		ppc_off =serial_sec*1000 + OFFSET - ts.tv_sec*1000 - ts.tv_nsec/1000000;
		myfilter(&ppc_off,500,&b_hadwaret,&i_hadwaret);
		if(ppc_off > 500 || ppc_off < -500)
		{
			myprintf("修改硬件时间：ppc_off=%d,hw=%d.%d,serial_time=%d\n",  
				ppc_off,(int)ts.tv_sec,(int)ts.tv_nsec/1000,(int)serial_sec);
			ts.tv_sec = serial_sec;
			ts.tv_nsec = (long)1000000*OFFSET;
			clock_settime(clkid, &ts);
		}
	}
	//myprintf("sys_b=%d,sys_i=%d\n",b_syst,i_syst);
	//myprintf("hws_b=%d,hws_i=%d\n",b_hadwaret,i_hadwaret);
	return 0;
}


void handleap(struct timeval *tv)
{
	struct timespec ts;
	int adj=0;
	if(leapNum >= 3)//连续3次闰秒预告
	{
		
		if((leap61 != 0)&&((tv->tv_sec)%60 == 1))		//正闰秒发生
			adj=-1;	//重复一秒
		else if((leap59 != 0)&&((tv->tv_sec)%60 == 59))//负闰秒发生
			adj=1;	//跳过一秒	
		else
			return;
		tv->tv_sec+=adj;
		if(tv->tv_usec<500*1000)
			tv->tv_usec+=500;	//补偿了500us
		settimeofday(tv, (struct timezone*)0);
		myprintf("闰秒(%d)-修改系统时间 sys_time=%d.%d\n",  adj,(int)tv->tv_sec,(int)tv->tv_usec);		
		if (clock_gettime(clkid, &ts)==0)
		{
			ts.tv_sec = tv->tv_sec;
			ts.tv_nsec = (long)1000*tv->tv_usec;	
			clock_settime(clkid, &ts);
			myprintf("闰秒(%d)-修改硬件时间 hw_time=%d.%d\n",  adj,(int)ts.tv_sec,(int)ts.tv_nsec/1000);
		}
		leap61 	=0;	
		leap59 	=0;
		leapNum	=0;
	}
}



clockid_t get_clockid(int fd)
{
#define CLOCKFD 3
#define FD_TO_CLOCKID(fd)	((~(clockid_t) (fd) << 3) | CLOCKFD)
	return FD_TO_CLOCKID(fd);
}



