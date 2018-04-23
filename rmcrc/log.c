#include <time.h>
#include <pthread.h>
#include "bsp.h"



#define LOG_WRITE_TIMEC 30

int	      now_mem_set;					//��ǰ�洢λ��
COM_EVENT com_event;					//��ͨ��־��¼��ָ��װ�ò���״̬�޹ص���־		
RECORD    mem_record[LOG_MAX_COUNT];	//װ����־(�ڴ��еĽṹ)
RECORD 	  temp_record[LOG_MAX_COUNT];	//װ����־�Ļ���
int	   	  temp_record_count;			//װ����־�������־����


static void add_event(int id,Time serialTime);	//���������¼�������־������


const char event_name[100][100] = {
	{"�����ź�  "},											//---0
	{"GPS�ź�  "},											//--1
	{"IRIG-B1�������ź�  "},						//--2
	{"IRIG-B2�������ź�  "},						//--3
	{"PTP1�����ź�  "},									//--4
	{"PTP2�����ź�  "},									//--5
	{"��������״̬  "},									//--6
	{"GPS����״̬  "},									//--7
	{"�������ǽ���ģ��״̬  "},					//--8
	{"GPS���ǽ���ģ��״̬  "},					//--9
	{"ʱ���������״̬  "},							//--10
	{"BDSʱ���������״̬  "},					//--11
	{"GPSʱ���������״̬  "},					//--12
	{"IRIG_B1ʱ���������״̬  "},			//--13
	{"IRIG_B2ʱ���������״̬  "},			//--14
	{"PTP1Դʱ���������״̬  "},				//--15
	{"PTP2Դʱ���������״̬  "},				//--16
	{"����ѱ��״̬  "},									//--17
	{"��ʼ��״̬  "},										//--18
	{"��Դģ��״̬  "},									//--19
	{"P1��Դģ��״̬  "},								//--20
	{"P2��Դģ��״̬  "},								//--21
	{"IRIG-B�������������ڱ���  "},			//--22
	{"���ж���ʱԴ������С��30���ӣ��״�ͬ����  "},		//--23
	{"���ж���ʱԴ������С��24Сʱ������ͬ������  "},	//--24
	{"�����ɻָ�����                              "},	//--25
	{"CPU���İ��쳣                               "},	//--26
	{"����ѱ��״̬�쳣����60��                    "},	//--27
	{"���ж���ʱԴ�����ó���30���ӣ��״�ͬ����  "},		//--28
	{"���ж���ʱԴ�����ó���24Сʱ������ͬ������  "},	//--29
	{"�������ع���   "},		//--30
	{"����Ԥ��  "},					//--31
	{"BDS����Ԥ��  "},			//--32
	{"GPS����Ԥ��  "},			//--33
	{"IRIG_B1����Ԥ��  "},	//--34
	{"IRIG_B2����Ԥ��  "},	//--35
	{"�����־�仯     "},	//--36
	{"ʱ��Դ�����仯	 "},	//--37
	{"PTP1���ȼ��仯 "},	//--38
	{"PTP1���ȼ��仯 "},	//--39
	{"               "},	//--40
	{"               "},	//--41
	{"GPS���ȼ��仯  "},	//--42
	{"BDS���ȼ��仯  "},	//--43
	{"B1���ȼ��仯   "},	//--44
	{"B2���ȼ��仯   "},	//--45
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
	{"�û���¼       "},	 //--59
	{"               "},	//--60
	{"��λ1�忨      "},	 //-61	OFF_E_BoardState
	{"               "},	//--62
	{"��λ2�忨      "},	 //-63
	{"               "},	//--64
	{"��λ3�忨      "},	 //-65
	{"               "},	//--66
	{"��λ4�忨      "},	 //-67
	{"               "},	//--68
	{"��λ5�忨      "},	 //-69
	{"               "},	//--70
	{"��λ6�忨      "},	 //-71
	{"               "},	//--72
	{"��λ7�忨      "},	 //-73
	{"               "},	//--74
	{"��λ8�忨      "},	 //-75
	{"               "},	//--76
	{"��λ9�忨      "},	 //-77
	{"               "},	//--78
	{"��λ10�忨     "},	 //-79
	{"               "},	//--80
	{"��λ11�忨     "},	 //-81
	{"               "},	//--82
	{"��λ12�忨     "},	 //-83
	{"               "},	//--84
	{"��λ13�忨     "},	 //-85
	{"               "},	//--86
	{"��λ14�忨     "},	 //-87
	{"               "},	//--88
	{"��λ15�忨     "},	 //-89
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
	usleep(1500000);//�ӳ�1.5�룬������һ֡��������
	
	param.rx_serial=1;//�޸ļ�¼2017.11.20
	while(1)
	{	 
		sem_wait(&sem_serial_accept);
		compare_log();
		sem_post(&sem_can_dy_send);	
	}	
	pthread_exit(NULL);
}
*/

void add_event(int id,Time serialTime)	//���������¼�������־������
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
	
	temp_record_count++;	//��������+1
	param.event_mem_id++;
	return;
}
//add_comm_vent
void event_from_lcd(int id)	 //�˻��������ò�������Comm��־��¼��
{
	while(com_event.op_flag) //��־λ�����ã�����
		usleep(1000);
	com_event.op_flag = 1;
	com_event.event_list[com_event.event_count] = id;
	com_event.event_count++;
	com_event.op_flag=0;	 //�ָ����ñ�־λ
}

void Handle_CommEvent(void)			//ת��Comm��־��¼���������(װ����־/UDP�¼�)
{
	int i;
	Time now_time;
	now_time.y.year 	= sys_time.year;
	now_time.mo.month 	= sys_time.mon;
	now_time.d.day 		= sys_time.day;
	now_time.h.hour 	= sys_time.hour;
	now_time.m.min 		= sys_time.min;
	now_time.s.second	= sys_time.sec;
	while(com_event.op_flag)  //��־λ�����ã�����
		usleep(1000);
	com_event.op_flag = 1;
	for(i=0;i<com_event.event_count;i++){
		add_event(com_event.event_list[i],now_time);	//����װ����־
		add_event_udp(com_event.event_list[i],now_time);//����װ��UDP�¼�
	}
	com_event.event_count=0;
	com_event.op_flag = 0;
}

int	write_DevLog(void)	//��װ����־(�������ڴ�)д���ļ�ϵͳ
{
	LOG_MESSAGE log_buf;//д������־buf
	int i,j;
	int event_id;
	char buf[LOG_RECORD_LEN];
	char file_buf[LOG_RECORD_LEN*LOG_MAX_COUNT];
	FILE *fp;
	for(event_id=0;event_id<param.event_count;event_id++)
	{
		for(i=0;i<LOG_RECORD_LEN;i++)	//����0
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
	if(param.dir==0)//���"/mnt/HD/log"�ļ�
		return -1;
	if((fp = fopen(DEV_LOG_FILE,"w")) == NULL){
		sprintf(log_buf.name,"д��־ʱ���ļ�ʧ��,��ʼ����־�ļ�");
		add_my_log(log_buf);
		param.event_mem_id=0;
		param.event_count=0;
		param.event_file_siet=0;
		if((fp = fopen(DEV_LOG_FILE,"w")) == NULL){
			sprintf(log_buf.name,"д��־ʱ�����ļ�ʧ��(2)�������ļ�����");
			add_my_log(log_buf);
			return -1;
		}
	}	
	int ret = fwrite(file_buf,1,param.event_count*LOG_RECORD_LEN,fp);
	if(ret!=param.event_count*LOG_RECORD_LEN){
		sprintf(log_buf.name,"д��־ʱ�ļ�д��ʧ��,��ʼ����־�ļ� ret=%d, length=%d",ret,param.event_count*LOG_RECORD_LEN);
		add_my_log(log_buf);
		fclose(fp);
		printf("%s\n",log_buf.name);		
		return -2;
	}
	fseek(fp,0L,SEEK_END);
	int file_len=ftell(fp);	//�����ļ�����(����0Lλ��)
	if(file_len!=ret || (file_len%LOG_RECORD_LEN)!=0){
		sprintf(log_buf.name,"д��־�󳤶ȴ���:ret=%d, length=%d",ret,file_len);
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
	
	
	param.writ_event_flag = 0;	//���д��־
	param.writ_event_time = 0;	//�����ʱ
	return 0;
}


int save_DevLog()//�Ѳ�������־���µ�װ����־(�������ڴ�)������ʶ��д���ļ�
{
	LOG_MESSAGE log_buf;//д������־buf
	int i,j,res;
	if(temp_record_count==0)							  //û����Ҫ��ģ�ֱ���˳�
		return -1;
	res=temp_record_count+param.event_count-LOG_MAX_COUNT;//��Ҫ�ƶ�������
	if(res>0){											  //��0����Ҫ��
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
	for(i=0;i<temp_record_count;i++)					  //��д�²������־
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
			printf("����־��%d,%d,%d,time=%d:%d:%d\n",
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
	param.writ_event_flag = 1;		//�ļ���д�ı�־λ
	return 0;
}

void comp_clock_offset(void)		//����6��Դ���Ӳ�ֵ
{
	//LOG_MESSAGE log_buf;//д������־buf
	int iTemp;
	char neg_flag;
	//unsigned int pps_errval_sec;
	//unsigned int pps_errval_nsec;
	long long PPS_errval;
	if(param.event_param[100].flag==0 || param.event_param[101].flag==0){
		clock_offset.BDS.sec  = 0;	//����ӳٻ���BD�����ӳ���Ч,ֵ=0
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
			neg_flag = 1;	//����Ϊ��
		}	
		else{
			neg_flag = 0;
		}
		clock_offset.BDS.sec = (long long)PPS_errval/1000000000;
		clock_offset.BDS.nsec= (long long)PPS_errval%1000000000;
		if(neg_flag){		//���������λΪ1.
			clock_offset.BDS.sec |= 0x80000000;
			clock_offset.BDS.nsec |= 0x80000000;
		}
	}
	else{							//δͬ����������Ҫ�����0x7fffffff
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
	//������:FPGA������������ 2017.11.21
}




int Run_DevData(void) //����Ƶ�ʺ��Ӳ��
{
	struct tm  calTime;
	int cycle1,freque;
	if(param.initial==0)	//��ʼ����δ���(���������־)
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
		FTP.LoTimeAbnormal |= 0x01;		//�����������ź�
	else
		FTP.LoTimeAbnormal &= 0xfe;	 	//�����������ź�=0��û�����䣩
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
	 
	if(lose_lock.bds_flag==0){	//δʧ��
		//��ģ�鵫��δͬ��
		if(myclog.BDS_recv_state==0&&(FTP.BDS.source_syn_state&SS_SYNC)!=0){
			lose_lock.bds_flag=1;							//�ж�ʧ��
			lose_lock.bds_sec=pps_second.locate_total_sec;	//��¼�ϴ�ʧ����ʱ���,���ڼ���ʧ��ʱ��
		}	
	}	
	else{						//ʧ��
		//1.��ģ��
		//2.��ģ������ͬ��
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



int Run_BoardState(Time serialTime)	//�����ͱ�����״̬��������Ӧ��־
{
	char save_flag=0;
	unsigned char board_type;
	int i;
	/*װ�ò�۵ı仯*/
	for(i=0;i<14;i++){
		//board_state=(FTP.Slot_state[i]>>4)&0x01;
		board_type = FTP.Slot_state[i]&0x1f;
		/*
			������
		*/
		//if(param.event_param[18].now_state==0)//ֻ�ھ���ѱ��������ж�
		if(1)
		{
			//��ۼ�⵽�忨����
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
				//��ۼ�⵽�忨�γ�
				else if(board_type==0x1f)
				{	
					add_event(2*i+OFF_E_BoardState,serialTime);
					add_event_udp(2*i+OFF_E_BoardState,serialTime);
				}
			}
		}
		bard_state_udp[i]=board_type_its[board_type];
		/*�޸��ڴ��а忨״̬(UDP��)*/
		//myprintf("---[%02d] %02x.%08x\n",i,board_type,bard_state_udp[i]);		
	}
	
	if(save_flag){	//����б����־��������״̬���ļ�ϵͳ
		save_board_state();
		save_flag = 0;
	}
	return 0;
}




static void comp_to_event(unsigned char num,unsigned char *myclog_state,unsigned char compval,Time serialTime)
{
	if(compval != (*myclog_state)){		
		param.event_param[num].old_val   = param.event_param[num].now_state;	//������һ�ε�״̬
		*myclog_state = compval;
		 param.event_param[num].now_state = *myclog_state;
		if(param.event_param[num].flag)		//����״̬��Ч
		{	
			add_event(num,serialTime);	    //��λ��Ϣ����װ����ʱ����
			add_event_udp(num,serialTime);  //��λ��Ϣ����װ��UDP���ͻ���
		}	
	}	
}

static int comp_to_udpevent(unsigned char num,unsigned char *myclog_state,unsigned char compval,Time serialTime)
{
	if(compval != (*myclog_state)){		
		param.event_param[num].old_val   = param.event_param[num].now_state;	//������һ�ε�״̬
		*myclog_state = compval;
		 param.event_param[num].now_state = *myclog_state;
		if(param.event_param[num].flag)		//����״̬��Ч
		{	
			add_event_udp(num,serialTime);  //��λ��Ϣ����װ��UDP���ͻ���
			return 1;
		}	
	}	
	return 0;
}





int Run_DevLog(Time serialTime)	//������־�Ĳ�������������־
{
	static long time_syn			= 0; 	//��������Դ�����õ�ʱ�䡣
	char  all_untime;				//����(Ͷ)��1��Դ:0       һ��Դ��û��:1
	char  temp_state,dev_res1,dev_res2;	
	Handle_CommEvent();				//����Comm�¼���������Ӧװ����־����UDP����
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
			all_untime = 0;	//����(Ͷ�ŵ�)��1��Դ:0   
		}	 
	else
		all_untime = 1;		//һ��Դ��û��

	static int last_s_bdsmode=0,bds_fcnt=0;
	int filter_bds;
	
	static int last_s_gpsmode=0,gps_fcnt=0;
	int filter_gps;
	
	filter_bds=(FTP.BDS.source_ant_state&0X10)>>4;
	filter_gps=(FTP.GPS.source_ant_state&0X10)>>4;
	if(param.initial==0){	//��ʼ����δ���(���������־)

		last_s_bdsmode=filter_bds;//��ʼ���˲�����ֵ
		last_s_gpsmode=filter_gps;
		set_initial(all_untime);
		param.initial=1;
	}
	else{					//��ʼ����ɣ���ʼ�����ж���־(��ת)

		
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
		
		// 0	BDSԴ�ź�(1)�쳣/(0)�ָ�
		// 1	GPSԴ�ź�(1)�쳣/(0)�ָ�
		// 2	IRIG-B1(����ʱ���׼)�ź�(1)�쳣/(0)�ָ�
		// 3	IRIG-B2���ȱ����ź�(1)�쳣/(0)�ָ�
		// 4	PTP1Դ�ź�(1)�쳣/(0)�ָ�
		// 5	PTP2Դ�ź�(1)�쳣/(0)�ָ�
		comp_to_event(0,&myclog.BDS_source_state,FTP.BDS.source_syn_state&SS_SIGNAL,serialTime);
		comp_to_event(1,&myclog.GPS_source_state,FTP.GPS.source_syn_state&SS_SIGNAL,serialTime);
		comp_to_event(2,&myclog.B1_source_state,FTP.IRIG_B1.source_syn_state&SS_SIGNAL,serialTime);	
		comp_to_event(3,&myclog.B2_source_state,FTP.IRIG_B2.source_syn_state&SS_SIGNAL,serialTime);
		comp_to_event(4,&myclog.PTP1_source_state,FTP.PTP1.source_syn_state&SS_SIGNAL,serialTime);
		comp_to_event(5,&myclog.PTP2_source_state,FTP.PTP2.source_syn_state&SS_SIGNAL,serialTime);
		//----6----��������״̬
		if((FTP.BDS.source_ant_state&0X0F)!=myclog.BDS_ant_state)
		{
			temp_state = myclog.BDS_ant_state;
			param.event_param[6].old_val = param.event_param[6].now_state;
			param.event_param[6].now_state=myclog.BDS_ant_state=FTP.BDS.source_ant_state&0x0f;
			if(param.event_param[6].flag)
			{
				add_event(6,serialTime);
				if(temp_state==2 || myclog.BDS_ant_state==2)//ֻ���������������������������Ų�����־
					add_event_udp(6,serialTime);
				//UDPֻ��ӳ����ģ���������쳣(bool),��״̬2(����)����������������2��
			}	
		}
		//----7----GPS����״̬
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
				//UDPֻ��ӳ����ģ���������쳣(bool),��״̬2(����)����������������2��
				//myprintf("temp_state=%d,GPS_ant_state=%d\n",temp_state,myclog.GPS_ant_state);
			}	
		}		

		
		//----8----�������ǽ���ģ��״̬
		comp_to_event(8,&myclog.BDS_recv_state,(FTP.BDS.source_ant_state&0X10)>>4,serialTime);
		comp_to_event(9,&myclog.GPS_recv_state,(FTP.GPS.source_ant_state&0X10)>>4,serialTime);

		// 10	ʱ���������״̬(1)�쳣/(0)�ָ�
		// 11	BDSʱ���������״̬(1)�쳣/(0)�ָ�
		// 12	GPSʱ���������״̬(1)�쳣/(0)�ָ�	
		// 13	IRIG-B1(����ʱ���׼)ʱ���������״̬(1)�쳣/(0)�ָ�	
		// 14	IRIG-B2(�ȱ�)ʱ���������״̬(1)�쳣/(0)�ָ�
		// 15	PTP1Դʱ���������״̬(1)�쳣/(0)�ָ�
		// 16	PTP2Դʱ���������״̬(1)�쳣/(0)�ָ�
		comp_to_event(10,&myclog.Lo_TimeAbnormal,FTP.LoTimeAbnormal&0X01,serialTime);	
		comp_to_event(11,&myclog.BDS_TimeAbnormal,(FTP.BDS.source_syn_state&SS_CONTINUITY)>>2,serialTime);	
		comp_to_event(12,&myclog.GPS_TimeAbnormal,(FTP.GPS.source_syn_state&SS_CONTINUITY)>>2,serialTime);
		comp_to_event(13,&myclog.B1_TimeAbnormal,(FTP.IRIG_B1.source_syn_state&SS_CONTINUITY)>>2,serialTime);
		comp_to_event(14,&myclog.B2_TimeAbnormal,(FTP.IRIG_B2.source_syn_state&SS_CONTINUITY)>>2,serialTime);
		comp_to_event(15,&myclog.PTP1_TimeAbnormal,(FTP.PTP1.source_syn_state&SS_CONTINUITY)>>2,serialTime);
		comp_to_event(16,&myclog.PTP2_TimeAbnormal,(FTP.PTP2.source_syn_state&SS_CONTINUITY)>>2,serialTime);
		
		static int err_cnt 	= 0;				 //��ѱ����ľ���ѱ��ʱ�쳣��Ҫ��ʱ60s
		#define SYNC_TIMEOUT 59					//ѱ����ʱʱ��
		if((FTP.Self_Checkstate&0x20)>>5)		//����ѱ���쳣
		{
			if(myclog.Crystal_state==0 && param.event_param[17].flag )
			{	
				if((param.tame&0x1)==0){	//δѱ����
					param.event_param[17].old_val 	= param.event_param[17].now_state;//�ȱ����ֵ
					param.event_param[17].now_state	= 1;	//�ٱ�����ֵ
					myclog.Crystal_state 			= 1;	//��־��ת
					add_event(17,serialTime);
					add_event_udp(17,serialTime);
					param.tame|=0x2;		//װ��״̬λ��0x2Ƶ��Դ�쳣	
				}	
				else{						//��ѱ����
					if(err_cnt==SYNC_TIMEOUT)
					{		//����60S
						param.event_param[17].old_val 	= param.event_param[17].now_state;//�ȱ����ֵ
						param.event_param[17].now_state	= 1;	//�ٱ�����ֵ
						myclog.Crystal_state 			= 1;	//��־��ת
						add_event(17,serialTime);
						add_event_udp(17,serialTime);
						param.tame|=0x2;	
					}
					if(err_cnt<SYNC_TIMEOUT+1)
						err_cnt++;
					if(err_cnt==SYNC_TIMEOUT)
					myprintf("ѱ�������ʱerr_cnt=%d\n",err_cnt);
				}
			}
			
		}
		else{									//����ѱ������
			if(myclog.Crystal_state==1 && param.event_param[17].flag) {
				param.event_param[17].old_val	= param.event_param[17].now_state;
				param.event_param[17].now_state = 0;
				myclog.Crystal_state			= 0;
				add_event(17,serialTime);		//����[18]����ѱ���쳣�ָ�
				add_event_udp(17,serialTime);
			}
			if((param.tame&0x1)==0){
				myprintf("�״ξ���ѱ��\n");
			}
			param.tame=0x1;						//ֻҪ�յ���״̬λ��0x01 ����
			err_cnt=0;
		}
		// 18	��ʼ��״̬(1)�쳣/(0)�ָ�
		// 19	��Դģ��״̬(1)�쳣/(0)�ָ�
		// 20	��Դģ��1״̬(1)�쳣/(0)�ָ�		
		// 21	��Դģ��2״̬(1)�쳣/(0)�ָ�
		comp_to_event(18,&myclog.Init_state,(FTP.Self_Checkstate&0x10)>>4,serialTime);
		comp_to_event(19,&myclog.Power_state,((FTP.Self_Checkstate&0XC0)!=0x00),serialTime);					
		comp_to_event(20,&myclog.P1_power_state,(FTP.Self_Checkstate&0X80)>>7,serialTime);
		comp_to_event(21,&myclog.P2_power_state,(FTP.Self_Checkstate&0X40)>>6,serialTime);
	
		//22	IRIG-B�������������ڱ���
		//myclog.B1_source_stat״̬���ڵ�[2]�иı�Ŀ��ܡ�
		dev_res1=(myclog.B1_source_state==0)&&((FTP.IRIG_B1.timestate&0X0F)>(FTP.LoTimestate&0X0F));
		dev_res2=(myclog.B2_source_state==0)&&((FTP.IRIG_B2.timestate&0X0F)>(FTP.LoTimestate&0X0F));
		if(dev_res1||dev_res2)		//����һ·(��Ч��)B����ڱ���
		{
			if(myclog.IRIG_Q==0)	//���ڱ���
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
			if(myclog.IRIG_Q==1)	//���ڱ����ָ�
			{
				param.event_param[22].old_val  = param.event_param[22].now_state;
				param.event_param[22].now_state=myclog.IRIG_Q=0;
				add_event(22,serialTime);
				add_event_udp(22,serialTime);
			}
		} 
		//----23----���ж���ʱԴ������С��30���ӣ��״�ͬ����
		//----28----���ж���ʱԴ�����ó���30���ӣ��״�ͬ����
		//----24----���ж���ʱԴ������С��24Сʱ������ͬ������
		//----29----���ж���ʱԴ�����ó���24Сʱ������ͬ������
		if(all_untime==1)			//���ж���ʱԴ������
		{		
			if(time_syn<1000000)
				time_syn++;	
			if(myclog.Init_state==1)//�״�ͬ��
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
			else					//�Ѿ�ͬ��
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
		else{	//ʱ��Դ������
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
		//----31---����Ԥ��
		if((FTP.LoTimestate&0x80) >>7 != myclog.Lo_leap_s)
		{
			param.event_param[31].old_val  = param.event_param[31].now_state;
			param.event_param[31].now_state=myclog.Lo_leap_s=(FTP.LoTimestate&0X80)>>7;
			if(param.event_param[31].flag)
			{
				add_event(31,serialTime);
				add_event_udp(31,serialTime);
				if(param.event_param[31].now_state==1){//ֻ�����뷢��ʱ������ֱ�ӱ������ţ������Ƚ�
					param.event_param[36].old_val  = param.event_param[36].now_state;
					param.event_param[36].now_state=myclog.Lo_leap_b=(FTP.LoTimestate&0X40)>>6;
					add_event(36,serialTime);
					add_event_udp(36,serialTime);
				}
			}	
		}
		//myprintf("[32]:%d[37]:%d\n",param.event_param[31].now_state,param.event_param[36].now_state);
		// 32	BDS����Ԥ��
		// 33	GPS����Ԥ��
		// 34	IRIG_B1����Ԥ��
		// 35	IRIG_B2����Ԥ��
		// 36	�����־(�����־����������)
		// 37	ʱ��Դѡ��
		comp_to_event(32,&myclog.BD_leap_s,(FTP.BDS.timestate&0X80)>>7,serialTime);
		comp_to_event(33,&myclog.GPS_leap_s,(FTP.GPS.timestate&0X80)>>7,serialTime);	
		comp_to_event(34,&myclog.B1_leap_s,(FTP.IRIG_B1.timestate&0X80) >> 7,serialTime);	
		comp_to_event(35,&myclog.B2_leap_s,(FTP.IRIG_B2.timestate&0X80) >> 7,serialTime);
		// 36��31��λ���Ѿ����⴦��	
		comp_to_event(37,&myclog.Time_sour,FTP.LoSourceTime,serialTime);
		//---38-45----���ȼ��ı�
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

		// 60	ͬ��״̬
		// 89	BDS �忨״̬ ==>��[8]�ĸ���
		// 91	GPS �忨״̬ ==>��[9]�ĸ���
		
		comp_to_event(60,&myclog.Dev_TB,(FTP.Self_Checkstate&0X08)>>3,serialTime);
		int res_comp;
		res_comp=0;	
		res_comp=comp_to_udpevent(89,&myclog.BDS_board_state,(FTP.BDS.source_ant_state&0X10)>>4,serialTime);
		if(res_comp & param.event_param[90].flag){
			param.event_param[90].old_val = param.event_param[90].now_state;
			if(param.event_param[89].now_state == 1)		//BDS�忨
		    	param.event_param[90].now_state = 0;		//BDS�忨����
			else
				param.event_param[90].now_state = 0x0103;	//BDS�������
			add_event_udp(90,serialTime); 
		}
		res_comp=comp_to_udpevent(91,&myclog.GPS_board_state,(FTP.GPS.source_ant_state&0X10)>>4,serialTime);
		if(res_comp & param.event_param[92].flag){
			param.event_param[92].old_val = param.event_param[92].now_state;
			if(param.event_param[91].now_state == 1)		//GPS�忨
		    	param.event_param[92].now_state = 0;		//GPS�忨����
			else
				param.event_param[92].now_state = 0x0107;	//GPS�������
			add_event_udp(92,serialTime); 
		}
		if(param.initial<3)
			param.initial++;
		//����װ�õĲ���
		
	}
	return 0;
}

int Update_DevLog(void)	//�����־�ļ�����
{
	save_DevLog();		//������־���ڴ�
	if(param.writ_event_flag==1 && param.writ_event_time>=LOG_WRITE_TIMEC)
		write_DevLog(); //������־���ļ�
	if(param.writ_event_time<LOG_WRITE_TIMEC)
		param.writ_event_time++;	
	return 0;			
}

int read_log_flie(void)
{
	LOG_MESSAGE log_buf;//д������־buf
	FILE *fp;
	int i,j;
	char file_buf[LOG_MAX_COUNT*LOG_RECORD_LEN];
	if(param.dir==0)	//���"/mnt/HD/log"�ļ�
		return -1;
	if((fp = fopen(DEV_LOG_FILE,"r")) == NULL)
	{
		sprintf(log_buf.name,"����־ʱ���ļ�ʧ��,��ʼ����־�ļ�");
		add_my_log(log_buf);
		param.event_mem_id=0;
		param.event_count=0;
		param.event_file_siet=0;
		if((fp = fopen(DEV_LOG_FILE,"w+")) == NULL)
		{
			sprintf(log_buf.name,"����־ʱ�����ļ�ʧ��");
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
		sprintf(log_buf.name,"����־�����ļ����ȴ���len=%d",file_len);
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
		printf("��%d��¼��memid=%d,index=%d,val=%d,time=%04d-%02d-%02d %02d:%02d:%02d\n",i,
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
		printf("��%d��¼��memid=%d,index=%d,val=%d,time=%04d-%02d-%02d %02d:%02d:%02d\n",i,
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



