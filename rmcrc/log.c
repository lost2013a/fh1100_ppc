#include <time.h>
#include <pthread.h>
#include "bsp.h"



#define LOG_WRITE_TIMEC 30

int	      now_mem_set;					//当前存储位置
COM_EVENT com_event;					//普通日志记录表，指和装置参数状态无关的日志		
RECORD    mem_record[LOG_MAX_COUNT];	//装置日志(内存中的结构)
RECORD 	  temp_record[LOG_MAX_COUNT];	//装置日志的缓存
int	   	  temp_record_count;			//装置日志缓存的日志条数


static void add_event(int id,Time serialTime);	//将发生的事件放入日志缓存中


const char event_name[100][100] = {
	{"北斗信号  "},											//---0
	{"GPS信号  "},											//--1
	{"IRIG-B1码输入信号  "},						//--2
	{"IRIG-B2码输入信号  "},						//--3
	{"PTP1输入信号  "},									//--4
	{"PTP2输入信号  "},									//--5
	{"北斗天线状态  "},									//--6
	{"GPS天线状态  "},									//--7
	{"北斗卫星接收模块状态  "},					//--8
	{"GPS卫星接收模块状态  "},					//--9
	{"时间跳变侦测状态  "},							//--10
	{"BDS时间跳变侦测状态  "},					//--11
	{"GPS时间跳变侦测状态  "},					//--12
	{"IRIG_B1时间跳变侦测状态  "},			//--13
	{"IRIG_B2时间跳变侦测状态  "},			//--14
	{"PTP1源时间跳变侦测状态  "},				//--15
	{"PTP2源时间跳变侦测状态  "},				//--16
	{"晶振驯服状态  "},									//--17
	{"初始化状态  "},										//--18
	{"电源模块状态  "},									//--19
	{"P1电源模块状态  "},								//--20
	{"P2电源模块状态  "},								//--21
	{"IRIG-B码输入质量低于本机  "},			//--22
	{"所有独立时源不可用小于30分钟（首次同步）  "},		//--23
	{"所有独立时源不可用小于24小时（曾经同步过）  "},	//--24
	{"其它可恢复故障                              "},	//--25
	{"CPU核心板异常                               "},	//--26
	{"晶振驯服状态异常超过60秒                    "},	//--27
	{"所有独立时源不可用超过30分钟（首次同步）  "},		//--28
	{"所有独立时源不可用超过24小时（曾经同步过）  "},	//--29
	{"其它严重故障   "},		//--30
	{"闰秒预告  "},					//--31
	{"BDS闰秒预告  "},			//--32
	{"GPS闰秒预告  "},			//--33
	{"IRIG_B1闰秒预告  "},	//--34
	{"IRIG_B2闰秒预告  "},	//--35
	{"闰秒标志变化     "},	//--36
	{"时钟源发生变化	 "},	//--37
	{"PTP1优先级变化 "},	//--38
	{"PTP1优先级变化 "},	//--39
	{"               "},	//--40
	{"               "},	//--41
	{"GPS优先级变化  "},	//--42
	{"BDS优先级变化  "},	//--43
	{"B1优先级变化   "},	//--44
	{"B2优先级变化   "},	//--45
	{"               "},	//--46
	{"               "},	//--47
	{"               "},	//--48
	{"               "},	//--49
	{"               "},	//--50
	{"               "},	//--51
	{"               "},	//--52
	{"               "},	//--53
	{"               "},	//--54
	{"               "},	//--55
	{"               "},	//--56
	{"               "},	//--57
	{"               "},	//--58
	{"用户登录       "},	 //--59
	{"               "},	//--60
	{"槽位1板卡      "},	 //-61	OFF_E_BoardState
	{"               "},	//--62
	{"槽位2板卡      "},	 //-63
	{"               "},	//--64
	{"槽位3板卡      "},	 //-65
	{"               "},	//--66
	{"槽位4板卡      "},	 //-67
	{"               "},	//--68
	{"槽位5板卡      "},	 //-69
	{"               "},	//--70
	{"槽位6板卡      "},	 //-71
	{"               "},	//--72
	{"槽位7板卡      "},	 //-73
	{"               "},	//--74
	{"槽位8板卡      "},	 //-75
	{"               "},	//--76
	{"槽位9板卡      "},	 //-77
	{"               "},	//--78
	{"槽位10板卡     "},	 //-79
	{"               "},	//--80
	{"槽位11板卡     "},	 //-81
	{"               "},	//--82
	{"槽位12板卡     "},	 //-83
	{"               "},	//--84
	{"槽位13板卡     "},	 //-85
	{"               "},	//--86
	{"槽位14板卡     "},	 //-87
	{"               "},	//--88
	{"槽位15板卡     "},	 //-89
	{"               "},	//--90
};


int board_type_its[0x20]={
0,//0x00	
0x00020410,//0x01
0,//0x02
0x00020835,//0x03
0x00000836,//0x04
0,//0x05
0x00000837,//0x06
0x00000838,//0x07
0,//0x08
0,//0x09
0,//0x0a
0,//0x0b
0,//0x0c
0,//0x0d
0,//0x0e
0,//0x0f
0,//0x10
0x0002040f,//0x11
0x0002020b,//0x12
0x00100017,//0x13
0x00080013,//0x14
0x00080014,//0x15
0x00100027,//0x16
0x0010001f,//0x17
0x0010001b,//0x18
0x00100033,//0x19
0x00040234,//0x1a
0x0005022b,//0x1b
0x0000020c,//0x1c
0x00000101,//0x1d
0x00000102,//0x1e
0x00000000,//0x1f
};

/*
void *Pthread_2_devlog(void *arg)
{
	sem_wait(&sem_serial_accept);
	printf("Fpga Serial OK\n");
	usleep(1500000);//延迟1.5秒，丢掉第一帧串口数据
	
	param.rx_serial=1;//修改记录2017.11.20
	while(1)
	{	 
		sem_wait(&sem_serial_accept);
		compare_log();
		sem_post(&sem_can_dy_send);	
	}	
	pthread_exit(NULL);
}
*/

void add_event(int id,Time serialTime)	//将发生的事件放入日志缓存中
{
	temp_record[temp_record_count].mem_id = param.event_mem_id;
	temp_record[temp_record_count].warn_time.year = serialTime.y.year;
	temp_record[temp_record_count].warn_time.mon = serialTime.mo.month;
	temp_record[temp_record_count].warn_time.day = serialTime.d.day;
	temp_record[temp_record_count].warn_time.hour= serialTime.h.hour;
	temp_record[temp_record_count].warn_time.min = serialTime.m.min;
	temp_record[temp_record_count].warn_time.sec = serialTime.s.second;
	temp_record[temp_record_count].index = id+1;
	
	temp_record[temp_record_count].value = param.event_param[id].now_state;
	temp_record[temp_record_count].old_val = param.event_param[id].old_val;
	if(id>=OFF_E_BoardState && id<(OFF_E_BoardState+14*2)){
		if(param.event_param[id].now_state==0x1f){
			temp_record[temp_record_count].value=1;
			temp_record[temp_record_count].old_val=0;
		}
		else{
			temp_record[temp_record_count].value=0;
			temp_record[temp_record_count].old_val=1;
		}
	}
	
	temp_record_count++;	//缓存条数+1
	param.event_mem_id++;
	return;
}
//add_comm_vent
void event_from_lcd(int id)	 //人机界面设置参数产生Comm日志记录表
{
	while(com_event.op_flag) //标志位不可用，阻塞
		usleep(1000);
	com_event.op_flag = 1;
	com_event.event_list[com_event.event_count] = id;
	com_event.event_count++;
	com_event.op_flag=0;	 //恢复可用标志位
}

void Handle_CommEvent(void)			//转化Comm日志记录表，产生输出(装置日志/UDP事件)
{
	int i;
	Time now_time;
	now_time.y.year 	= sys_time.year;
	now_time.mo.month 	= sys_time.mon;
	now_time.d.day 		= sys_time.day;
	now_time.h.hour 	= sys_time.hour;
	now_time.m.min 		= sys_time.min;
	now_time.s.second	= sys_time.sec;
	while(com_event.op_flag)  //标志位不可用，阻塞
		usleep(1000);
	com_event.op_flag = 1;
	for(i=0;i<com_event.event_count;i++){
		add_event(com_event.event_list[i],now_time);	//产生装置日志
		add_event_udp(com_event.event_list[i],now_time);//产生装置UDP事件
	}
	com_event.event_count=0;
	com_event.op_flag = 0;
}

int	write_DevLog(void)	//把装置日志(缓存在内存)写入文件系统
{
	LOG_MESSAGE log_buf;//写调试日志buf
	int i,j;
	int event_id;
	char buf[LOG_RECORD_LEN];
	char file_buf[LOG_RECORD_LEN*LOG_MAX_COUNT];
	FILE *fp;
	for(event_id=0;event_id<param.event_count;event_id++)
	{
		for(i=0;i<LOG_RECORD_LEN;i++)	//先清0
			buf[i]=' ';
		sprintf(buf,"%03d,%02d,%02d,%02d,%04d-%02d-%02d %02d:%02d:%02d,",
				event_id+1,			//mem_record[event_id].mem_id%1000,
				mem_record[event_id].index%100,
				mem_record[event_id].value%100,
				mem_record[event_id].old_val%100,
				mem_record[event_id].warn_time.year%10000,
				mem_record[event_id].warn_time.mon%100,
				mem_record[event_id].warn_time.day%100,
				mem_record[event_id].warn_time.hour%100,
				mem_record[event_id].warn_time.min%100,
				mem_record[event_id].warn_time.sec%100);
		i=0;
		j=33;
		do{
			buf[i+j] = event_name[mem_record[event_id].index-1][i];
			i++;
		}while(event_name[mem_record[event_id].index-1][i]!=' ' && (i+j)<=(LOG_RECORD_LEN-2));
		buf[LOG_RECORD_LEN-2]=0x0a;
		buf[LOG_RECORD_LEN-1]=0x0d;
		memcpy(&file_buf[event_id*LOG_RECORD_LEN],&buf,LOG_RECORD_LEN);
	}	
	if(param.dir==0)//检查"/mnt/HD/log"文件
		return -1;
	if((fp = fopen(DEV_LOG_FILE,"w")) == NULL){
		sprintf(log_buf.name,"写日志时打开文件失败,初始化日志文件");
		add_my_log(log_buf);
		param.event_mem_id=0;
		param.event_count=0;
		param.event_file_siet=0;
		if((fp = fopen(DEV_LOG_FILE,"w")) == NULL){
			sprintf(log_buf.name,"写日志时建立文件失败(2)，放弃文件保存");
			add_my_log(log_buf);
			return -1;
		}
	}	
	int ret = fwrite(file_buf,1,param.event_count*LOG_RECORD_LEN,fp);
	if(ret!=param.event_count*LOG_RECORD_LEN){
		sprintf(log_buf.name,"写日志时文件写入失败,初始化日志文件 ret=%d, length=%d",ret,param.event_count*LOG_RECORD_LEN);
		add_my_log(log_buf);
		fclose(fp);
		printf("%s\n",log_buf.name);		
		return -2;
	}
	fseek(fp,0L,SEEK_END);
	int file_len=ftell(fp);	//读出文件长度(距离0L位置)
	if(file_len!=ret || (file_len%LOG_RECORD_LEN)!=0){
		sprintf(log_buf.name,"写日志后长度错误:ret=%d, length=%d",ret,file_len);
		add_my_log(log_buf);
	}	

	/*
	struct timeval	tv1,tv2,tvr;
	struct timezone tz;
	gettimeofday(&tv1,&tz);
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
	gettimeofday(&tv2,&tz);
	subTime( (TimeInternal*)&tvr, ( TimeInternal*)&tv2, ( TimeInternal*) &tv1);
	myprintf("LOG_savetime=%ld.%ld\n",tvr.tv_sec,tvr.tv_usec);
	*/
	direct_write(fp,"write_devlog");
	
	
	param.writ_event_flag = 0;	//清掉写标志
	param.writ_event_time = 0;	//清掉计时
	return 0;
}


int save_DevLog()//把产生的日志更新到装置日志(缓存在内存)，并标识可写入文件
{
	LOG_MESSAGE log_buf;//写调试日志buf
	int i,j,res;
	if(temp_record_count==0)							  //没有需要存的，直接退出
		return -1;
	res=temp_record_count+param.event_count-LOG_MAX_COUNT;//需要移动的条数
	if(res>0){											  //＞0，需要移
		for(j=0;j<LOG_MAX_COUNT-res;j++){
			mem_record[j].mem_id = mem_record[j+res].mem_id;
			mem_record[j].index  = mem_record[j+res].index;
			mem_record[j].value  = mem_record[j+res].value;
			mem_record[j].old_val= mem_record[j+res].old_val;
			mem_record[j].warn_time.year  = mem_record[j+res].warn_time.year;
			mem_record[j].warn_time.mon   = mem_record[j+res].warn_time.mon;
			mem_record[j].warn_time.day   = mem_record[j+res].warn_time.day;
			mem_record[j].warn_time.hour  = mem_record[j+res].warn_time.hour;
			mem_record[j].warn_time.min   = mem_record[j+res].warn_time.min;
			mem_record[j].warn_time.sec   = mem_record[j+res].warn_time.sec;
		}
	}
	if(param.event_count<LOG_MAX_COUNT){
			param.event_count+=temp_record_count;
			if(param.event_count>LOG_MAX_COUNT)
				param.event_count=LOG_MAX_COUNT;
	}
	for(i=0;i<temp_record_count;i++)					  //填写新插入的日志
	{		
		mem_record[param.event_count+i-temp_record_count].mem_id = temp_record[i].mem_id;	
		mem_record[param.event_count+i-temp_record_count].index  = temp_record[i].index;	
		mem_record[param.event_count+i-temp_record_count].value  = temp_record[i].value;	
		mem_record[param.event_count+i-temp_record_count].old_val= temp_record[i].old_val;
		mem_record[param.event_count+i-temp_record_count].warn_time.year = temp_record[i].warn_time.year;	
		mem_record[param.event_count+i-temp_record_count].warn_time.mon  = temp_record[i].warn_time.mon;	
		mem_record[param.event_count+i-temp_record_count].warn_time.day  = temp_record[i].warn_time.day;	
		mem_record[param.event_count+i-temp_record_count].warn_time.hour = temp_record[i].warn_time.hour;	
		mem_record[param.event_count+i-temp_record_count].warn_time.min  = temp_record[i].warn_time.min;	
		mem_record[param.event_count+i-temp_record_count].warn_time.sec  = temp_record[i].warn_time.sec;
		/*if(0){
			printf("存日志：%d,%d,%d,time=%d:%d:%d\n",
				mem_record[param.event_count+i-res].mem_id,
				mem_record[param.event_count+i-res].index,
				mem_record[param.event_count+i-res].value,
				mem_record[param.event_count+i-res].warn_time.hour,
				mem_record[param.event_count+i-res].warn_time.min,
				mem_record[param.event_count+i-res].warn_time.sec);		
		}
		*/
		if(__MY_PROG_LOG){
			sprintf(log_buf.name," 1 Event_LOG[%03d]: val=%02d",temp_record[i].index,
				temp_record[i].value);
			add_my_log(log_buf);	
		}
	}
	param.writ_event_flag = 1;		//文件可写的标志位
	return 0;
}

void comp_clock_offset(void)		//计算6个源的钟差值
{
	//LOG_MESSAGE log_buf;//写调试日志buf
	int iTemp;
	char neg_flag;
	//unsigned int pps_errval_sec;
	//unsigned int pps_errval_nsec;
	long long PPS_errval;
	if(param.event_param[100].flag==0 || param.event_param[101].flag==0){
		clock_offset.BDS.sec  = 0;	//输出延迟或者BD输入延迟无效,值=0
		clock_offset.BDS.nsec = 0;
	}	
	else if((FTP.BDS.source_syn_state&SS_SYNC)==0){
		iTemp = 256*(256*(256*(FTP.BDS.ppsErrval[0]&0x7f)+FTP.BDS.ppsErrval[1])+FTP.BDS.ppsErrval[2])+FTP.BDS.ppsErrval[3];
		if(FTP.BDS.ppsErrval[0]&0x80)
			iTemp = -iTemp*PPS_K;
		else													
			iTemp = iTemp*PPS_K;
		PPS_errval = (long long)1000000000*(pps_second.bds_total_sec-pps_second.locate_total_sec+1)+iTemp;
		if(PPS_errval<0) {
			PPS_errval = -PPS_errval;
			neg_flag = 1;	//符号为负
		}	
		else{
			neg_flag = 0;
		}
		clock_offset.BDS.sec = (long long)PPS_errval/1000000000;
		clock_offset.BDS.nsec= (long long)PPS_errval%1000000000;
		if(neg_flag){		//负数，最高位为1.
			clock_offset.BDS.sec |= 0x80000000;
			clock_offset.BDS.nsec |= 0x80000000;
		}
	}
	else{							//未同步，按南网要求输出0x7fffffff
		clock_offset.BDS.sec =  0x7fffffff;
		clock_offset.BDS.nsec = 0x7fffffff;
	}
			
	if(param.event_param[102].flag==0 || param.event_param[103].flag==0)
	{
		clock_offset.GPS.sec =  0;
		clock_offset.GPS.nsec = 0;
	}		
	else if((FTP.GPS.source_syn_state&SS_SYNC)==0)
	{
		iTemp = 256*(256*(256*(FTP.GPS.ppsErrval[0]&0x7f)+FTP.GPS.ppsErrval[1])+FTP.GPS.ppsErrval[2])+FTP.GPS.ppsErrval[3];
		if(FTP.GPS.ppsErrval[0]&0x80) iTemp = -iTemp*PPS_K;
		else													iTemp = iTemp*PPS_K;
		PPS_errval = (long long)1000000000*(pps_second.gps_total_sec-pps_second.locate_total_sec+1)+iTemp;
		if(PPS_errval<0) 
		{
			PPS_errval = -PPS_errval;
			neg_flag = 1;
		}	
		else
			neg_flag = 0;
		clock_offset.GPS.sec = (long long)PPS_errval/1000000000;
		clock_offset.GPS.nsec = (long long)PPS_errval%1000000000;
		if(neg_flag)
		{
			clock_offset.GPS.sec |= 0x80000000;
			clock_offset.GPS.nsec |= 0x80000000;
		}
	}
	else
	{
		clock_offset.GPS.sec =  0x7fffffff;
		clock_offset.GPS.nsec = 0x7fffffff;
	}	
	
	if(param.event_param[104].flag==0 ||param.event_param[105].flag==0)
	{
		clock_offset.B1.sec =  0;
		clock_offset.B1.nsec = 0;
	}		
	else if((FTP.IRIG_B1.source_syn_state&SS_SYNC)==0)
	{
		iTemp = 256*(256*(256*(FTP.IRIG_B1.ppsErrval[0]&0x7f)+FTP.IRIG_B1.ppsErrval[1])+FTP.IRIG_B1.ppsErrval[2])+FTP.IRIG_B1.ppsErrval[3];
		if(FTP.IRIG_B1.ppsErrval[0]&0x80) iTemp = -iTemp*PPS_K;
		else													iTemp = iTemp*PPS_K;
		PPS_errval = (long long)1000000000*(pps_second.b1_total_sec-pps_second.locate_total_sec+1)+iTemp;
		if(PPS_errval<0) 
		{
			PPS_errval = -PPS_errval;
			neg_flag = 1;
		}	
		else
			neg_flag = 0;
		clock_offset.B1.sec = (long long)PPS_errval/1000000000;
		clock_offset.B1.nsec = (long long)PPS_errval%1000000000;
		
		if(neg_flag)
		{
			clock_offset.B1.sec |= 0x80000000;
			clock_offset.B1.nsec |= 0x80000000;
		}
	}
	else
	{
		clock_offset.B1.sec =  0x7fffffff;
		clock_offset.B1.nsec = 0x7fffffff;
	}	
	
	if(param.event_param[106].flag==0 || param.event_param[107].flag==0)
	{
		clock_offset.B2.sec =  0;
		clock_offset.B2.nsec = 0;
	}
	else if((FTP.IRIG_B2.source_syn_state&SS_SYNC)==0)
	{
		iTemp = 256*(256*(256*(FTP.IRIG_B2.ppsErrval[0]&0x7f)+FTP.IRIG_B2.ppsErrval[1])+FTP.IRIG_B2.ppsErrval[2])+FTP.IRIG_B2.ppsErrval[3];
		if(FTP.IRIG_B2.ppsErrval[0]&0x80) iTemp = -iTemp*PPS_K;
		else															iTemp = iTemp*PPS_K;
		PPS_errval = (long long)1000000000*(pps_second.b2_total_sec-pps_second.locate_total_sec+1)+iTemp;
		if(PPS_errval<0) 
		{
			PPS_errval = -PPS_errval;
			neg_flag = 1;
		}	
		else
			neg_flag = 0;
		clock_offset.B2.sec = (long long)PPS_errval/1000000000;
		clock_offset.B2.nsec = (long long)PPS_errval%1000000000;
		
		if(neg_flag)
		{
			clock_offset.B2.sec |= 0x80000000;
			clock_offset.B2.nsec |= 0x80000000;
		}
	}
	else
	{
		clock_offset.B2.sec =  0x7fffffff;
		clock_offset.B2.nsec = 0x7fffffff;
	}	
	
	if(param.event_param[108].flag==0 || param.event_param[109].flag==0)
	{
		clock_offset.PTP1.sec =  0;
		clock_offset.PTP1.nsec = 0;
	}	
	else if((FTP.PTP1.source_syn_state&SS_SYNC)==0)
	{
		iTemp = 256*(256*(256*(FTP.PTP1.ppsErrval[0]&0x7f)+FTP.PTP1.ppsErrval[1])+FTP.PTP1.ppsErrval[2])+FTP.PTP1.ppsErrval[3];
		if(FTP.PTP1.ppsErrval[0]&0x80) iTemp = -iTemp*PPS_K;
		else																iTemp = iTemp*PPS_K;
		PPS_errval = (long long)1000000000*(pps_second.ptp1_total_sec-pps_second.locate_total_sec+1)+iTemp;
		if(PPS_errval<0) 
		{
			PPS_errval = -PPS_errval;
			neg_flag = 1;
		}	
		else
			neg_flag = 0;
		clock_offset.PTP1.sec = (long long)PPS_errval/1000000000;
		clock_offset.PTP1.nsec = (long long)PPS_errval%1000000000;
		
		if(neg_flag)
		{
			clock_offset.PTP1.sec |= 0x80000000;
			clock_offset.PTP1.nsec |= 0x80000000;
		}
	}
	else
	{
		clock_offset.PTP1.sec =  0x7fffffff;
		clock_offset.PTP1.nsec = 0x7fffffff;
	}	
	
	if(param.event_param[110].flag==0 || param.event_param[111].flag==0)
	{	
		clock_offset.PTP2.sec =  0;
		clock_offset.PTP2.nsec = 0;
	}	
	else if((FTP.PTP2.source_syn_state&SS_SYNC)==0)
	{
		iTemp = 256*(256*(256*(FTP.PTP2.ppsErrval[0]&0x7f)+FTP.PTP2.ppsErrval[1])+FTP.PTP2.ppsErrval[2])+FTP.PTP2.ppsErrval[3];
		if(FTP.PTP2.ppsErrval[0]&0x80) iTemp = -iTemp*PPS_K;
		else																iTemp = iTemp*PPS_K;
		PPS_errval = (long long)1000000000*(pps_second.ptp2_total_sec-pps_second.locate_total_sec+1)+iTemp;
		if(PPS_errval<0) 
		{
			PPS_errval = -PPS_errval;
			neg_flag = 1;
		}	
		else
			neg_flag = 0;
		clock_offset.PTP2.sec = (long long)PPS_errval/1000000000;
		clock_offset.PTP2.nsec = (long long)PPS_errval%1000000000;
		
		if(neg_flag)
		{
			clock_offset.PTP2.sec |= 0x80000000;
			clock_offset.PTP2.nsec |= 0x80000000;
		}
	}
	else
	{
		clock_offset.PTP2.sec =  0x7fffffff;
		clock_offset.PTP2.nsec = 0x7fffffff;
	}
	//调试用:FPGA分析跳变问题 2017.11.21
}




int Run_DevData(void) //计算频率和钟差等
{
	struct tm  calTime;
	int cycle1,freque;
	if(param.initial==0)	//初始化还未完成(不会产生日志)
		return -1;
	cycle1 = ((FTP.Channel1.frequency[0]*256+FTP.Channel1.frequency[1])*256+FTP.Channel1.frequency[2])*256+FTP.Channel1.frequency[3];
	freque = 500000000000/cycle1;
	if(freque<610000 && freque>390000)
		myclog.freque1 = freque;
	//int cycle2 = ((FTP.Channel2.frequency[0]*256+FTP.Channel2.frequency[1])*256+FTP.Channel2.frequency[2])*256+FTP.Channel2.frequency[3];
	freque = 500000000000/cycle1;
	if(freque<610000 && freque>390000)
		myclog.freque2 = freque;	
	if(((FTP.BDS.source_syn_state&SS_CONTINUITY)>>2) ||
		 ((FTP.GPS.source_syn_state&SS_CONTINUITY)>>2) ||
		 ((FTP.IRIG_B1.source_syn_state&SS_CONTINUITY)>>2) ||
		 ((FTP.IRIG_B2.source_syn_state&SS_CONTINUITY)>>2))
		FTP.LoTimeAbnormal |= 0x01;		//生成总跳变信号
	else
		FTP.LoTimeAbnormal &= 0xfe;	 	//生成总跳变信号=0（没有跳变）
	calTime.tm_year = (FTP.Locatetime.year[0]<<8)+FTP.Locatetime.year[1]-1900;
	calTime.tm_mon  = FTP.Locatetime.month-1;
	calTime.tm_mday = FTP.Locatetime.day;
	calTime.tm_hour = FTP.Locatetime.hour;
	calTime.tm_min  = FTP.Locatetime.min;
	calTime.tm_sec  = FTP.Locatetime.second;
	pps_second.locate_total_sec =mktime(&calTime)-1;
	
	//if(file_param.control.global_control1&0x01 && file_param.control.bds_control&0x20)
	if(param.event_param[0].flag && file_param.control.bds_control&0x20)
	{	
		calTime.tm_year = (FTP.BDS.time.year[0]<<8)+FTP.BDS.time.year[1]-1900;
		calTime.tm_mon  = FTP.BDS.time.month-1;
		calTime.tm_mday = FTP.BDS.time.day;
		calTime.tm_hour = FTP.BDS.time.hour;
		calTime.tm_min  = FTP.BDS.time.min;
		calTime.tm_sec  = FTP.BDS.time.second;
		pps_second.bds_total_sec = mktime(&calTime)-1;
	}
	else
	{
		pps_second.bds_total_sec = 0;
		FTP.BDS.ppsErrval[0] = 0;
		FTP.BDS.ppsErrval[1] = 0;
		FTP.BDS.ppsErrval[2] = 0;
		FTP.BDS.ppsErrval[3] = 0;
	}	
	
	//if(file_param.control.global_control1&0x02 && file_param.control.gps_control&0x20)
	if(param.event_param[1].flag && file_param.control.gps_control&0x20)
	{	
		calTime.tm_year = (FTP.GPS.time.year[0]<<8)+FTP.GPS.time.year[1]-1900;
		calTime.tm_mon  = FTP.GPS.time.month-1;
		calTime.tm_mday = FTP.GPS.time.day;
		calTime.tm_hour = FTP.GPS.time.hour;
		calTime.tm_min  = FTP.GPS.time.min;
		calTime.tm_sec  = FTP.GPS.time.second;
		pps_second.gps_total_sec = mktime(&calTime)-1;
	}
	else
	{
		pps_second.gps_total_sec = 0;
		FTP.GPS.ppsErrval[0] = 0;
		FTP.GPS.ppsErrval[1] = 0;
		FTP.GPS.ppsErrval[2] = 0;
		FTP.GPS.ppsErrval[3] = 0;
	}
	
	//if(file_param.control.global_control1&0x04 && file_param.control.b1_control&0x20)
	if(param.event_param[2].flag && file_param.control.b1_control&0x20)
	{	
		calTime.tm_year = (FTP.IRIG_B1.time.year[0]<<8)+FTP.IRIG_B1.time.year[1]-1900;
		calTime.tm_mon  = FTP.IRIG_B1.time.month-1;
		calTime.tm_mday = FTP.IRIG_B1.time.day;
		calTime.tm_hour = FTP.IRIG_B1.time.hour;
		calTime.tm_min  = FTP.IRIG_B1.time.min;
		calTime.tm_sec  = FTP.IRIG_B1.time.second;
		pps_second.b1_total_sec = mktime(&calTime)-1;
	}
	else
	{
		pps_second.b1_total_sec = 0;
		FTP.IRIG_B1.ppsErrval[0] = 0;
		FTP.IRIG_B1.ppsErrval[1] = 0;
		FTP.IRIG_B1.ppsErrval[2] = 0;
		FTP.IRIG_B1.ppsErrval[3] = 0;
	}
	
	//if(file_param.control.global_control1&0x08 && file_param.control.b2_control&0x20)
	if(param.event_param[3].flag && file_param.control.b2_control&0x20)
	{	
		calTime.tm_year = (FTP.IRIG_B2.time.year[0]<<8)+FTP.IRIG_B2.time.year[1]-1900;
		calTime.tm_mon  = FTP.IRIG_B2.time.month-1;
		calTime.tm_mday = FTP.IRIG_B2.time.day;
		calTime.tm_hour = FTP.IRIG_B2.time.hour;
		calTime.tm_min  = FTP.IRIG_B2.time.min;
		calTime.tm_sec  = FTP.IRIG_B2.time.second;
		pps_second.b2_total_sec = mktime(&calTime)-1;
	}
	else
	{
		pps_second.b2_total_sec = 0;
		FTP.IRIG_B2.ppsErrval[0] = 0;
		FTP.IRIG_B2.ppsErrval[1] = 0;
		FTP.IRIG_B2.ppsErrval[2] = 0;
		FTP.IRIG_B2.ppsErrval[3] = 0;
	}
	
	//if(file_param.control.global_control1&0x10 && file_param.control.ptp1_control&0x20)
	if(param.event_param[4].flag && file_param.control.ptp1_control&0x20)
	{	
		calTime.tm_year = (FTP.PTP1.time.year[0]<<8)+FTP.PTP1.time.year[1]-1900;
		calTime.tm_mon  = FTP.PTP1.time.month-1;
		calTime.tm_mday = FTP.PTP1.time.day;
		calTime.tm_hour = FTP.PTP1.time.hour;
		calTime.tm_min  = FTP.PTP1.time.min;
		calTime.tm_sec  = FTP.PTP1.time.second;
		pps_second.ptp1_total_sec = mktime(&calTime)-1;
	}
	else
	{
		pps_second.ptp1_total_sec = 0;
		FTP.PTP1.ppsErrval[0] = 0;
		FTP.PTP1.ppsErrval[1] = 0;
		FTP.PTP1.ppsErrval[2] = 0;
		FTP.PTP1.ppsErrval[3] = 0;
	}
	
	//if(file_param.control.global_control1&0x20 && file_param.control.ptp2_control&0x20)
	if(param.event_param[5].flag && file_param.control.ptp2_control&0x20)
	{	
		calTime.tm_year = (FTP.PTP2.time.year[0]<<8)+FTP.PTP2.time.year[1]-1900;
		calTime.tm_mon  = FTP.PTP2.time.month-1;
		calTime.tm_mday = FTP.PTP2.time.day;
		calTime.tm_hour = FTP.PTP2.time.hour;
		calTime.tm_min  = FTP.PTP2.time.min;
		calTime.tm_sec  = FTP.PTP2.time.second;
		pps_second.ptp2_total_sec = mktime(&calTime)-1;
	}
	else
	{
		pps_second.ptp2_total_sec = 0;
		FTP.PTP2.ppsErrval[0] = 0;
		FTP.PTP2.ppsErrval[1] = 0;
		FTP.PTP2.ppsErrval[2] = 0;
		FTP.PTP2.ppsErrval[3] = 0;
	}
	 
	if(lose_lock.bds_flag==0){	//未失锁
		//有模块但是未同步
		if(myclog.BDS_recv_state==0&&(FTP.BDS.source_syn_state&SS_SYNC)!=0){
			lose_lock.bds_flag=1;							//判断失锁
			lose_lock.bds_sec=pps_second.locate_total_sec;	//记录上次失锁的时间戳,用于计算失锁时间
		}	
	}	
	else{						//失锁
		//1.无模块
		//2.有模块且已同步
		if(myclog.BDS_recv_state!=0||(FTP.BDS.source_syn_state&SS_SYNC)==0)
			lose_lock.bds_flag=0;
	}	
	if(lose_lock.gps_flag==0)
	{
		if(myclog.GPS_recv_state==0&&(FTP.GPS.source_syn_state&SS_SYNC)!=0)
		{
			lose_lock.gps_flag=1;
			lose_lock.gps_sec=pps_second.locate_total_sec;
		}	
	}	
	else
	{
		if(myclog.GPS_recv_state!=0||(FTP.GPS.source_syn_state&SS_SYNC)==0)
			lose_lock.gps_flag=0;
	}
	if(lose_lock.b1_flag==0)
	{
		if((FTP.IRIG_B1.source_syn_state&SS_SYNC)!=0)
		{
			lose_lock.b1_flag=1;
			lose_lock.b1_sec=pps_second.locate_total_sec;
		}	
	}	
	else
	{
		if((FTP.IRIG_B1.source_syn_state&SS_SYNC)==0)
			lose_lock.b1_flag=0;
	}
	if(lose_lock.b2_flag==0)
	{
		if((FTP.IRIG_B2.source_syn_state&SS_SYNC)!=0)
		{
			lose_lock.b2_flag=1;
			lose_lock.b2_sec=pps_second.locate_total_sec;
		}	
	}	
	else
	{
		if((FTP.IRIG_B2.source_syn_state&SS_SYNC)==0)
			lose_lock.b2_flag=0;
	}
	return 0;
}



int Run_BoardState(Time serialTime)	//解析和保存插槽状态，产生相应日志
{
	char save_flag=0;
	unsigned char board_type;
	int i;
	/*装置插槽的变化*/
	for(i=0;i<14;i++){
		//board_state=(FTP.Slot_state[i]>>4)&0x01;
		board_type = FTP.Slot_state[i]&0x1f;
		/*
			调试用
		*/
		//if(param.event_param[18].now_state==0)//只在晶振驯服后才做判断
		if(1)
		{
			//插槽检测到板卡插入
			if(board_type_its[board_type]!=param.event_param[2*i+OFF_E_BoardState+1].now_state)
			{
				param.event_param[2*i+OFF_E_BoardState+1].old_val = param.event_param[2*i+OFF_E_BoardState+1].now_state;
				param.event_param[2*i+OFF_E_BoardState+1].now_state = board_type_its[board_type];
				//add_event(2*i+OFF_E_BoardState+1,serialTime);
				add_event_udp(2*i+OFF_E_BoardState+1,serialTime);

				param.event_param[2*i+OFF_E_BoardState].old_val = param.event_param[2*i+OFF_E_BoardState].now_state;
				param.event_param[2*i+OFF_E_BoardState].now_state = board_type;
				save_flag = 1;
				//myprintf("!=[%02d] %02x.%08x(%08x)\n",i,board_type,param.event_param[2*i+OFF_E_BoardState+1].old_val,param.event_param[2*i+OFF_E_BoardState+1].now_state);
				if(param.event_param[2*i+OFF_E_BoardState].old_val==0x1f)
				{	
					add_event(2*i+OFF_E_BoardState,serialTime);
					add_event_udp(2*i+OFF_E_BoardState,serialTime);	
				}	
				//插槽检测到板卡拔出
				else if(board_type==0x1f)
				{	
					add_event(2*i+OFF_E_BoardState,serialTime);
					add_event_udp(2*i+OFF_E_BoardState,serialTime);
				}
			}
		}
		bard_state_udp[i]=board_type_its[board_type];
		/*修改内存中板卡状态(UDP用)*/
		//myprintf("---[%02d] %02x.%08x\n",i,board_type,bard_state_udp[i]);		
	}
	
	if(save_flag){	//如果有保存标志，保存插槽状态到文件系统
		save_board_state();
		save_flag = 0;
	}
	return 0;
}




static void comp_to_event(unsigned char num,unsigned char *myclog_state,unsigned char compval,Time serialTime)
{
	if(compval != (*myclog_state)){		
		param.event_param[num].old_val   = param.event_param[num].now_state;	//保存上一次的状态
		*myclog_state = compval;
		 param.event_param[num].now_state = *myclog_state;
		if(param.event_param[num].flag)		//控制状态有效
		{	
			add_event(num,serialTime);	    //变位信息放入装置临时缓存
			add_event_udp(num,serialTime);  //变位信息放入装置UDP上送缓存
		}	
	}	
}

static int comp_to_udpevent(unsigned char num,unsigned char *myclog_state,unsigned char compval,Time serialTime)
{
	if(compval != (*myclog_state)){		
		param.event_param[num].old_val   = param.event_param[num].now_state;	//保存上一次的状态
		*myclog_state = compval;
		 param.event_param[num].now_state = *myclog_state;
		if(param.event_param[num].flag)		//控制状态有效
		{	
			add_event_udp(num,serialTime);  //变位信息放入装置UDP上送缓存
			return 1;
		}	
	}	
	return 0;
}





int Run_DevLog(Time serialTime)	//处理日志的产生，并保存日志
{
	static long time_syn			= 0; 	//计算所有源不可用的时间。
	char  all_untime;				//至少(投)有1个源:0       一个源都没有:1
	char  temp_state,dev_res1,dev_res2;	
	Handle_CommEvent();				//解析Comm事件，产生对应装置日志或者UDP上送
	/*
	if((param.event_param[1].flag && (FTP.BDS.source_syn_state&SS_SIGNAL)==0) ||
		 (file_param.control.global_control1&0x02 && (FTP.GPS.source_syn_state&SS_SIGNAL)==0) ||
		 (file_param.control.global_control1&0x04 && (FTP.IRIG_B1.source_syn_state&SS_SIGNAL)==0) ||
		 (file_param.control.global_control1&0x08 && (FTP.IRIG_B2.source_syn_state&SS_SIGNAL)==0) ||
		 (file_param.control.global_control1&0x10 && (FTP.PTP1.source_syn_state&SS_SIGNAL)==0) ||
		 (file_param.control.global_control1&0x20 && (FTP.PTP2.source_syn_state&SS_SIGNAL)==0))
		{
		*/
	if(	(param.event_param[0].flag && (FTP.BDS.source_syn_state&SS_SIGNAL)==0) ||
	   	(param.event_param[1].flag && (FTP.GPS.source_syn_state&SS_SIGNAL)==0) ||
		(param.event_param[2].flag && (FTP.IRIG_B1.source_syn_state&SS_SIGNAL)==0) ||
		(param.event_param[3].flag && (FTP.IRIG_B2.source_syn_state&SS_SIGNAL)==0) ||
		(param.event_param[4].flag && (FTP.PTP1.source_syn_state&SS_SIGNAL)==0) ||
		(param.event_param[5].flag && (FTP.PTP2.source_syn_state&SS_SIGNAL)==0))
		{	
			all_untime = 0;	//至少(投放的)有1个源:0   
		}	 
	else
		all_untime = 1;		//一个源都没有

	static int last_s_bdsmode=0,bds_fcnt=0;
	int filter_bds;
	
	static int last_s_gpsmode=0,gps_fcnt=0;
	int filter_gps;
	
	filter_bds=(FTP.BDS.source_ant_state&0X10)>>4;
	filter_gps=(FTP.GPS.source_ant_state&0X10)>>4;
	if(param.initial==0){	//初始化还未完成(不会产生日志)

		last_s_bdsmode=filter_bds;//初始化滤波器的值
		last_s_gpsmode=filter_gps;
		set_initial(all_untime);
		param.initial=1;
	}
	else{					//初始化完成，开始正常判读日志(翻转)

		
		if((filter_bds!=last_s_bdsmode))	
		{	
			if(bds_fcnt<3){
				bds_fcnt++;
				FTP.BDS.source_ant_state&=0xef;
				FTP.BDS.source_ant_state|=(last_s_bdsmode<<4);	
			}
			else{
				last_s_bdsmode=filter_bds;
				bds_fcnt=0;
			}
		}
		else{
			bds_fcnt=0;
		}
		//myprintf("bds now:%d,old:%d,cnt:%d state:%x\n",filter_bds,last_s_bdsmode,bds_fcnt,FTP.BDS.source_ant_state); 

		if((filter_gps!=last_s_gpsmode))	
		{	
			if(gps_fcnt<3){
				gps_fcnt++;
				FTP.GPS.source_ant_state&=0xef;
				FTP.GPS.source_ant_state|=(last_s_gpsmode<<4);	
			}
			else{
				last_s_gpsmode=filter_gps;
				gps_fcnt=0;
			}
		}
		else{
			gps_fcnt=0;
		}
		//myprintf("gps now:%d,old:%d,cnt:%d state:%x\n",filter_gps,last_s_gpsmode,gps_fcnt,FTP.GPS.source_ant_state); 
		
		// 0	BDS源信号(1)异常/(0)恢复
		// 1	GPS源信号(1)异常/(0)恢复
		// 2	IRIG-B1(有线时间基准)信号(1)异常/(0)恢复
		// 3	IRIG-B2（热备）信号(1)异常/(0)恢复
		// 4	PTP1源信号(1)异常/(0)恢复
		// 5	PTP2源信号(1)异常/(0)恢复
		comp_to_event(0,&myclog.BDS_source_state,FTP.BDS.source_syn_state&SS_SIGNAL,serialTime);
		comp_to_event(1,&myclog.GPS_source_state,FTP.GPS.source_syn_state&SS_SIGNAL,serialTime);
		comp_to_event(2,&myclog.B1_source_state,FTP.IRIG_B1.source_syn_state&SS_SIGNAL,serialTime);	
		comp_to_event(3,&myclog.B2_source_state,FTP.IRIG_B2.source_syn_state&SS_SIGNAL,serialTime);
		comp_to_event(4,&myclog.PTP1_source_state,FTP.PTP1.source_syn_state&SS_SIGNAL,serialTime);
		comp_to_event(5,&myclog.PTP2_source_state,FTP.PTP2.source_syn_state&SS_SIGNAL,serialTime);
		//----6----北斗天线状态
		if((FTP.BDS.source_ant_state&0X0F)!=myclog.BDS_ant_state)
		{
			temp_state = myclog.BDS_ant_state;
			param.event_param[6].old_val = param.event_param[6].now_state;
			param.event_param[6].now_state=myclog.BDS_ant_state=FTP.BDS.source_ant_state&0x0f;
			if(param.event_param[6].flag)
			{
				add_event(6,serialTime);
				if(temp_state==2 || myclog.BDS_ant_state==2)//只有正常变其他或者其他变正常才产生日志
					add_event_udp(6,serialTime);
				//UDP只反映天线模块正常和异常(bool),即状态2(正常)到其他或者其他到2。
			}	
		}
		//----7----GPS天线状态
		if((FTP.GPS.source_ant_state&0X0F)!=myclog.GPS_ant_state)
		{
			temp_state = myclog.GPS_ant_state;
			param.event_param[7].old_val = param.event_param[7].now_state;
			param.event_param[7].now_state=myclog.GPS_ant_state=FTP.GPS.source_ant_state&0x0f;
			if(param.event_param[7].flag)
			{
				add_event(7,serialTime);
				if(temp_state==2 || myclog.GPS_ant_state==2)
					add_event_udp(7,serialTime);
				//UDP只反映天线模块正常和异常(bool),即状态2(正常)到其他或者其他到2。
				//myprintf("temp_state=%d,GPS_ant_state=%d\n",temp_state,myclog.GPS_ant_state);
			}	
		}		

		
		//----8----北斗卫星接收模块状态
		comp_to_event(8,&myclog.BDS_recv_state,(FTP.BDS.source_ant_state&0X10)>>4,serialTime);
		comp_to_event(9,&myclog.GPS_recv_state,(FTP.GPS.source_ant_state&0X10)>>4,serialTime);

		// 10	时间跳变侦测状态(1)异常/(0)恢复
		// 11	BDS时间跳变侦测状态(1)异常/(0)恢复
		// 12	GPS时间跳变侦测状态(1)异常/(0)恢复	
		// 13	IRIG-B1(有线时间基准)时间跳变侦测状态(1)异常/(0)恢复	
		// 14	IRIG-B2(热备)时间跳变侦测状态(1)异常/(0)恢复
		// 15	PTP1源时间跳变侦测状态(1)异常/(0)恢复
		// 16	PTP2源时间跳变侦测状态(1)异常/(0)恢复
		comp_to_event(10,&myclog.Lo_TimeAbnormal,FTP.LoTimeAbnormal&0X01,serialTime);	
		comp_to_event(11,&myclog.BDS_TimeAbnormal,(FTP.BDS.source_syn_state&SS_CONTINUITY)>>2,serialTime);	
		comp_to_event(12,&myclog.GPS_TimeAbnormal,(FTP.GPS.source_syn_state&SS_CONTINUITY)>>2,serialTime);
		comp_to_event(13,&myclog.B1_TimeAbnormal,(FTP.IRIG_B1.source_syn_state&SS_CONTINUITY)>>2,serialTime);
		comp_to_event(14,&myclog.B2_TimeAbnormal,(FTP.IRIG_B2.source_syn_state&SS_CONTINUITY)>>2,serialTime);
		comp_to_event(15,&myclog.PTP1_TimeAbnormal,(FTP.PTP1.source_syn_state&SS_CONTINUITY)>>2,serialTime);
		comp_to_event(16,&myclog.PTP2_TimeAbnormal,(FTP.PTP2.source_syn_state&SS_CONTINUITY)>>2,serialTime);
		
		static int err_cnt 	= 0;				 //已驯服后的晶振驯服时异常需要计时60s
		#define SYNC_TIMEOUT 59					//驯服超时时间
		if((FTP.Self_Checkstate&0x20)>>5)		//晶振驯服异常
		{
			if(myclog.Crystal_state==0 && param.event_param[17].flag )
			{	
				if((param.tame&0x1)==0){	//未驯服过
					param.event_param[17].old_val 	= param.event_param[17].now_state;//先保存旧值
					param.event_param[17].now_state	= 1;	//再保存新值
					myclog.Crystal_state 			= 1;	//日志翻转
					add_event(17,serialTime);
					add_event_udp(17,serialTime);
					param.tame|=0x2;		//装置状态位：0x2频率源异常	
				}	
				else{						//已驯服过
					if(err_cnt==SYNC_TIMEOUT)
					{		//连续60S
						param.event_param[17].old_val 	= param.event_param[17].now_state;//先保存旧值
						param.event_param[17].now_state	= 1;	//再保存新值
						myclog.Crystal_state 			= 1;	//日志翻转
						add_event(17,serialTime);
						add_event_udp(17,serialTime);
						param.tame|=0x2;	
					}
					if(err_cnt<SYNC_TIMEOUT+1)
						err_cnt++;
					if(err_cnt==SYNC_TIMEOUT)
					myprintf("驯服后晶振计时err_cnt=%d\n",err_cnt);
				}
			}
			
		}
		else{									//晶振驯服正常
			if(myclog.Crystal_state==1 && param.event_param[17].flag) {
				param.event_param[17].old_val	= param.event_param[17].now_state;
				param.event_param[17].now_state = 0;
				myclog.Crystal_state			= 0;
				add_event(17,serialTime);		//产生[18]晶振驯服异常恢复
				add_event_udp(17,serialTime);
			}
			if((param.tame&0x1)==0){
				myprintf("首次晶振驯服\n");
			}
			param.tame=0x1;						//只要收到，状态位：0x01 正常
			err_cnt=0;
		}
		// 18	初始化状态(1)异常/(0)恢复
		// 19	电源模块状态(1)异常/(0)恢复
		// 20	电源模块1状态(1)异常/(0)恢复		
		// 21	电源模块2状态(1)异常/(0)恢复
		comp_to_event(18,&myclog.Init_state,(FTP.Self_Checkstate&0x10)>>4,serialTime);
		comp_to_event(19,&myclog.Power_state,((FTP.Self_Checkstate&0XC0)!=0x00),serialTime);					
		comp_to_event(20,&myclog.P1_power_state,(FTP.Self_Checkstate&0X80)>>7,serialTime);
		comp_to_event(21,&myclog.P2_power_state,(FTP.Self_Checkstate&0X40)>>6,serialTime);
	
		//22	IRIG-B码输入质量低于本机
		//myclog.B1_source_stat状态有在点[2]有改变的可能。
		dev_res1=(myclog.B1_source_state==0)&&((FTP.IRIG_B1.timestate&0X0F)>(FTP.LoTimestate&0X0F));
		dev_res2=(myclog.B2_source_state==0)&&((FTP.IRIG_B2.timestate&0X0F)>(FTP.LoTimestate&0X0F));
		if(dev_res1||dev_res2)		//任意一路(有效的)B码低于本机
		{
			if(myclog.IRIG_Q==0)	//低于本机
			{
				if(param.event_param[22].flag)
				{
					param.event_param[22].old_val  = param.event_param[22].now_state;
					param.event_param[22].now_state=myclog.IRIG_Q=1;
					add_event(22,serialTime);
					add_event_udp(22,serialTime);
				}	
			}
		}	
		else if((FTP.IRIG_B1.timestate&0X0F)<=(FTP.LoTimestate&0X0F)&&(FTP.IRIG_B2.timestate&0X0F)<=(FTP.LoTimestate&0X0F))
		{
			if(myclog.IRIG_Q==1)	//低于本机恢复
			{
				param.event_param[22].old_val  = param.event_param[22].now_state;
				param.event_param[22].now_state=myclog.IRIG_Q=0;
				add_event(22,serialTime);
				add_event_udp(22,serialTime);
			}
		} 
		//----23----所有独立时源不可用小于30分钟（首次同步）
		//----28----所有独立时源不可用超过30分钟（首次同步）
		//----24----所有独立时源不可用小于24小时（曾经同步过）
		//----29----所有独立时源不可用超过24小时（曾经同步过）
		if(all_untime==1)			//所有独立时源不可用
		{		
			if(time_syn<1000000)
				time_syn++;	
			if(myclog.Init_state==1)//首次同步
			{		
				if(myclog.source_frist_err_30m==0 && param.event_param[23].flag)
				{
					if(time_syn>=3)
					{
						param.event_param[23].old_val = param.event_param[23].now_state;
						param.event_param[23].now_state=myclog.source_frist_err_30m=1;
						add_event(23,serialTime);
						add_event_udp(23,serialTime);
					}	
				}
				if(myclog.source_frist_err_off_30m==0 && param.event_param[28].flag)
				{
					if(time_syn>=(30*60+3))
					{
						param.event_param[28].old_val  = param.event_param[28].now_state;
						param.event_param[28].now_state=myclog.source_frist_err_off_30m=1;
						add_event(28,serialTime);
						add_event_udp(28,serialTime);
					}
				}
			}
			else					//已经同步
			{	
				if(myclog.source_err_24h==0 && param.event_param[24].flag)
				{
					if(time_syn>=3)
					{
						param.event_param[24].old_val  = param.event_param[24].now_state;
						param.event_param[24].now_state=myclog.source_err_24h=1;	
						add_event(24,serialTime);
						add_event_udp(24,serialTime);
					}	
				}
				if(myclog.source_err_off_24h==0 && param.event_param[29].flag)
				{
					if(time_syn >= (60*60*24+3))
					{
						param.event_param[29].old_val  = param.event_param[29].now_state;
						param.event_param[29].now_state=myclog.source_err_off_24h=1;
						add_event(29,serialTime);
						add_event_udp(29,serialTime);
					}
				}
			}				
		}
		else{	//时间源可以用
		    time_syn = 0;
			param.event_param[23].old_val = param.event_param[23].now_state;
			param.event_param[24].old_val = param.event_param[24].now_state;
			param.event_param[28].old_val = param.event_param[28].now_state;
			param.event_param[29].old_val = param.event_param[29].now_state;
			param.event_param[23].now_state=0;
			param.event_param[24].now_state=0;
			param.event_param[28].now_state=0;
			param.event_param[29].now_state=0;
			if(myclog.source_frist_err_30m)
			{
				myclog.source_frist_err_30m = 0;			
				add_event(23,serialTime);
				add_event_udp(23,serialTime);
			}
			if(myclog.source_frist_err_off_30m)
			{
				myclog.source_frist_err_off_30m=0;
				add_event(28,serialTime);
				add_event_udp(28,serialTime);
			}
			if(myclog.source_err_24h)
			{
				myclog.source_err_24h=0;					
				add_event(24,serialTime);
				add_event_udp(24,serialTime);
			}
			if(myclog.source_err_off_24h)
			{
				myclog.source_err_off_24h=0;
				add_event(29,serialTime);
				add_event_udp(29,serialTime);
			}
		}
		//----31---闰秒预告
		if((FTP.LoTimestate&0x80) >>7 != myclog.Lo_leap_s)
		{
			param.event_param[31].old_val  = param.event_param[31].now_state;
			param.event_param[31].now_state=myclog.Lo_leap_s=(FTP.LoTimestate&0X80)>>7;
			if(param.event_param[31].flag)
			{
				add_event(31,serialTime);
				add_event_udp(31,serialTime);
				if(param.event_param[31].now_state==1){//只在闰秒发生时产生，直接报正负号，不做比较
					param.event_param[36].old_val  = param.event_param[36].now_state;
					param.event_param[36].now_state=myclog.Lo_leap_b=(FTP.LoTimestate&0X40)>>6;
					add_event(36,serialTime);
					add_event_udp(36,serialTime);
				}
			}	
		}
		//myprintf("[32]:%d[37]:%d\n",param.event_param[31].now_state,param.event_param[36].now_state);
		// 32	BDS闰秒预告
		// 33	GPS闰秒预告
		// 34	IRIG_B1闰秒预告
		// 35	IRIG_B2闰秒预告
		// 36	闰秒标志(闰秒标志正负不报告)
		// 37	时间源选择
		comp_to_event(32,&myclog.BD_leap_s,(FTP.BDS.timestate&0X80)>>7,serialTime);
		comp_to_event(33,&myclog.GPS_leap_s,(FTP.GPS.timestate&0X80)>>7,serialTime);	
		comp_to_event(34,&myclog.B1_leap_s,(FTP.IRIG_B1.timestate&0X80) >> 7,serialTime);	
		comp_to_event(35,&myclog.B2_leap_s,(FTP.IRIG_B2.timestate&0X80) >> 7,serialTime);
		// 36在31的位子已经特殊处理	
		comp_to_event(37,&myclog.Time_sour,FTP.LoSourceTime,serialTime);
		//---38-45----优先级改变
		if(param.event_param[43].now_state!=param.bad_priorty)
		{
			param.event_param[43].old_val = param.event_param[43].now_state;
			param.event_param[43].now_state =param.bad_priorty;
			event_from_lcd(43);
		}	
		if(param.event_param[42].now_state!=param.gps_priorty)
		{
			param.event_param[42].old_val = param.event_param[42].now_state;
		    param.event_param[42].now_state =param.gps_priorty;
			event_from_lcd(42);
		}	
		if(param.event_param[44].now_state!=param.b1_priorty)
		{
			param.event_param[44].old_val = param.event_param[44].now_state;
		    param.event_param[44].now_state =param.b1_priorty;
			event_from_lcd(44);
		}
		if(param.event_param[45].now_state!=param.b2_priorty)
		{
			param.event_param[45].old_val = param.event_param[45].now_state;
		    param.event_param[45].now_state =param.b2_priorty;
			event_from_lcd(45);
		}
		if(param.event_param[38].now_state!=param.PTP1_priorty)
		{
			param.event_param[38].old_val = param.event_param[38].now_state;
		    param.event_param[38].now_state =param.PTP1_priorty;
			event_from_lcd(38);
		}
		if(param.event_param[39].now_state!=param.PTP2_priorty)
		{
			param.event_param[39].old_val = param.event_param[39].now_state;
		 	param.event_param[39].now_state =param.PTP2_priorty;
			event_from_lcd(39);
		}

		// 60	同步状态
		// 89	BDS 板卡状态 ==>点[8]的副本
		// 91	GPS 板卡状态 ==>点[9]的副本
		
		comp_to_event(60,&myclog.Dev_TB,(FTP.Self_Checkstate&0X08)>>3,serialTime);
		int res_comp;
		res_comp=0;	
		res_comp=comp_to_udpevent(89,&myclog.BDS_board_state,(FTP.BDS.source_ant_state&0X10)>>4,serialTime);
		if(res_comp & param.event_param[90].flag){
			param.event_param[90].old_val = param.event_param[90].now_state;
			if(param.event_param[89].now_state == 1)		//BDS板卡
		    	param.event_param[90].now_state = 0;		//BDS板卡类型
			else
				param.event_param[90].now_state = 0x0103;	//BDS插件类型
			add_event_udp(90,serialTime); 
		}
		res_comp=comp_to_udpevent(91,&myclog.GPS_board_state,(FTP.GPS.source_ant_state&0X10)>>4,serialTime);
		if(res_comp & param.event_param[92].flag){
			param.event_param[92].old_val = param.event_param[92].now_state;
			if(param.event_param[91].now_state == 1)		//GPS板卡
		    	param.event_param[92].now_state = 0;		//GPS板卡类型
			else
				param.event_param[92].now_state = 0x0107;	//GPS插件类型
			add_event_udp(92,serialTime); 
		}
		if(param.initial<3)
			param.initial++;
		//计算装置的参数
		
	}
	return 0;
}

int Update_DevLog(void)	//完成日志文件更新
{
	save_DevLog();		//更新日志到内存
	if(param.writ_event_flag==1 && param.writ_event_time>=LOG_WRITE_TIMEC)
		write_DevLog(); //更新日志到文件
	if(param.writ_event_time<LOG_WRITE_TIMEC)
		param.writ_event_time++;	
	return 0;			
}

int read_log_flie(void)
{
	LOG_MESSAGE log_buf;//写调试日志buf
	FILE *fp;
	int i,j;
	char file_buf[LOG_MAX_COUNT*LOG_RECORD_LEN];
	if(param.dir==0)	//检查"/mnt/HD/log"文件
		return -1;
	if((fp = fopen(DEV_LOG_FILE,"r")) == NULL)
	{
		sprintf(log_buf.name,"读日志时打开文件失败,初始化日志文件");
		add_my_log(log_buf);
		param.event_mem_id=0;
		param.event_count=0;
		param.event_file_siet=0;
		if((fp = fopen(DEV_LOG_FILE,"w+")) == NULL)
		{
			sprintf(log_buf.name,"读日志时建立文件失败");
			add_my_log(log_buf);
			return -1;
		}
	}
	fseek(fp,0L,SEEK_END);
	int file_len=ftell(fp);
	if(file_len==0)
	{
		fclose(fp);
		param.event_mem_id=0;
		param.event_count=0;
		param.event_file_siet=0;
		return 0;
	}
	if(file_len>LOG_MAX_COUNT*LOG_RECORD_LEN || (file_len%LOG_RECORD_LEN)!=0)
	{
		sprintf(log_buf.name,"读日志发现文件长度错误：len=%d",file_len);
		add_my_log(log_buf);
		fclose(fp);
		param.event_mem_id=0;
		param.event_count=0;
		param.event_file_siet=0;
		if((fp = fopen(DEV_LOG_FILE,"w+")) != NULL)
		fclose(fp);
		return -2;
	}
	fseek(fp,0L,SEEK_SET);
	fread(file_buf, 1,file_len, fp);
	fclose(fp);
	param.event_count = file_len/LOG_RECORD_LEN;
	for(i=0;i<param.event_count;i++)
	{
		temp_record[i].mem_id = 100*(file_buf[i*LOG_RECORD_LEN+0]-0x30) +
								10*(file_buf[i*LOG_RECORD_LEN+1]-'0') +file_buf[i*LOG_RECORD_LEN+2]-'0';
		temp_record[i].index  = 10*(file_buf[i*LOG_RECORD_LEN+4]-'0') +
								file_buf[i*LOG_RECORD_LEN+5]-'0';
		temp_record[i].value  = 10*(file_buf[i*LOG_RECORD_LEN+7]-'0') +
								file_buf[i*LOG_RECORD_LEN+8]-'0';	
		temp_record[i].old_val= 10*(file_buf[i*LOG_RECORD_LEN+10]-'0') +
								file_buf[i*LOG_RECORD_LEN+11]-'0';														 	  											 	  
		
		temp_record[i].warn_time.year=1000*(file_buf[i*LOG_RECORD_LEN+13]-'0') +
										100*(file_buf[i*LOG_RECORD_LEN+14]-'0') +
										10*(file_buf[i*LOG_RECORD_LEN+15]-'0') +
										file_buf[i*LOG_RECORD_LEN+16]-'0';
		temp_record[i].warn_time.mon  = 10*(file_buf[i*LOG_RECORD_LEN+18]-'0') +
										file_buf[i*LOG_RECORD_LEN+19]-'0';
		temp_record[i].warn_time.day  = 10*(file_buf[i*LOG_RECORD_LEN+21]-'0') +
										file_buf[i*LOG_RECORD_LEN+22]-'0';	
		temp_record[i].warn_time.hour = 10*(file_buf[i*LOG_RECORD_LEN+24]-'0') +
										file_buf[i*LOG_RECORD_LEN+25]-'0';
		temp_record[i].warn_time.min  = 10*(file_buf[i*LOG_RECORD_LEN+27]-'0') +
										file_buf[i*LOG_RECORD_LEN+28]-'0';	
		temp_record[i].warn_time.sec  = 10*(file_buf[i*LOG_RECORD_LEN+30]-'0') +
										file_buf[i*LOG_RECORD_LEN+31]-'0';
		if(0){
		printf("第%d记录：memid=%d,index=%d,val=%d,time=%04d-%02d-%02d %02d:%02d:%02d\n",i,
						temp_record[i].mem_id,
						temp_record[i].index,
						temp_record[i].value,
						temp_record[i].warn_time.year,
						temp_record[i].warn_time.mon,
						temp_record[i].warn_time.day,
						temp_record[i].warn_time.hour,
						temp_record[i].warn_time.min,
						temp_record[i].warn_time.sec);	
		}
	}
	param.event_file_siet = param.event_count;
//	param.event_mem_id=temp_record[param.event_count-1].mem_id+1;
	j=0;
	for(i=0;i<param.event_count;i++)
	{
		param.event_mem_id=temp_record[j].mem_id;
		mem_record[i].mem_id = temp_record[j].mem_id;
		mem_record[i].index  = temp_record[j].index;
		mem_record[i].value  = temp_record[j].value;
		mem_record[i].warn_time.year = temp_record[j].warn_time.year;
		mem_record[i].warn_time.mon  = temp_record[j].warn_time.mon;
		mem_record[i].warn_time.day  = temp_record[j].warn_time.day;
		mem_record[i].warn_time.hour = temp_record[j].warn_time.hour;
		mem_record[i].warn_time.min  = temp_record[j].warn_time.min;
		mem_record[i].warn_time.sec  = temp_record[j].warn_time.sec;
		j++;
		if(j>=LOG_MAX_COUNT)
			j=0;
		if(0){
		printf("第%d记录：memid=%d,index=%d,val=%d,time=%04d-%02d-%02d %02d:%02d:%02d\n",i,
						mem_record[i].mem_id,
						mem_record[i].index,
						mem_record[i].value,
						mem_record[i].warn_time.year,
						mem_record[i].warn_time.mon,
						mem_record[i].warn_time.day,
						mem_record[i].warn_time.hour,
						mem_record[i].warn_time.min,
						mem_record[i].warn_time.sec);
		}
	}
	return 0;
}



