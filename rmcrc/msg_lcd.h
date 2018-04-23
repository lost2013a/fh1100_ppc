#ifndef __MSG_LCD_H
#define __MSG_LCD_H

#include <linux/types.h>
#include "gongyong.h"

typedef struct
{
	int		locate_total_sec;	//����ʱ���
	int		bds_total_sec;		//BDSʱ���
	int		gps_total_sec;
	int		b1_total_sec;
	int		b2_total_sec;
	int		ptp1_total_sec;
	int		ptp2_total_sec;
}PPS_SECOND;




typedef struct
{
	unsigned char 	Uart_baud;
	unsigned char 	Uart_protocol;
}Dev_S_Uart;	

typedef struct
{
	unsigned char BDSoffsetVal[4];
	unsigned char GPSoffsetVal[4];
	unsigned char B1offsetVal[4];
	unsigned char B2offsetVal[4];
	unsigned char PTP1offsetVal[4];
	unsigned char PTP2offsetVal[4];
	unsigned char ClockoffsetVal[4];
	unsigned char PulseoffsetVal[4];
	unsigned char SmesoffsetVal[4];
	unsigned char RIGBoffsetVal[4];
}Dev_S_DelayOffset;

typedef struct 
{
	unsigned char 		Zoneinfo;			//ʱ��:B7(0��+��1��-),B6,B5B4B3B2B1B0(ʱ��ֵ)
	unsigned char 		Time_offset_05hour;	//00���趨��01�趨��
	unsigned char 		PPXtype;			//0-PPS��1-PPM��2-PPH��3-PPX
	unsigned char 		AM_proportion;		//0-1��2��1-1��3��2-1��4��3-1��5��4-1��6
	Dev_S_Uart 			uart1;
	Dev_S_Uart 			uart2;
	unsigned char 		IRIG_B_verify;		//B7B6B5B4:ͨ�� 1IRIG-B ����У�鷽ʽ 0-��1-�� 3-ż
	unsigned char 		Master_Slaver;		//00��ʱ�ӣ�01��ʱ��
	unsigned char 		Source_Mode;		//00��Դ��01��Դ��������
	unsigned char 		Priority[3];		//ʱ��Դ���ȼ�B7~B4 B3~B0   B7~B4   B3~B0
	Dev_S_DelayOffset 	Mod_value;			  
	unsigned char 		Source_outputflag;  //��û���ⲿԴ����ʱ��ͨ����λʹ�ܿ������
	CTime 				setSystime;			//�����ϵͳʱ���ʼ������
}Dev_S_SysMode;

typedef struct //����ʱ
{
	unsigned char 		flag;
	XTime  				start;
	XTime 				end;
}Dev_S_TimeSummer;


typedef struct 
{
	unsigned char 	Latitude[6];  //γ��
	unsigned char 	Longitude[6]; //����
	unsigned char	High[6];      //�߶�
}Dev_S_BD3D;

typedef struct
{
	unsigned char BDS_Clock_offset[6];		 //BDSԴ���Ӳ�룩
	unsigned char BDS_Locateclock_offset[4]; //BDSԴ�뱾��ʱ�ӵ��Ӳ���룩
	unsigned char GPS_Clock_offset[6];		 //GPSԴ���Ӳ�룩
	unsigned char GPS_Locateclock_offset[4]; //GPSԴ�뱾��ʱ�ӵ��Ӳ���룩
	unsigned char B1_Clock_offset[6];		 //B1Դ���Ӳ�룩
	unsigned char B1_Locateclock_offset[4];  //B1Դ�뱾��ʱ�ӵ��Ӳ���룩
	unsigned char B2_Clock_offset[6];		 //B2Դ���Ӳ�룩
	unsigned char B2_Locateclock_offset[4];  //B2Դ�뱾��ʱ�ӵ��Ӳ���룩
	unsigned char PTP1_Clock_offset[6];		 //PTP1Դ���Ӳ�룩
	unsigned char PTP1_Locateclock_offset[4];//PTP1Դ�뱾��ʱ�ӵ��Ӳ���룩
	unsigned char PTP2_Clock_offset[6];		 //PTP2Դ���Ӳ�룩
	unsigned char PTP2_Locateclock_offset[4];//PTP2Դ�뱾��ʱ�ӵ��Ӳ���룩
}Dev_S_ClkOff;

typedef struct 
{
	Dev_S_ClkOff  offclock;
	unsigned char P1010_Adjust_CO_offset[4];//P1010���㡢���������Ƶ��ƫ��ֵ
	unsigned char P1010_Source_result;		//P1010�����ѡԴ���ֵ
	unsigned char P1010_SelfCheck;			//P1010������Լ�״̬B1���澯��״̬����B0�����ϵ�״̬��
}Dev_S_FPGA2MSG;


typedef struct 
{
	unsigned char		Check_head[4];
	unsigned char		Packet_len;
	Dev_S_SysMode 		Sysset;
	Dev_S_TimeSummer 	summerTime;
	Dev_S_BD3D			Bd3Dset;	
	Dev_S_FPGA2MSG    	Dyn_FPGA_message; 
	unsigned char       crc;
}PPC2FPGA_MSG;

typedef struct 
{		
	unsigned char DOMAINNUMBER ;			//13.ʱ����
	unsigned char ANNOUNCE_INTERVAL;		//14.Announce��������
	unsigned char ANNOUNCE_RECEIPT_TIMEOUT; //15.Announce���ճ�ʱʱ��
	unsigned char SYNC_INTERVAL;			//16.Sync��������
	unsigned char PDELAYREQ_INTERVAL;		//17.PDelay��������
	unsigned char DELAYREQ_INTERVAL;		//18.Delay��������
	unsigned char SLAVE_PRIORITY1 ;			//19.���ȼ�1
	unsigned char SLAVE_PRIORITY2 ;			//20.���ȼ�2
	unsigned char reserve;					//����
}PTPD_Type;
typedef struct 
{
	unsigned char 	ethernet_mode;		//����ģʽ 0����ETH/ETE��1����ETH-PTP;2����UDP-ETE;3����UDP-PTP��4��NTP��
	unsigned char  	Time_Synmode;  		//��ʱģʽ
	unsigned char 	spreadmode;			//���䷽ʽ		
	unsigned char 	spreadgaps;			//�㲥���
	unsigned char  	master_slaverflag;	//��������	(������)
	unsigned char 	ip[4];    			//IP��ַ					
	unsigned char 	mask[4]; 			//��������						
	unsigned char 	wg[4]; 				//����
	unsigned char 	mac[6];      		//MAC��ַ	
	unsigned char 	localport[2];		//���˶˿ں�	
	unsigned char 	remoteip[4];		//�Զ�IP��ַ 
	unsigned char 	remoteport[2];		//�Զ˶˿ں�
	PTPD_Type ptpd;						//PTP�ṹ��	
}NetDev_Type;

//״̬������
typedef struct 
{
	unsigned char	global_control1;	
	unsigned char	global_control2;
	unsigned char	bds_control;
	unsigned char	gps_control;
	unsigned char	b1_control;
	unsigned char	b2_control;
	unsigned char	ptp1_control;
	unsigned char	ptp2_control;
	unsigned char	bak_control1;
	unsigned char	bak_control2;
	unsigned char	fault_control;
	unsigned char	warning_control;
	unsigned char	bak_control3;
	unsigned char	bak_control4;
	unsigned char	bak_control5;
	unsigned char	bak_control6;
}Dev_S_CtlWord;
typedef struct 
{		
	Dev_S_SysMode 		Sysset;	
	Dev_S_TimeSummer 	summerTime;
	Dev_S_BD3D			Bd3Dset;
	unsigned char 		Slot_State[15];	
	Dev_S_CtlWord		control;
	NetDev_Type			Net_set1;
	NetDev_Type			Net_set2;
	NetDev_Type			Net_set3;
	NetDev_Type			Net_set4;
	NetDev_Type			Net_set5;
	NetDev_Type 		Net_set6;
}LCD2PPC_MSG;

typedef struct 
{
	unsigned char BDS_source_state;			//source_syn_state
	unsigned char GPS_source_state;			//source_syn_state
	unsigned char B1_source_state;			//FpgaToPowerpc.IRIG_B1.source_syn_state B0
	unsigned char B2_source_state;			//FpgaToPowerpc.IRIG_B1.source_syn_state B0
	unsigned char PTP1_source_state;
	unsigned char PTP2_source_state;	
	unsigned char BDS_ant_state;
	unsigned char GPS_ant_state;
	unsigned char BDS_recv_state;			//����ģ��״̬
	unsigned char GPS_recv_state;			//GPSģ��״̬
	unsigned char BDS_board_state;			//�����忨״̬
	unsigned char GPS_board_state;			//GPS�忨״̬
	unsigned char Lo_TimeAbnormal;			//ʱ���������
	unsigned char BDS_TimeAbnormal;			//����Դʱ������
	unsigned char GPS_TimeAbnormal;			//GPSԴʱ������
	unsigned char B1_TimeAbnormal;
	unsigned char B2_TimeAbnormal;
	unsigned char PTP1_TimeAbnormal;
	unsigned char PTP2_TimeAbnormal;	
	unsigned char Crystal_state;	 		//����ѱ���쳣(��ʱ)����Ӧ18
	unsigned char Init_state;				//��ʼ״̬
	unsigned char Power_state;				//��Դģ��
	unsigned char P1_power_state;			//Self_Checkstate
	unsigned char P2_power_state;			//Self_Checkstate
	unsigned char IRIG_Q ;					//irig-b�������������ڱ���
	unsigned char source_frist_err_30m;		//���ж���ʱԴ��������,��������30���ӣ�װ���״�ͬ����
	unsigned char source_err_24h;			//���ж���ʱԴ�������ã���������24Сʱ��װ������ͬ������
	unsigned char normal_err;				//�����Ŀɻָ���Ӱ��װ���������еĹ���
	unsigned char core_err;					//CPU�Ⱥ��İ忨�쳣			
	//unsigned char Crystal_err_60s;			//����ѱ���쳣(��60s,һ����û��Դͬ��)��Ӧ28
	unsigned char source_frist_err_off_30m; //���ж���ʱԴ��������,������30���ӣ�װ���״�ͬ����
	unsigned char source_err_off_24h;		//���ж���ʱԴ�������ã�������24Сʱ��װ������ͬ������
	unsigned char abnormal_err;				//�����Ĳ��ɻָ�������Ӱ��װ���������еĹ���
	unsigned char Lo_leap_s;				//����Ԥ��
	unsigned char BD_leap_s;
	unsigned char GPS_leap_s;
	unsigned char B1_leap_s;
	unsigned char B2_leap_s;
	unsigned char Lo_leap_b;				//�����־
	unsigned char Time_sour;				//ʱ��Դ
	unsigned char Dev_TB;					//װ��ͬ��״̬
	unsigned int  freque1;					//����Ƶ��1
	unsigned int  freque2;					//����Ƶ��2
}Mclklog;


#endif
