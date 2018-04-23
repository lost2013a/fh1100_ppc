#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h> 
#include <linux/types.h>
#include <dirent.h>
#include "bsp.h"
#include "gongyong.h"

char* bsp_net[6]={
	"eth0.10",
	"eth0.11",
	"eth0.12",
	"eth0.13",
	"eth2",
	"eth1",
};

int	open_log_flag=0;	//������־������ͬ������
int	prog_log_count=0;	//������־�ļ���

LOG_MESSAGE log_list[100];		//������־�����б�
extern PARAM param;


/*
	��ʼ�����ã�����set_eth ��0 ����Ҫ���������IP����Ϣ��
*/

void dev_console_init(void)
{
	int i;
	for(i=0;i<UDP_COUNT;i++)//62 93 event_param�Ǵ�0��ʼ��
	{
		
		if(i<46 || i>58)
			param.event_param[i].flag=1;	//Ĭ����Ч��״̬����λ
		else
			param.event_param[i].flag=0;	//Ĭ�Ϲر�	
		//param.event_param[i].now_state=0;
		param.event_param[i].index = i+1;	//��1��ʼ������0
	}
                                                                                                                                                                       
	
	//---------BDS ctrl byte
	if((file_param.control.global_control2&0x01)==0)
	{
		param.event_param[0].flag=0;
		param.event_param[6].flag=0;
		param.event_param[8].flag=0;
		param.event_param[11].flag=0;
		param.event_param[40].flag=0;
		//param.event_param[43].flag=0;
		//param.event_param[89].flag=0;
		//param.event_param[90].flag=0;
		param.event_param[93].flag=0;
		param.event_param[100].flag=0;
		param.event_param[101].flag=0;
		param.event_param[118].flag=0;
		param.event_param[119].flag=0;
	}
	else
	{
		if((file_param.control.bds_control&0x01)==0)
			param.event_param[0].flag=0;
		if((file_param.control.bds_control&0x02)==0)
			param.event_param[6].flag=0;	
		if((file_param.control.bds_control&0x04)==0)
			param.event_param[8].flag=0;	
		if((file_param.control.bds_control&0x08)==0)
			param.event_param[11].flag=0;
		if((file_param.control.bds_control&0x10)==0)
		{	
			param.event_param[100].flag=0;
			param.event_param[101].flag=0;
		}	
		if((file_param.control.bds_control&0x40)==0)
			param.event_param[119].flag=0;
	}	
	//--------------GPS ctrl byte
	if((file_param.control.global_control2&0x02)==0)
	{
		param.event_param[1].flag=0;
		param.event_param[7].flag=0;
		param.event_param[9].flag=0;
		param.event_param[12].flag=0;
		param.event_param[41].flag=0;
		//param.event_param[42].flag=0;
		//param.event_param[91].flag=0;
		//param.event_param[92].flag=0;
		param.event_param[94].flag=0;
		param.event_param[102].flag=0;
		param.event_param[103].flag=0;
		param.event_param[120].flag=0;
		param.event_param[121].flag=0;
	}
	else
	{
		if((file_param.control.gps_control&0x01)==0)
			param.event_param[1].flag=0;
		if((file_param.control.gps_control&0x02)==0)
			param.event_param[7].flag=0;
		if((file_param.control.gps_control&0x04)==0)
			param.event_param[9].flag=0;
		if((file_param.control.gps_control&0x08)==0)
			param.event_param[12].flag=0;	
		if((file_param.control.gps_control&0x10)==0)
		{	
			param.event_param[102].flag=0;		
			param.event_param[103].flag=0;	
		}	
		if((file_param.control.gps_control&0x40)==0)
			param.event_param[121].flag=0;
	}	
	//----------B1 ctrl byte
	if((file_param.control.global_control2&0x04)==0)
	{
		param.event_param[2].flag=0;
		param.event_param[13].flag=0;
		//param.event_param[44].flag=0;
		param.event_param[95].flag=0;
		param.event_param[104].flag=0;
		param.event_param[105].flag=0;
		param.event_param[122].flag=0;
		param.event_param[123].flag=0;
	}
	else
	{
		if((file_param.control.b1_control&0x01)==0)
			param.event_param[2].flag=0;
		if((file_param.control.b1_control&0x08)==0)
			param.event_param[13].flag=0;	
		if((file_param.control.b1_control&0x10)==0)
		{	
			param.event_param[104].flag=0;	
			param.event_param[105].flag=0;	
		}	
		if((file_param.control.b1_control&0x40)==0)
			param.event_param[123].flag=0;
	}	
	//--------B2 ctrl byte
	if((file_param.control.global_control2&0x08)==0)
	{
		param.event_param[3].flag=0;
		param.event_param[14].flag=0;
		//param.event_param[45].flag=0;
		param.event_param[96].flag=0;
		param.event_param[106].flag=0;
		param.event_param[107].flag=0;
		param.event_param[124].flag=0;
		param.event_param[125].flag=0;
	}
	else
	{
		if((file_param.control.b2_control&0x01)==0)
			param.event_param[3].flag=0;
		if((file_param.control.b2_control&0x08)==0)
			param.event_param[14].flag=0;	
		if((file_param.control.b2_control&0x10)==0)
		{	
			param.event_param[106].flag=0;	
			param.event_param[107].flag=0;	
		}	
		if((file_param.control.b2_control&0x40)==0)
			param.event_param[125].flag=0;
	}	
	//--------ptp1 ctrl byte
	if((file_param.control.global_control2&0x10)==0)
	{
		param.event_param[4].flag=0;
		param.event_param[15].flag=0;
		param.event_param[97].flag=0;
		param.event_param[108].flag=0;
		param.event_param[109].flag=0;
	}
	else
	{
		if((file_param.control.ptp1_control&0x01)==0)
			param.event_param[4].flag=0;
		if((file_param.control.ptp1_control&0x08)==0)
			param.event_param[15].flag=0;	
		if((file_param.control.ptp1_control&0x10)==0)
		{	
			param.event_param[108].flag=0;	
			param.event_param[109].flag=0;	
		}	
	}	
	//--------ptp2 ctrl byte
	if((file_param.control.global_control2&0x20)==0)
	{
		param.event_param[5].flag=0;
		param.event_param[16].flag=0;
		param.event_param[98].flag=0;
		param.event_param[110].flag=0;
		param.event_param[111].flag=0;
	}
	else
	{
		if((file_param.control.ptp2_control&0x01)==0)
			param.event_param[5].flag=0;
		if((file_param.control.ptp2_control&0x08)==0)
			param.event_param[16].flag=0;	
		if((file_param.control.ptp2_control&0x10)==0)
		{	
			param.event_param[110].flag=0;	
			param.event_param[111].flag=0;	
		}	
	}	
	//��Դ״̬
	if((file_param.control.warning_control&0x80)==0)
	{
		param.event_param[19].flag=0;
		param.event_param[20].flag=0;
		param.event_param[21].flag=0;
	}	
}


void main_initial(char set_eth)
{
	LOG_MESSAGE log_buf;//д������־buf
	int	i;
	int ret;
	ret = read_param();	//���ļ��ж�ȡ������Ϣ
	if(ret==0)			//��ȡ�ɹ�
	{	
		sprintf(log_buf.name,"װ�ò�����ʼ���ɹ�");
		add_my_log(log_buf);
	}	
	else				//��ȡʧ�ܣ�ʹ��һ��Ĭ�ϵ�����
	{	
		param.man_flag = 0;
		param.IP1[0]=192;
		param.IP1[1]=168;
		param.IP1[2]=7;
		param.IP1[3]=162;
		param.bad_priorty = 1;
		param.gps_priorty = 2;
		param.b1_priorty  = 3;
		param.b2_priorty  = 4;
		param.PTP1_priorty  = 5;
		param.PTP2_priorty  = 6;
		memcpy(&param.IP1,&file_param.Net_set5.ip,4);	//���ñ���IP
		sprintf(log_buf.name,"װ�ò�����ȡʧ�ܣ�ʹ�û�������");
		add_my_log(log_buf);
	}
	//��ʼ�����ú�״̬
	//0-60��Ĭ��0
	for(i=0;i<61;i++)
	{
		param.event_param[i].now_state=0;
	}
	//61-92������read_board ,
	//96-126��Ĭ��0
	for(i=93;i<UDP_COUNT;i++)
	{
		param.event_param[i].now_state=0;
	}
	param.rx_serial=0;	//��δ�յ�����
	param.initial=0;	//��ʼ��δ���
	param.event_param[43].now_state =param.bad_priorty;
    param.event_param[42].now_state =param.gps_priorty;
    param.event_param[44].now_state =param.b1_priorty;
    param.event_param[45].now_state =param.b2_priorty;
    param.event_param[38].now_state =param.PTP1_priorty;
 	param.event_param[39].now_state =param.PTP2_priorty;
	myclog.freque1 = 500000;
	myclog.freque2 = 500000;	
	
	if(set_eth)			//��Ҫ��������IP
	{	
		ret=SetIfAddr(bsp_net[0], file_param.Net_set1.ip, file_param.Net_set1.mask,file_param.Net_set1.wg);
		if(ret<0)
		{
			sprintf(log_buf.name,"����[%s]��ʼ����ʧ�ܣ�error code=%d",bsp_net[0],ret);
			add_my_log(log_buf);
		}
	
		ret=SetIfAddr(bsp_net[1], file_param.Net_set2.ip, file_param.Net_set2.mask,file_param.Net_set2.wg);
		if(ret<0)
		{
			sprintf(log_buf.name,"����[%s]��ʼ����ʧ�ܣ�error code=%d",bsp_net[1],ret);
			add_my_log(log_buf);
		}
			
		ret=SetIfAddr(bsp_net[2], file_param.Net_set3.ip, file_param.Net_set3.mask,file_param.Net_set3.wg);
		if(ret<0)
		{
			sprintf(log_buf.name,"����[%s]��ʼ����ʧ�ܣ�error code=%d",bsp_net[2],ret);
			add_my_log(log_buf);
		}
		
		ret=SetIfAddr(bsp_net[3], file_param.Net_set4.ip, file_param.Net_set4.mask,file_param.Net_set4.wg);
		if(ret<0)
		{
			sprintf(log_buf.name,"����[%s]��ʼ����ʧ�ܣ�error code=%d",bsp_net[3],ret);
			add_my_log(log_buf);
		}
		
		ret=SetIfAddr(bsp_net[4], file_param.Net_set5.ip, file_param.Net_set5.mask,file_param.Net_set5.wg);
		if(ret<0)
		{
			sprintf(log_buf.name,"����[%s]��ʼ����ʧ�ܣ�error code=%d",bsp_net[4],ret);
			add_my_log(log_buf);
		}
		
		ret=SetIfAddr(bsp_net[5], file_param.Net_set6.ip, file_param.Net_set6.mask,file_param.Net_set6.wg);
		if(ret<0)
		{
			sprintf(log_buf.name,"����[%s]��ʼ����ʧ�ܣ�error code=%d",bsp_net[5],ret);
			add_my_log(log_buf);
		}
	}
}

static void set_clog_to(unsigned char num,unsigned char *myclog_state,unsigned char setval)
{
	if(param.event_param[num].flag)
		*myclog_state = setval;
	else
		*myclog_state = 0;
	param.event_param[num].now_state = *myclog_state;
}
void set_initial(char all_untime)
{

	char  dev_res1,dev_res2;
	//myprintf("��ʼ��״̬(���������־),��ȡ�µ�״̬���ж�����\n");
	// 0	BDSԴ�ź�(1)�쳣/(0)�ָ�
	// 1	GPSԴ�ź�(1)�쳣/(0)�ָ�
	// 2	IRIG-B1(����ʱ���׼)�ź�(1)�쳣/(0)�ָ�
	// 3	IRIG-B2���ȱ����ź�(1)�쳣/(0)�ָ�
	// 4	PTP1Դ�ź�(1)�쳣/(0)�ָ�
	set_clog_to(0,&myclog.BDS_source_state,FTP.BDS.source_syn_state&SS_SIGNAL);
	set_clog_to(1,&myclog.GPS_source_state,FTP.GPS.source_syn_state&SS_SIGNAL);
	set_clog_to(2,&myclog.B1_source_state,FTP.IRIG_B1.source_syn_state&SS_SIGNAL);	
	set_clog_to(3,&myclog.B2_source_state,FTP.IRIG_B2.source_syn_state&SS_SIGNAL);
	set_clog_to(4,&myclog.PTP1_source_state,FTP.PTP1.source_syn_state&SS_SIGNAL);
	
	// 5	PTP2Դ�ź�(1)�쳣/(0)�ָ�
	// 6	BDS����״̬(1)�쳣/(0)�ָ�
	// 7	GPS����״̬(1)�쳣/(0)�ָ�		
	// 8	BDS����ģ��״̬(1)�쳣/(0)�ָ�
	// 9	GPS����ģ��״̬(1)�쳣/(0)�ָ�
	set_clog_to(5,&myclog.PTP2_source_state,FTP.PTP2.source_syn_state&SS_SIGNAL);
	set_clog_to(6,&myclog.BDS_ant_state,FTP.BDS.source_ant_state&0x0f);		
	set_clog_to(7,&myclog.GPS_ant_state,FTP.GPS.source_ant_state&0x0f);	
	

	
	set_clog_to(8,&myclog.BDS_recv_state,(FTP.BDS.source_ant_state&0X10)>>4);
	set_clog_to(9,&myclog.GPS_recv_state,(FTP.GPS.source_ant_state&0X10)>>4);
	
	// 10	ʱ���������״̬(1)�쳣/(0)�ָ�
	// 11	BDSʱ���������״̬(1)�쳣/(0)�ָ�
	// 12	GPSʱ���������״̬(1)�쳣/(0)�ָ�	
	// 13	IRIG-B1(����ʱ���׼)ʱ���������״̬(1)�쳣/(0)�ָ�
	// 14	IRIG-B2(�ȱ�)ʱ���������״̬(1)�쳣/(0)�ָ�
	set_clog_to(10,&myclog.Lo_TimeAbnormal,FTP.LoTimeAbnormal&0X01);	
	set_clog_to(11,&myclog.BDS_TimeAbnormal,(FTP.BDS.source_syn_state&SS_CONTINUITY)>>2);		
	set_clog_to(12,&myclog.GPS_TimeAbnormal,(FTP.GPS.source_syn_state&SS_CONTINUITY)>>2);
	set_clog_to(13,&myclog.B1_TimeAbnormal,(FTP.IRIG_B1.source_syn_state&SS_CONTINUITY)>>2);
	set_clog_to(14,&myclog.B2_TimeAbnormal,(FTP.IRIG_B2.source_syn_state&SS_CONTINUITY)>>2);
	
	// 15	PTP1Դʱ���������״̬(1)�쳣/(0)�ָ�
	// 16	PTP2Դʱ���������״̬(1)�쳣/(0)�ָ�		
	set_clog_to(15,&myclog.PTP1_TimeAbnormal,(FTP.PTP1.source_syn_state&SS_CONTINUITY)>>2);
	set_clog_to(16,&myclog.PTP2_TimeAbnormal,(FTP.PTP2.source_syn_state&SS_CONTINUITY)>>2);	
	// 17	����ѱ��״̬(1)�쳣/(0)�ָ�
	if((FTP.Self_Checkstate&0x20)>>5 && param.event_param[17].flag){
		param.event_param[17].now_state=myclog.Crystal_state=1;
	}
	else{
		param.event_param[17].now_state=myclog.Crystal_state=0;
	}
	// 18	��ʼ��״̬(1)�쳣/(0)�ָ�
	// 19	��Դģ��״̬(1)�쳣/(0)�ָ�
	// 20	��Դģ��1״̬(1)�쳣/(0)�ָ�		
	// 21	��Դģ��2״̬(1)�쳣/(0)�ָ�
	set_clog_to(18,&myclog.Init_state,(FTP.Self_Checkstate&0x10)>>4);
	set_clog_to(19,&myclog.Power_state,((FTP.Self_Checkstate&0XC0)!=0x00));				
	set_clog_to(20,&myclog.P1_power_state,(FTP.Self_Checkstate&0X80)>>7);
	set_clog_to(21,&myclog.P2_power_state,(FTP.Self_Checkstate&0X40)>>6);
	
	// 22	IRIG-B�������������ڱ���  
	//myclog.B1_source_stat״̬���ڵ�[2]�иı�Ŀ��ܣ�dev_res1,dev_res2ֵ�����ʱ���жϱ������ڵ�[2]���ж�
	dev_res1=(myclog.B1_source_state==0)&&((FTP.IRIG_B1.timestate&0X0F)>(FTP.LoTimestate&0X0F));
	dev_res2=(myclog.B2_source_state==0)&&((FTP.IRIG_B2.timestate&0X0F)>(FTP.LoTimestate&0X0F));
	if((dev_res1||dev_res2)&&param.event_param[22].flag)
		 param.event_param[22].now_state=myclog.IRIG_Q=1;
	else
		 param.event_param[22].now_state=myclog.IRIG_Q=0;		
	// 23	���ж���ʱԴ������С��30���ӣ��״�ͬ����
	// 24	���ж���ʱԴ������С��24Сʱ������ͬ������
	// 25	�����ɻָ�����
	// 26	CPU�Ⱥ��İ忨�쳣
	// 27	����ѱ��״̬�쳣����60��
	// 28	���ж���ʱԴ�����ó���30���ӣ��״�ͬ����
	// 29	���ж���ʱԴ�����ó���24Сʱ������ͬ������
	if(all_untime)
	{
		if(param.event_param[23].flag && myclog.Init_state==1)
			param.event_param[23].now_state=myclog.source_frist_err_30m = 1;
		else
			param.event_param[23].now_state=myclog.source_frist_err_30m = 0;	
		if(param.event_param[24].flag && myclog.Init_state==0)	
			param.event_param[24].now_state=myclog.source_err_24h=1;
		else
			param.event_param[24].now_state=myclog.source_err_24h=0;	
	}
	else
	{
		param.event_param[23].now_state=myclog.source_frist_err_30m=0;
		param.event_param[24].now_state=myclog.source_err_24h=0;
	}
	param.event_param[28].now_state=myclog.source_frist_err_off_30m=0;
	param.event_param[29].now_state=myclog.source_err_off_24h=0;
	// 31	����Ԥ��
	// 32	BDS����Ԥ��
	// 33	GPS����Ԥ��
	// 34	IRIG_B1����Ԥ��
	// 35	IRIG_B2����Ԥ�� 
	// 36	�����־
	// 37	ʱ��Դѡ��
	set_clog_to(31,&myclog.Lo_leap_s,(FTP.LoTimestate&0X80)>>7);
	set_clog_to(32,&myclog.BD_leap_s,(FTP.BDS.timestate&0X80)>>7);
	set_clog_to(33,&myclog.GPS_leap_s,(FTP.GPS.timestate&0X80)>>7);	
	set_clog_to(34,&myclog.B1_leap_s,(FTP.IRIG_B1.timestate&0X80) >> 7);	
	set_clog_to(35,&myclog.B2_leap_s,(FTP.IRIG_B2.timestate&0X80) >> 7);
	set_clog_to(36,&myclog.Lo_leap_b,(FTP.LoTimestate&0X40)>>6);	
	set_clog_to(37,&myclog.Time_sour,FTP.LoSourceTime);
	// 60	ͬ��״̬
	set_clog_to(60,&myclog.Dev_TB,(FTP.Self_Checkstate&0X08)>>3);
	// 89	BDS �忨״̬ ==>��[8]�ĸ���
	// 91	GPS �忨״̬ ==>��[9]�ĸ���
	set_clog_to(89,&myclog.BDS_board_state,(FTP.BDS.source_ant_state&0X10)>>4);
	set_clog_to(91,&myclog.GPS_board_state,(FTP.GPS.source_ant_state&0X10)>>4);
}


void read_time(void)	//��ȡϵͳʱ�䵽sys_time
{
	struct tm  *sysTime;
	struct timeval  tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);
	handleap(&tv);
	sysTime   = localtime(&tv.tv_sec);
	sys_time.year = sysTime->tm_year +1900;
	sys_time.mon  = sysTime->tm_mon+1;
	sys_time.day  = sysTime->tm_mday;
	sys_time.hour = sysTime->tm_hour;
	sys_time.min  = sysTime->tm_min;
	sys_time.sec  = sysTime->tm_sec;
	sys_time.total_sec = tv.tv_sec;
	sys_time.msec      = tv.tv_usec/1000;
	sys_time.usec      = tv.tv_usec;
	
}




int CreateDir(void)     //����"/mnt/HD/log"Ŀ¼
{
	char DirName[256];   
	strcpy(DirName, DEBUG_LOG_FILE);   
	unsigned char   i,len = strlen(DirName);   
	if(DirName[len-1] != '/')   //����'/'
	{
		strcat(DirName,   "/");
	}
	len = strlen(DirName);   
	for(i = 1; i<len; i++)   
	{   
		if(DirName[i] == '/')   		
		{   
			DirName[i] = 0;   					//��ȡ��Ŀ¼��
			if(access(DirName,F_OK) != 0)   	//����ļ���Ȩ��(�Ƿ����)
			{ 
				if(mkdir(DirName, 0755) == -1)	//��755Ȩ���½�Ŀ¼   
				{   
			        perror("mkdir   error");  
			        return   -1;  
				}   
			}   
			DirName[i] = '/';   
		}   
	}   
	if(is_dir_exist(config_dir) == -1)	
		create_config_dir(config_dir);
	return   0;   
}

int create_config_dir(const char *dir)
{
	int ok = 0; 
   	if((ok = mkdir(dir,0755)) != 0)  
   	{  
		printf("mkdir %s\n", dir);  
		return 0;
   	}
	return 1;
}

int is_dir_exist(const char *dir_path)
{
	if(dir_path ==  NULL)
		return -1;
	if(opendir(dir_path) == NULL)
		return -1;
	return 0;
}



int	read_board_state(void)
{
	int   i;
	FILE *fp;
	unsigned char file_buff[15],type;
	char file_name[CONFIG_PATH] =  DEV_BOARD_STA_FILE;
	
	for(i=0;i<15;i++)
		file_buff[i]=1;
	if((fp = fopen(file_name,"rb")) != NULL)//�Զ����ƴ�
	{
		fread(file_buff, 1,15, fp);			 //��ʧ��
		fclose(fp);
	}	
	for(i=0;i<14;i++)
	{
		type=file_buff[i];
		param.event_param[2*i+OFF_E_BoardState].now_state=type;
		param.event_param[2*i+OFF_E_BoardState+1].now_state=board_type_its[type];
		//myprintf("r[%02d]:%02x,%08x\n",i,file_buff[i],board_type_its[type]);
	}
	return 0;	
}

int save_board_state(void)
{
	int 	i;
	FILE *fp;
	unsigned char file_buff[15];
	char file_name[CONFIG_PATH] =  DEV_BOARD_STA_FILE;
	
	for(i=0;i<14;i++){
		file_buff[i]=param.event_param[2*i+OFF_E_BoardState].now_state;
		//myprintf("w[%02d]:%02x\n",i,file_buff[i]);
	}
	if((fp = fopen(file_name,"w+")) == NULL)
		return -1;
	fwrite(file_buff, 1,15, fp);
	/*
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);	
	*/
	direct_write(fp,"write_boardstat");
	return 0;	 	
}

void create_log(int init)
{
	char file_name[256];
	FILE *file_p;
	if(param.dir)//���"/mnt/HD/log"�ļ�
	{
		pthread_mutex_lock(&lock_log);		//+
		if(init>0){
			sprintf(file_name,"%s/prog_log.%02d",DEBUG_LOG_FILE,sys_time.day);
		}
		else{
			sprintf(file_name,"%s/prog_log.init",DEBUG_LOG_FILE);
		}
		file_p = fopen(file_name, "w+");//д��ʱ������ļ����ڣ��ᱻ��գ���ͷ��ʼд
		fclose(file_p);
		pthread_mutex_unlock(&lock_log);	//-
	}	
	
}



void add_my_log(LOG_MESSAGE log_message)
{
	pthread_mutex_lock(&lock_log); 		//+
	if(prog_log_count<100){
		memcpy(&log_list[prog_log_count],&log_message,sizeof(LOG_MESSAGE));
		prog_log_count++;
		if(param.print_prog_log){
			myprintf("--prog_log(%d):[%02d.%02d]%s\n",prog_log_count,sys_time.min,sys_time.sec,log_message.name);
		}
	}
	pthread_mutex_unlock(&lock_log);	//-
}


void save_my_log(void)	//���������־
{
	int i,cnt;
	char buf[300];
	char file_name[256]={0};
	FILE *file_p;
	if(param.dir==0)//���"/mnt/HD/log"�ļ�
		return;
	pthread_mutex_lock(&lock_log);		//+
	if(prog_log_count==0){				//��������ж�һ��
		pthread_mutex_unlock(&lock_log);//-
		return;
	}
	if(time_acquired==0){	
		sprintf(file_name,"%s/prog_log.init",DEBUG_LOG_FILE);//��ϵͳ���ڱ���
	}
	else{
		sprintf(file_name,"%s/prog_log.%02d",DEBUG_LOG_FILE,sys_time.day);//��ϵͳ���ڱ���
	}
	file_p = fopen(file_name, "a+");	//�ļ������ڣ��򴴽�
	for(cnt=0;cnt<prog_log_count;cnt++){
		sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d ",sys_time.year,sys_time.mon,sys_time.day,sys_time.hour,sys_time.min,sys_time.sec);
		memcpy(&buf[20],&log_list[cnt].name,MAX_LOGMSG_LEN);
		for(i=0;i<MAX_LOGMSG_LEN-4;i++){//ǿ�ƽضϲ�����0d,0a����
			if(buf[i]==0)
				break;
		}
		buf[++i]=0x0d;
		buf[++i]=0x0a;
		buf[++i]=0;
		if (file_p!=NULL){
			fwrite(&buf,1,i,file_p);
		}
	}
	prog_log_count=0;
	fclose(file_p);
	pthread_mutex_unlock(&lock_log);//-
}


void normalizeTime(TimeInternal *r)//���㸺��
{
	r->seconds += r->useconds / 1000000;	
	r->useconds -= (r->useconds / 1000000) * 1000000;

	if (r->seconds > 0 && r->useconds < 0) {
		r->seconds -= 1;
		r->useconds += 1000000;
	} else if (r->seconds < 0 && r->useconds > 0) {
		r->seconds += 1;
		r->useconds -= 1000000;
	}
}

void subTime(TimeInternal *r,TimeInternal *x,TimeInternal *y)
{
	r->seconds = x->seconds - y->seconds;
	r->useconds = x->useconds - y->useconds;
	normalizeTime(r);
}

void direct_write(FILE *fp,const char *type)
{
	char tmp = *type;
	tmp = ~tmp;	//����༭���澯
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
}





