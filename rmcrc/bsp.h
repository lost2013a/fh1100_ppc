#ifndef _BSP_H
#define _BSP_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "can_init.h"
#include "serial.h"
#include "config.h"
#include "udp.h"
#include "config.h"
#include "serial.h"
#include "log.h"
#include "gongyong.h"
#include "mycan.h"
#include "msg_lcd.h"

//���ص��Դ�ӡ��Ϣ
#define __MY_PRINTF_ON__ 	//���ص��Դ�ӡ��Ϣ
#define __MY_PROG_UDP  1	//����UDP�¼��������Լ�¼
#define __MY_PROG_LOG  0	//����װ����־�������Լ�¼

//#define __MY_PROG_PTHREAD  	//ϵͳ�߳���־�������Լ�¼



#ifdef  __MY_PRINTF_ON__  
#define myprintf(format,...)\
do{\
	printf("DBGV(%s.c[%03d]): ",__func__,__LINE__);printf(""format"", ##__VA_ARGS__);\
}while(0)
#else  
#define myprintf(format,...)  
#endif  


/*
#ifdef  __MY_PRINTF_ON__  
#define myprintf(format,...) printf(""format"", ##__VA_ARGS__)
#else  
#define myprintf(format,...)  
#endif  
*/
 

/*
__LINE__����ǰԴ�����кţ�

__FILE__����ǰԴ�ļ�����

__DATE__����ǰ�ı�������

__TIME__����ǰ����ʱ�䣻
*/

/*�����ļ��ı���·��*/
#define config_dir 		  	"config"
#define DEV_PARM_FILE		"./config/param_file"
#define DEV_PARM_BACK1_FILE	"./config/param_file_back1"
#define DEV_PARM_BACK2_FILE	"./config/param_file_back2"
#define DEV_BOARD_STA_FILE  "./config/board_state"
#define DEV_VERSIONS_FILE 	"./config/versions_msg.txt"	
#define DEV_LOG_FILE		"/mnt/HD/log/FH1100.log"
#define DEBUG_LOG_FILE  	"/mnt/HD/log"


#define OK 1
#define NO 0
#define EVENT_COUNT 38
#define UDP_COUNT	126
#define MAX_LOGMSG_LEN 256
#define OFF_E_BoardState 61
extern  char* bsp_net[6];
extern pthread_mutex_t lock_log;
extern char leapNum,leap61,leap59;



typedef struct _EVENT_PARAM
{
	char	flag;			//״̬����λ��1����Ч
	//char	flag2;			
	int 	now_state;		//��ǰ״̬
	int   	old_val;		//������ϴε�״̬	
	int		index;			//
	char	type;			//
}EVENT_PARAM;
typedef struct _PARAM
{
	int	man_flag;				//���ӱ�ʶ 0---������1--�ӻ�
	int	mdi_flag;				//���Դ��ʶ 0--���Դ��1--����Դ
	int	bud1;					//������
	int	xiyi1;					//Э��
	int	bad2;
	int	xiyi2;
	int	bcd;
	int	bad_priorty;			//�������ȼ�
	int	gps_priorty;			//GPS���ȼ�
	int	b1_priorty;				//B1���ȼ�
	int	b2_priorty;				//B2���ȼ�
	int	PTP1_priorty;			//PTP1���ȼ�
	int	PTP2_priorty;			//PTP2���ȼ�
	int	bcd_bc;					//BCD����
	int	gps_bc;					//GPS����
	int	irigb1_bc;				//B1����
	int	irigb2_bc;				//B2����
	int	ptp1_bc;				//PTP1����
	int	ptp2_bc;				//PTP2����
	int	time_xz;				//ʱ������ֵ
	int	mc_xz;					//��������ֵ
	int	serial_xz;				//��������ֵ
	int	irigb_xz;				//B������ֵ
	char IP1[4];				//����ip
	unsigned int read_file;
	
	char	read_param;			//�����ļ�������־
	char	initial;			//��ʼ����־
	char	print_serial;		//��ӡ�������ݱ�־
	char	print_can;			//��ӡcan���ݱ�
	char	print_serial_tx;	//��ӡ���͵Ĵ�������
	char	print_prog_log;		//��ӡ״̬��־
	char	warn;				//�澯״̬��
	char	fault;				//����״̬��
	char	tame;				//ѱ��,����b1:(0���쳣1�쳣)b0(0�����ʼ����δ��ɣ�1���)
								//0x1:������0b1x(0x2,0x3)�쳣��0x0����ʼ��״̬
	char	rx_serial;			//�Ƿ��Ѿ��յ���������
	char	dir;				//�Ƿ��Ѿ�����logĿ¼��־
	int		event_mem_id;		//�ڴ����
	int		event_count;		//��¼����
	int		event_file_siet;	//���ļ��е�λ��
	int		writ_event_flag;	//��ʶ��Ҫ���ڴ��е�װ��д���ļ�ϵͳ
	int		writ_event_time;	//����־д���ļ���ʱ��
	EVENT_PARAM	event_param[UDP_COUNT]; //װ�õĵ��
	char	udp_state[UDP_COUNT];
}PARAM;

typedef	struct	_LOG_MESSAGE
{
	char name[MAX_LOGMSG_LEN];		//��־���� 
}LOG_MESSAGE;

typedef struct
{
	unsigned int sec;
	unsigned int nsec;
}TIME_OFFSET;

typedef struct
{
	TIME_OFFSET	BDS;
	TIME_OFFSET	GPS;
	TIME_OFFSET	B1;
	TIME_OFFSET	B2;
	TIME_OFFSET	PTP1;
	TIME_OFFSET	PTP2;
}CLOCK_OFFSET;

typedef  struct {
	long seconds;
	long useconds;
} TimeInternal;

extern SYS_TIME 	 sys_time;		//װ��(ϵͳ)ʱ�䣬���߳�5������£�ֻ�õ��뼶����
extern PPC2FPGA_MSG  ToFpga;		//����FPGA�������ݽṹ
extern FPGA2PPC_MSG  FTP;			//FPGA�Ĵ������ݽ��ջ���
extern FPGA2PPC_MSG  PTI;			//FPGA�Ĵ�������
extern Mclklog 	  	 myclog;		//װ��״̬
extern PARAM		 param;			//װ�ò���,����װ�õ��¼���¼
extern LCD2PPC_MSG   file_param;	//װ�õ����ã��б������ļ���
extern char 		 time_acquired;

extern CLOCK_OFFSET  clock_offset;	//6��ʱ��Դ�������Ӳ�ֵ
extern PPS_SECOND    pps_second;	//���غ�6��ʱ��Դ������뼶ʱ���(�����1970��)
extern LOSE_LOCK	 lose_lock;		//�ź�ʧ����ʱ���¼

extern unsigned int  bard_state_udp[16];

extern int	open_log_flag;	//���Լ�¼ͬ������
extern int	prog_log_count;	//���Լ�¼����
extern int board_type_its[0x20];

int create_config_dir(const char *dir);
int is_dir_exist(const char *dir_path);
void dev_console_init(void);
void main_initial(char set_eth);
void set_initial(char all_untime);
void read_time(void);
int CreateDir(void);  
int	read_board_state(void);
int save_board_state(void);
void create_log(int init);
void add_my_log(LOG_MESSAGE log_message);
void save_my_log(void);

void subTime(TimeInternal *r,TimeInternal *x,TimeInternal *y);
void direct_write(FILE *fp,const char *type);
#endif