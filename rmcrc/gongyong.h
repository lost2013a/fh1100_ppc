#ifndef __GONGYONG_H
#define __GONGYONG_H
#include <linux/types.h>
#include <semaphore.h>
#define	 LOG_MAX_COUNT	200			//最大日志记录条数
#define	 LOG_RECORD_LEN	100			//一个字节的记录长度


//获取文件的编译时间
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



typedef struct 
{
	unsigned char year[2];
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char min;
	unsigned char second;
}CTime;

typedef struct 
{
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char min;
	unsigned char second;
}XTime;


typedef struct 
{ 
	union year
	{			
		int year;		
		unsigned char cyear[2];		
	}y;
	union month
	{
		int month;
		unsigned char cmonth;
	}mo;
	union day
	{
		int day;
		unsigned char cday;
	}d;
	union hour
	{
		int hour;
		unsigned char chour;
	}h;
	union min
	{
		int min;
		unsigned char cmin;
	}m;
	union second
	{
		int second;
		unsigned char csecond;
	}s;
}Time;




//-------------------------------------------------------------------



												          
typedef struct _SYS_TIME
{
	volatile unsigned short year;
	volatile unsigned char  mon;
	volatile unsigned char  day;
	volatile unsigned char  hour;
	volatile unsigned char  min;
	volatile unsigned char  sec;
	volatile unsigned short msec;
	volatile unsigned long  usec;
	volatile unsigned long  total_sec;
}SYS_TIME;												          

typedef struct _RECORD
{
	int			mem_id;		//事件号
	SYS_TIME	warn_time;	//告警时间
	char		index;		//序号
	char		value;		//值
	char      	old_val;	//上一次的值
}RECORD;






#endif
