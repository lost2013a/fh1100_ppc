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

//开关调试打印信息
#define __MY_PRINTF_ON__ 	//开关调试打印信息
#define __MY_PROG_UDP  1	//开关UDP事件生产调试记录
#define __MY_PROG_LOG  0	//开关装置日志生产调试记录

//#define __MY_PROG_PTHREAD  	//系统线程日志生产调试记录



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
__LINE__：当前源代码行号；

__FILE__：当前源文件名；

__DATE__：当前的编译日期

__TIME__：当前编译时间；
*/

/*参数文件的保存路径*/
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
	char	flag;			//状态控制位，1：有效
	//char	flag2;			
	int 	now_state;		//当前状态
	int   	old_val;		//保存的上次的状态	
	int		index;			//
	char	type;			//
}EVENT_PARAM;
typedef struct _PARAM
{
	int	man_flag;				//主从标识 0---主机，1--从机
	int	mdi_flag;				//多机源标识 0--多机源，1--单机源
	int	bud1;					//波特率
	int	xiyi1;					//协议
	int	bad2;
	int	xiyi2;
	int	bcd;
	int	bad_priorty;			//北斗优先级
	int	gps_priorty;			//GPS优先级
	int	b1_priorty;				//B1优先级
	int	b2_priorty;				//B2优先级
	int	PTP1_priorty;			//PTP1优先级
	int	PTP2_priorty;			//PTP2优先级
	int	bcd_bc;					//BCD补偿
	int	gps_bc;					//GPS补偿
	int	irigb1_bc;				//B1补偿
	int	irigb2_bc;				//B2补偿
	int	ptp1_bc;				//PTP1补偿
	int	ptp2_bc;				//PTP2补偿
	int	time_xz;				//时钟修正值
	int	mc_xz;					//脉冲修正值
	int	serial_xz;				//串口修正值
	int	irigb_xz;				//B码修正值
	char IP1[4];				//本机ip
	unsigned int read_file;
	
	char	read_param;			//配置文件保护标志
	char	initial;			//初始化标志
	char	print_serial;		//打印串口数据标志
	char	print_can;			//打印can数据标
	char	print_serial_tx;	//打印发送的串口数据
	char	print_prog_log;		//打印状态日志
	char	warn;				//告警状态字
	char	fault;				//故障状态字
	char	tame;				//驯服,晶振：b1:(0无异常1异常)b0(0晶振初始化还未完成；1完成)
								//0x1:正常。0b1x(0x2,0x3)异常。0x0，初始化状态
	char	rx_serial;			//是否已经收到串口数据
	char	dir;				//是否已经建好log目录标志
	int		event_mem_id;		//内存序号
	int		event_count;		//记录条数
	int		event_file_siet;	//在文件中的位置
	int		writ_event_flag;	//标识需要把内存中的装置写入文件系统
	int		writ_event_time;	//把日志写成文件的时间
	EVENT_PARAM	event_param[UDP_COUNT]; //装置的点表
	char	udp_state[UDP_COUNT];
}PARAM;

typedef	struct	_LOG_MESSAGE
{
	char name[MAX_LOGMSG_LEN];		//日志类型 
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

extern SYS_TIME 	 sys_time;		//装置(系统)时间，由线程5负责更新，只用到秒级精度
extern PPC2FPGA_MSG  ToFpga;		//发往FPGA串口数据结构
extern FPGA2PPC_MSG  FTP;			//FPGA的串口数据接收缓存
extern FPGA2PPC_MSG  PTI;			//FPGA的串口数据
extern Mclklog 	  	 myclog;		//装置状态
extern PARAM		 param;			//装置参数,包含装置的事件记录
extern LCD2PPC_MSG   file_param;	//装置的配置，有保存在文件中
extern char 		 time_acquired;

extern CLOCK_OFFSET  clock_offset;	//6个时间源计算后的钟差值
extern PPS_SECOND    pps_second;	//本地和6个时间源换算的秒级时间戳(相对与1970年)
extern LOSE_LOCK	 lose_lock;		//信号失锁的时间记录

extern unsigned int  bard_state_udp[16];

extern int	open_log_flag;	//调试记录同步机制
extern int	prog_log_count;	//调试记录计数
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