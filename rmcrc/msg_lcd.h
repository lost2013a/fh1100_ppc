#ifndef __MSG_LCD_H
#define __MSG_LCD_H

#include <linux/types.h>
#include "gongyong.h"

typedef struct
{
	int		locate_total_sec;	//本地时间戳
	int		bds_total_sec;		//BDS时间戳
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
	unsigned char 		Zoneinfo;			//时区:B7(0：+，1：-),B6,B5B4B3B2B1B0(时区值)
	unsigned char 		Time_offset_05hour;	//00不设定，01设定；
	unsigned char 		PPXtype;			//0-PPS，1-PPM，2-PPH，3-PPX
	unsigned char 		AM_proportion;		//0-1：2，1-1：3，2-1：4，3-1：5，4-1：6
	Dev_S_Uart 			uart1;
	Dev_S_Uart 			uart2;
	unsigned char 		IRIG_B_verify;		//B7B6B5B4:通道 1IRIG-B 编码校验方式 0-无1-奇 3-偶
	unsigned char 		Master_Slaver;		//00主时钟，01从时钟
	unsigned char 		Source_Mode;		//00多源，01单源？？？？
	unsigned char 		Priority[3];		//时钟源优先级B7~B4 B3~B0   B7~B4   B3~B0
	Dev_S_DelayOffset 	Mod_value;			  
	unsigned char 		Source_outputflag;  //在没有外部源输入时，通过此位使能可以输出
	CTime 				setSystime;			//输出的系统时间初始化设置
}Dev_S_SysMode;

typedef struct //夏令时
{
	unsigned char 		flag;
	XTime  				start;
	XTime 				end;
}Dev_S_TimeSummer;


typedef struct 
{
	unsigned char 	Latitude[6];  //纬度
	unsigned char 	Longitude[6]; //经度
	unsigned char	High[6];      //高度
}Dev_S_BD3D;

typedef struct
{
	unsigned char BDS_Clock_offset[6];		 //BDS源的钟差（秒）
	unsigned char BDS_Locateclock_offset[4]; //BDS源与本地时钟的钟差（纳秒）
	unsigned char GPS_Clock_offset[6];		 //GPS源的钟差（秒）
	unsigned char GPS_Locateclock_offset[4]; //GPS源与本地时钟的钟差（纳秒）
	unsigned char B1_Clock_offset[6];		 //B1源的钟差（秒）
	unsigned char B1_Locateclock_offset[4];  //B1源与本地时钟的钟差（纳秒）
	unsigned char B2_Clock_offset[6];		 //B2源的钟差（秒）
	unsigned char B2_Locateclock_offset[4];  //B2源与本地时钟的钟差（纳秒）
	unsigned char PTP1_Clock_offset[6];		 //PTP1源的钟差（秒）
	unsigned char PTP1_Locateclock_offset[4];//PTP1源与本地时钟的钟差（纳秒）
	unsigned char PTP2_Clock_offset[6];		 //PTP2源的钟差（秒）
	unsigned char PTP2_Locateclock_offset[4];//PTP2源与本地时钟的钟差（纳秒）
}Dev_S_ClkOff;

typedef struct 
{
	Dev_S_ClkOff  offclock;
	unsigned char P1010_Adjust_CO_offset[4];//P1010计算、调整晶振的频率偏差值
	unsigned char P1010_Source_result;		//P1010处理的选源结果值
	unsigned char P1010_SelfCheck;			//P1010处理的自检状态B1（告警灯状态）、B0（故障灯状态）
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
	unsigned char DOMAINNUMBER ;			//13.时钟域
	unsigned char ANNOUNCE_INTERVAL;		//14.Announce报文周期
	unsigned char ANNOUNCE_RECEIPT_TIMEOUT; //15.Announce接收超时时间
	unsigned char SYNC_INTERVAL;			//16.Sync报文周期
	unsigned char PDELAYREQ_INTERVAL;		//17.PDelay报文周期
	unsigned char DELAYREQ_INTERVAL;		//18.Delay报文周期
	unsigned char SLAVE_PRIORITY1 ;			//19.优先级1
	unsigned char SLAVE_PRIORITY2 ;			//20.优先级2
	unsigned char reserve;					//保留
}PTPD_Type;
typedef struct 
{
	unsigned char 	ethernet_mode;		//网络模式 0代表ETH/ETE；1代表ETH-PTP;2代表UDP-ETE;3代表UDP-PTP；4、NTP；
	unsigned char  	Time_Synmode;  		//对时模式
	unsigned char 	spreadmode;			//传输方式		
	unsigned char 	spreadgaps;			//广播间隔
	unsigned char  	master_slaverflag;	//主从配置	(新增的)
	unsigned char 	ip[4];    			//IP地址					
	unsigned char 	mask[4]; 			//子网掩码						
	unsigned char 	wg[4]; 				//网关
	unsigned char 	mac[6];      		//MAC地址	
	unsigned char 	localport[2];		//本端端口号	
	unsigned char 	remoteip[4];		//对端IP地址 
	unsigned char 	remoteport[2];		//对端端口号
	PTPD_Type ptpd;						//PTP结构体	
}NetDev_Type;

//状态控制字
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
	unsigned char BDS_recv_state;			//北斗模块状态
	unsigned char GPS_recv_state;			//GPS模块状态
	unsigned char BDS_board_state;			//北斗板卡状态
	unsigned char GPS_board_state;			//GPS板卡状态
	unsigned char Lo_TimeAbnormal;			//时间跳变侦测
	unsigned char BDS_TimeAbnormal;			//北斗源时间跳变
	unsigned char GPS_TimeAbnormal;			//GPS源时间跳变
	unsigned char B1_TimeAbnormal;
	unsigned char B2_TimeAbnormal;
	unsigned char PTP1_TimeAbnormal;
	unsigned char PTP2_TimeAbnormal;	
	unsigned char Crystal_state;	 		//晶振驯服异常(短时)，对应18
	unsigned char Init_state;				//初始状态
	unsigned char Power_state;				//电源模块
	unsigned char P1_power_state;			//Self_Checkstate
	unsigned char P2_power_state;			//Self_Checkstate
	unsigned char IRIG_Q ;					//irig-b码输入质量低于本机
	unsigned char source_frist_err_30m;		//所有独立时源均不可用,但不超过30分钟（装置首次同步）
	unsigned char source_err_24h;			//所有独立时源均不可用，但不超过24小时（装置曾经同步过）
	unsigned char normal_err;				//其它的可恢复或不影响装置正常运行的故障
	unsigned char core_err;					//CPU等核心板卡异常			
	//unsigned char Crystal_err_60s;			//晶振驯服异常(超60s,一般是没有源同步)对应28
	unsigned char source_frist_err_off_30m; //所有独立时源均不可用,但超过30分钟（装置首次同步）
	unsigned char source_err_off_24h;		//所有独立时源均不可用，但超过24小时（装置曾经同步过）
	unsigned char abnormal_err;				//其它的不可恢复或严重影响装置正常运行的故障
	unsigned char Lo_leap_s;				//闰秒预告
	unsigned char BD_leap_s;
	unsigned char GPS_leap_s;
	unsigned char B1_leap_s;
	unsigned char B2_leap_s;
	unsigned char Lo_leap_b;				//闰秒标志
	unsigned char Time_sour;				//时钟源
	unsigned char Dev_TB;					//装置同步状态
	unsigned int  freque1;					//电网频率1
	unsigned int  freque2;					//电网频率2
}Mclklog;


#endif
