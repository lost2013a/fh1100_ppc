#ifndef __MSG_FPGA_H
#define __MSG_FPGA_H
#include <linux/types.h>
#include "gongyong.h"

#define	PPS_K 8	//FPGA中一个周期对应的ns
/*时间源状态的位*/
#define	SS_SIGNAL  		0x01		//信号状态 0正常
#define	SS_SYNC  		0x02		//同步状态
#define	SS_CONTINUITY  	0x04		//连续性



typedef struct
{
	unsigned char 	bds_flag;	//信号(失锁)标志
	unsigned char 	gps_flag; 
	unsigned char 	b1_flag;
	unsigned char 	b2_flag;
	unsigned char 	ptp1_flag;
	unsigned char 	ptp2_flag;
	unsigned int  	bds_sec;	//信号失锁的时间记录，单位秒
	unsigned int  	gps_sec;
	unsigned int  	b1_sec;
	unsigned int  	b2_sec;
	unsigned int  	ptp1_sec;
	unsigned int  	ptp2_sec;
}__attribute__((packed)) LOSE_LOCK;


typedef struct 
{
	CTime			time;			 //大时间
	unsigned char	zoneinfo;		 //时区 
	unsigned char	timestate;		 //源状态信息,b7闰秒预告,b6闰秒标志
	unsigned char  	satellites;		 //卫星数
	unsigned char	source_ant_state;//模块状态0未初始化,1未知,2正常,3短路,4开路
	unsigned char	source_syn_state;//时间源状态b2连续性b1同步b0信号
	unsigned char 	latitude[6];     //纬度
	unsigned char 	longitude[6];    //经度
	unsigned char 	high[6];         //高度
	unsigned char 	ppsErrval[4];	 //PPS的相对钟差值
	unsigned char 	TBD[2];
}__attribute__((packed)) sourcestate;

typedef struct 
{
	CTime			time;		 //大时间
	unsigned char	zoneinfo;	 //时区
	unsigned char	timestate;	 //BD源状态信息
	unsigned char	source_syn_state;	 //源同步状态、
	//PTI	unsigned char		Errval[4];//源秒以上的相对差值PTI
	unsigned char 	ppsErrval[4];//PPS的相对钟差值
	unsigned char 	TBD[2];
}__attribute__((packed)) B_state;

typedef struct
{
	CTime	time;				//大时间
	unsigned char	zoneinfo;	//时区
	unsigned char	timestate;	//源状态信息
	unsigned char	source_syn_state;//源同步状态、	
	unsigned char 	ppsErrval[4];	 //PPS的相对钟差值
	unsigned char  	Time_Synmode;    //对时模式
	unsigned char 	ethernet_mode;	 //网络模式 0代表ETH/ETE；1代表ETH-PTP;2代表UDP-ETE;3代表UDP-PTP；4、NTP；
	unsigned char	run_state;		 //运行状态
	//PTI	unsigned char	Errval[4];//时间信息相对于本地时间信息的误差值PTI
	unsigned char	BestsourceMac[6];//最佳主时钟MAC地址
	unsigned char 	locatemac[6];    //本地MAC地址	
	unsigned char	linkdelay[4];	 //链路延时
	unsigned char 	modified_filed[4]; //修正域
	unsigned char 	TBD[2];
}__attribute__((packed)) Ptpstate;
 typedef struct
{
	unsigned char Hzval[4];		//
	unsigned char warnMessage;  //告警信息
	unsigned char SSMmessage;	//SSM信息
	unsigned char TBD[2];
}__attribute__((packed)) SSMstate;
typedef struct 
{
	unsigned char	frequency[4];	  //电网工频值
	unsigned char	freqClock[3];	  //工频钟
	unsigned char	freqClockDiffe[3];//工频钟差值
}__attribute__((packed)) powerFreq;

typedef struct
{
	unsigned char  		Self_Checkstate;	//装置自检状态			
	unsigned char		Slot_state[15];		//待定 插槽1~15的板卡状态
	unsigned char		CryOscistate[8];	//晶振锁定,频率,偏差估算, 
	unsigned char		AtomTimestate[8]; 	//原子钟状态 原子钟锁定,频率,频率估算
	CTime 				Locatetime;			//本地时间信息
	unsigned char		LoZoneinfo;			//时区:B7(0：+，1：-),B6,B5B4B3B2B1B0(时区值)
	unsigned char		LoTimestate;		//B7闰秒预告、B6闰秒标志、B5夏制时预告、B4夏制时标志、B3B2B1B0时间质量位
	unsigned char  		LoSourceTime;		//时间源选择结果0：BD源；1：gps；2：主时钟地面有线(CABLE)3：主时钟热备时源(STBY);4：自守时5：从时钟B1源：6：从时钟B2源；7：PTP1源；8：PTP2源。
	unsigned char  		LoTimeAbnormal;		//时间跳变侦测状态异常
	unsigned char		TBD[2];
	unsigned char 		LoppsErrval[4];
	sourcestate 		BDS;
	sourcestate 		GPS;
	B_state  			IRIG_B1;
	B_state  			IRIG_B2;
	Ptpstate 			PTP1;
	Ptpstate 			PTP2;
	SSMstate 			E1_7;
	SSMstate 			E1_8;
	powerFreq 			Channel1;
	powerFreq 			Channel2;
}__attribute__((packed)) FPGA2PPC_MSG;




#endif