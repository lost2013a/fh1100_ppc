#ifndef __LOG_H
#define __LOG_H

#include <linux/types.h>
#include "serial.h"


#include <stdio.h>
#include <string.h>

#define	 OK 			1
#define	 MAX_LINE 		1024


												
typedef struct _UDP_EVENT
{
	int				index;
	int				type;
	int				val;
	SYS_TIME		time;
}UDP_EVENT;											
													
													
typedef struct 
{
	char op_flag;
	int  event_count;
	int  event_list[100];
}COM_EVENT;													
													
extern COM_EVENT com_event;													
extern RECORD mem_record[LOG_MAX_COUNT];
extern int	  temp_record_count;			

void event_from_lcd(int id);	//生成事件的(节点号)向量表

int Run_DevData(void);
int Run_BoardState(Time serialTime);
int Run_DevLog(Time serialTime);
int Update_DevLog(void);	
void comp_clock_offset(void);		//计算6个源的钟差值
int read_log_flie(void);
#endif