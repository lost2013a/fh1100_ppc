#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>

#include "bsp.h"

#define DEVICE_PTP0		 "/dev/ptp0"
#define DEVICE_Usart1 	 "/dev/ttyS1"

#define SERIAL_LENTH 	 267		//267+6��֡ͷ	

//Ӳ��
clockid_t clkid;		 //PowPC��ʱ��(Ӳ��)���

/*�̷ֲ߳�*/
/*
�߳�0 Pthread_0_serail		��FPGA���ڣ�����Serial�ź���,��������(����)��FPGA
//�߳�2 Pthread_2_devlog		�ȴ�Serial��������־��������CAN0�ź���
�߳�3 Pthread_3_canrec		����ӽ�������CAN��Ϣ
�߳�4 Pthread_4_cansend		�ȴ�CAN0�ź���������CAN0
�߳�5 Pthread_5_udp         	��ʱ��UDP SOCKET����
*/
/*ͬ������*/
//sem_t sem_serial_accept;
sem_t sem_can_dy_send;
pthread_mutex_t lock_log;		//������־(��¼)д����

/*ȫ�ֱ���*/
SYS_TIME 	  sys_time;		//װ��(ϵͳ)ʱ�䣬���߳�5������£�ֻ�õ��뼶����
Mclklog 	  myclog;		//װ��״̬��¼
PARAM		  param;		//װ�ò���,����װ�õĵ��
LCD2PPC_MSG   file_param;   //װ�õ����ã��б������ļ���
PPC2FPGA_MSG  ToFpga;		//����FPGA�������ݽṹ
FPGA2PPC_MSG  FTP;			//FPGA�Ĵ������ݽ��ջ���
FPGA2PPC_MSG  PTI;			//FPGA�Ĵ�������(���Կ�����װ�õ�״̬)
char leapNum=0,leap61=0,leap59=0;
char 	time_acquired;		//�Ѿ���ô�ʱ��
char    PROGRAM_DATA[20];	//����汾����
/*
*������Ҫ��ȫ�ֱ���*
RECORD mem_record[LOG_MAX_COUNT];		//װ����־(�ڴ��еĽṹ)			������log.c
RECORD temp_record[LOG_MAX_COUNT];		//װ����־����ʱ����				������log.c
UDP_EVENT	udp_event[EVENT_COUNT];		//UDP������Ϣ���� 			������udp.c
*/
CLOCK_OFFSET  clock_offset;	//6��ʱ��Դ�������Ӳ�ֵ
PPS_SECOND    pps_second;	//���غ�6��ʱ��Դ������뼶ʱ���(�����1970��)
LOSE_LOCK	  lose_lock;	//�ź�ʧ����ʱ���¼
unsigned int  bard_state_udp[16];	//װ�õĲ��״̬(UDP��)

/*�ֲ�����*/
static int  serial_fd = 0;	//��FPGAͨ�ŵĴ����ļ��ľ��
static void Pthread_0_serail(void);
static clockid_t get_clockid(int fd);

/*�ⲿ����*/

//extern void *Pthread_2_adjtime(void *arg);
extern void *Pthread_3_canrec(void *arg);
extern void *Pthread_4_cansend(void *arg);
extern void *Pthread_5_udp(void *arg);	

int main(int argc,char *argv[])
{
	LOG_MESSAGE log_buf;//д������־buf
	int fd,err;
	int argv_tmp;
	pthread_t tid[5];
	memset(&param,0,sizeof(PARAM)); 		//�ṹ��������0
	memset(&FTP,0,sizeof(FPGA2PPC_MSG)); 
	memset(&myclog, 0, sizeof(myclog));
	time_acquired=0;
	if(CreateDir()>=0)						//���(�򴴽�)"/mnt/HD/log"�ļ���
		param.dir = 1;
	if(argc==2){
	  	argv_tmp = atoi(argv[1]);
	  	if(argv_tmp&0x01)param.print_serial		= 1;	//��ӡ�յ�����
	  	if(argv_tmp&0x02)param.print_can		= 1;	//��ӡ����Can
	  	if(argv_tmp&0x04)param.print_serial_tx	= 1;	//��ӡ���ʹ���
	  	if(argv_tmp&0x08)param.print_prog_log	= 1;	//��ӡ״̬��־			
  	}
	create_log(0);
	main_initial(1);			//��ʼ���������ã��������ڵ����ò���
	dev_console_init();
	com_event.op_flag = 0;		//Comm��־�ļ�¼���־λ����
	com_event.event_count=0;	//Comm��־�ļ�¼���¼��0
	param.writ_event_time=60;	//�����󼴿�дװ����־
	
  	read_log_flie();			//��ȡ�������ļ�ϵͳ�ĵ�װ����־
  	read_board_state();			//���ļ���ȡ���״̬��ֻ�ڿ�����ʱ��ʹ��
	serial_fd = UART0_Open(DEVICE_Usart1);
	while(serial_fd==FALSE){		//�򿪴���
		sleep(2);
		serial_fd = UART0_Open(DEVICE_Usart1);
	}
	err = UART0_Init(serial_fd,BAUDRATE,0,8,1,'n');
	while(err==FALSE){				//���ô���
		sleep(2);
		err = UART0_Init(serial_fd,BAUDRATE,0,8,1,'n');
	}
	fd = open(DEVICE_PTP0, O_RDWR);
	while(fd<0) {					//��Ӳ��ʱ�����
		sleep(2);
		fd = open(DEVICE_PTP0, O_RDWR);
	}
	clkid = get_clockid(fd);
	while(clkid == -1){	//Ӳ��ʱ�䲻�ɶ�д
		sleep(2);
		clkid = get_clockid(fd);
	}
	close_can(CAN_PORT);			//�����ȹر�CAN����������CAN������
	open_can(CAN_PORT, CAN_BITRATE);
    socket_fd = socket_connect(CAN_PORT);
    if (socket_fd < 0){      
        disconnect(&socket_fd);
        panic("socket can ERR\n");
    }
	sprintf(PROGRAM_DATA,"%c%c%c%c-%c%c-%c%c "__TIME__"",BUILD_Y0,BUILD_Y1,BUILD_Y2,BUILD_Y3,
			BUILD_M0,BUILD_M1,BUILD_D0,BUILD_D1);
	sprintf(log_buf.name,"---CPUϵͳ��ʼ�����! (����汾����: %s)",PROGRAM_DATA);
	add_my_log(log_buf);
	log_buf.name[255]='0';
	myprintf("%s\n",log_buf.name);
	read_time();
	save_my_log();					//BSPӲ����ʼ����ɣ�д������־
	
	/*��ʼ��ͬ���ź�*/
	/*
	sem_init(&sem_serial_accept, 0, 0);
	*/
	sem_init(&sem_can_dy_send, 0, 0);
	pthread_mutex_init(&lock_log, NULL);
	//pthread_create(&tid[1], NULL, Pthread_2_adjtime, NULL);
	pthread_create(&tid[2], NULL, Pthread_3_canrec, NULL);
	pthread_create(&tid[3], NULL, Pthread_4_cansend, NULL);
	pthread_create(&tid[4], NULL, Pthread_5_udp, NULL);
	
	/*���߳�-���ڽ�������*/
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
	if(0)	//����
	{	
		static int dbgtime=0;
		if(sys_time.sec==0)
			dbgtime++;
		switch(dbgtime){
		case 0:
			break;	
		case 1:
			
			//FTP.LoTimestate|=0x80; 	//������1
			//FTP.LoTimestate&=0xbf;	//������2
			
			FTP.LoTimestate|=0xc0; 	//������
			if(sys_time.sec==59)	//
			FTP.LoTimestate&=0x7f; 	//������
			
			break;	
		case 2:
			
			//if(sys_time.sec==0){
			//	FTP.LoTimestate|=0x80; 	//������1
			//	FTP.LoTimestate&=0xbf;	//������2
			//}
			
			break;	
		case 3:	
			break;	
		default:
			break;
		}
	}
	if(0)	//����ͬ��
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

	if(0)	//���Ծ���ѱ��
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
	
	if(0)	//����ģ���쳣
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
	LOG_MESSAGE log_buf;//д������־buf
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
							dusec=(1000*1000*datalen/BAUDRATE)*10+120000;//�����˴���ʱ��120ms
							usleep(dusec);
							//int nread;
							//ioctl(serial_fd,FIONREAD,&nread);//������fd�л���nread���ַ���Ҫ��ȡ
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
								memcpy(&FTP, buf+6, SERIAL_LENTH);     //���ĸ��Ƶ��ڴ�����ݽṹ��
								mydebug_FTP();
								if(param.rx_serial<2){
									if(param.rx_serial==1){
										sprintf(log_buf.name,"FPGA���ڽ�������");
										add_my_log(log_buf);
									}
									param.rx_serial++;//�޸ļ�¼2017.11.20
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
									if(param.read_param){		//����װ���߼�ǰ�����Զ�ȡ���µ����ã������߼������������
										save_param();			//��file_param��ֵд���ļ�
										read_param();			//�������ļ���ֵ���뵽file_param��ToFpga��ֵ
										dev_console_init();		//��file_param��ֵ����parm
										myprintf("�����ı䣬���¶�ȡ����\n");
										param.read_param = 0;
									}
									Run_BoardState(serialTime);	//���״̬�߼�
									Run_DevData();				//װ�����ݼ���
									comp_clock_offset();		//����6��Դ���Ӳ�ֵ
									Run_DevLog(serialTime);		//װ����־�߼�
									Update_DevLog();			//װ��д�ļ�(��������FLASH��д)
									
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
						read(serial_fd, &buf[1], 300-2);//�����λ
						if(serail_err<3){
							serail_err++;
							sprintf(log_buf.name,"FPGA���ڽ��շ����쳣[%02d�ֽ�](��������%d��)",ret,serail_err);
							add_my_log(log_buf);
						}
						usleep(100*1000);					   //�ӳ�100����
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
	if(fpga_initial_count<5){	//��ʼ�׶β���������Ϣ
		fpga_initial_count++;
		return;	
	}
	deal_config_fpga();        //���ݴ���
	memcpy(ToFpga.Check_head, headbuf, sizeof(headbuf));
	ToFpga.Packet_len = 159;//0xA7;  
 	/*���ڼ���crc*/
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



/*�����͵�fpga������*/
int  deal_config_fpga(void)
{
	//LOG_MESSAGE log_buf;//д������־buf
	static char old_wang_state=0;
	//__u8	FaultLamp = 0;//���ϵ�
	//__u8	WarnLamp = 0;//�澯��
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
	//printf("����Ƶ��1=%d, %x, %x, %x\n",myclog.freque1,ToFpga.summerTime.end.year,ToFpga.summerTime.end.month,ToFpga.summerTime.end.day);
	//printf("����Ƶ��2=%d, %x, %x, %x\n",myclog.freque2,ToFpga.summerTime.end.hour,ToFpga.summerTime.end.min,ToFpga.summerTime.end.second);
	dyndata.P1010_Source_result = 0xff;
	if(param.warn)  dyndata.P1010_SelfCheck |= 0x02;
	if(param.fault) dyndata.P1010_SelfCheck |= 0x01;
	if((dyndata.P1010_SelfCheck&0x03)	!= old_wang_state)
	{
		//sprintf(log_buf.name,"װ�ø澯(����)״̬�����仯: %d-->%d",old_wang_state,dyndata.P1010_SelfCheck&0x03);
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
		if(*i < 4){ 			//��̬��δ����
			if(absoff < d)
				(*i)++;	
			else if((*i)>0)
				(*i)--;
		}
		else {					//��̬�Ѿ�����	
			if(absoff<(d/2)){
				if(*b>0){
					(*b)--;
				}
			}
			else {
				(*b)++; 		
				*off = 0; 		//���Դ˴�żȻ�Ĳ���
			}
			ret=1;
		}
	}
	else{  						 //���ȶ�����>5,��̬�Ѿ��ƻ�
		*i = 0;
		*b = 0;
	}
	//myprintf("off=%d, *i=%d, *b=%d\n",*off,*i,*b);
	return ret;
}



/*У��powerpc��ʱ��*/
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
	/*��ȡ��ǰϵͳʱ��*/
	gettimeofday(&tvppc,NULL);
	  
	/*����ʱ����powerʱ��ʱ�����1s������ϵͳʱ��*/	
	int OFFSET = 10*1000*(SERIAL_LENTH+6+1)/BAUDRATE + 120+4;//������ϵͳ����ʱ��124ms
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
	if(1){//����ʵ��
		if((FTP.LoTimestate&0x80) >>7){	//����ʱ����ʹ�ô���ʱ���޸�ʱ��
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
		myprintf("�޸�ϵͳʱ�䣺ppc_off=%d,sys_time=%d.%d,serialtime=%d\n",  
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
			myprintf("�޸�Ӳ��ʱ�䣺ppc_off=%d,hw=%d.%d,serial_time=%d\n",  
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
	if(leapNum >= 3)//����3������Ԥ��
	{
		
		if((leap61 != 0)&&((tv->tv_sec)%60 == 1))		//�����뷢��
			adj=-1;	//�ظ�һ��
		else if((leap59 != 0)&&((tv->tv_sec)%60 == 59))//�����뷢��
			adj=1;	//����һ��	
		else
			return;
		tv->tv_sec+=adj;
		if(tv->tv_usec<500*1000)
			tv->tv_usec+=500;	//������500us
		settimeofday(tv, (struct timezone*)0);
		myprintf("����(%d)-�޸�ϵͳʱ�� sys_time=%d.%d\n",  adj,(int)tv->tv_sec,(int)tv->tv_usec);		
		if (clock_gettime(clkid, &ts)==0)
		{
			ts.tv_sec = tv->tv_sec;
			ts.tv_nsec = (long)1000*tv->tv_usec;	
			clock_settime(clkid, &ts);
			myprintf("����(%d)-�޸�Ӳ��ʱ�� hw_time=%d.%d\n",  adj,(int)ts.tv_sec,(int)ts.tv_nsec/1000);
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



