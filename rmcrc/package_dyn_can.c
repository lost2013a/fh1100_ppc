#include "gongyong.h"
#include "serial.h"
#include "config.h"
#include <time.h>
#include "bsp.h"

int	bds_offset_nsCount,gps_offset_nsCount,b1_offset_nsCount,b2_offset_nsCount;

void Package_dynframe(S_CanFrame *sendframe, ToIn_Loc_Data *data)
{
		int i = 0;
		sendframe[0].can_id = 0x01100130;
		sendframe[0].can_dlc = 8;
		sendframe[0].data[0] = PTI.Self_Checkstate;
		//故障触发条件	
		//告警触发条件
		sendframe[0].data[1] = data->FaultTrigcond;
		sendframe[0].data[2] = data->WarnTrigcond;
		sendframe[0].data[3] = PTI.LoTimestate;
		sendframe[0].data[4] = PTI.LoSourceTime;
		sendframe[0].data[5] = PTI.LoTimeAbnormal;	
		sendframe[0].data[6] = param.event_count;								
		sendframe[0].data[7] = file_param.Sysset.Master_Slaver; 
		
		sendframe[1].can_id = 0x01100131;
		sendframe[1].can_dlc = 8;
		sendframe[1].data[0] = PTI.LoZoneinfo;
		sendframe[1].data[1] = PTI.Locatetime.year[0];
		sendframe[1].data[2] = PTI.Locatetime.year[1];
		sendframe[1].data[3] = PTI.Locatetime.month;
		sendframe[1].data[4] = PTI.Locatetime.day;
		sendframe[1].data[5] = PTI.Locatetime.hour;
		sendframe[1].data[6] = PTI.Locatetime.min;
		sendframe[1].data[7] = PTI.Locatetime.second;
	
		sendframe[2].can_id = 0x01100132;
		sendframe[2].can_dlc = 8;
		for(i = 0 ; i <  8 ; i++)
			sendframe[2].data[i] = PTI.CryOscistate[i];
	
		sendframe[3].can_id = 0x01100133;
		sendframe[3].can_dlc =	8;
		for(i = 0 ; i <  8 ; i++)
			sendframe[3].data[i] = PTI.AtomTimestate[i];
	
		sendframe[4].can_id = 0x01100540;
		sendframe[4].can_dlc = 8;
		sendframe[4].data[0] = PTI.BDS.zoneinfo;		//时区
		sendframe[4].data[1] = PTI.BDS.timestate;		//b7:闰秒预告
		sendframe[4].data[2] = PTI.BDS.satellites;		//星数
		sendframe[4].data[3] = PTI.BDS.source_ant_state;//源模块、天线状态
		sendframe[4].data[4] = PTI.BDS.source_syn_state;//源同步状态、；
		sendframe[5].can_id = 0x01100541;
		sendframe[5].can_dlc  = 8;
		for(i = 0 ; i < 6; i++)
			sendframe[5].data[i] = PTI.BDS.latitude[i];
	
		sendframe[6].can_id = 0x01100542;
		sendframe[6].can_dlc = 8;
		for(i = 0 ; i < 6; i++)
			sendframe[6].data[i] = PTI.BDS.longitude[i];
	
		sendframe[7].can_id = 0x01100543;
		sendframe[7].can_dlc  = 8;
		for(i = 0 ; i < 6; i++)
			sendframe[7].data[i] = PTI.BDS.high[i];
	
		sendframe[8].can_id = 0x01100544;
		sendframe[8].can_dlc = 8;
		sendframe[8].data[0] = (clock_offset.BDS.sec&0xff000000)>>24;
		sendframe[8].data[1] = (clock_offset.BDS.sec&0x00ff0000)>>16;
		sendframe[8].data[2] = (clock_offset.BDS.sec&0x0000ff00)>>8;
		sendframe[8].data[3] = clock_offset.BDS.sec&0xff;
		sendframe[8].data[4] = (clock_offset.BDS.nsec&0x7f000000)>>24;
		sendframe[8].data[5] = (clock_offset.BDS.nsec&0x00ff0000)>>16;
		sendframe[8].data[6] = (clock_offset.BDS.nsec&0x0000ff00)>>8;
		sendframe[8].data[7] = clock_offset.BDS.nsec&0xff;
		
		sendframe[9].can_id = 0x01100a40; 
		sendframe[9].can_dlc = 8;
		sendframe[9].data[0] = PTI.GPS.zoneinfo;
		sendframe[9].data[1] = PTI.GPS.timestate;
		sendframe[9].data[2] = PTI.GPS.satellites;
		sendframe[9].data[3] = PTI.GPS.source_ant_state;//源模块、天线状态
		sendframe[9].data[4] = PTI.GPS.source_syn_state;//源同步状态
		sendframe[10].can_id = 0x01100a41;
		sendframe[10].can_dlc  = 8;
		for(i = 0 ; i < 6; i++)
			sendframe[10].data[i] = PTI.GPS.latitude[i];
	
		sendframe[11].can_id = 0x01100a42;
		sendframe[11].can_dlc  = 8;
		for(i = 0 ; i < 6; i++)
			sendframe[11].data[i] = PTI.GPS.longitude[i];
	
		sendframe[12].can_id = 0x01100a43;
		sendframe[12].can_dlc  = 8;
		for(i = 0 ; i < 6; i++)
			sendframe[12].data[i] = PTI.GPS.high[i];
	
		sendframe[13].can_id = 0x01100a44;
		sendframe[13].can_dlc  = 8;
		
		sendframe[13].data[0] = (clock_offset.GPS.sec&0xff000000)>>24;
		sendframe[13].data[1] = (clock_offset.GPS.sec&0x00ff0000)>>16;
		sendframe[13].data[2] = (clock_offset.GPS.sec&0x0000ff00)>>8;
		sendframe[13].data[3] = clock_offset.GPS.sec&0xff;
		sendframe[13].data[4] = (clock_offset.GPS.nsec&0x7f000000)>>24;
		sendframe[13].data[5] = (clock_offset.GPS.nsec&0x00ff0000)>>16;
		sendframe[13].data[6] = (clock_offset.GPS.nsec&0x0000ff00)>>8;
		sendframe[13].data[7] = clock_offset.GPS.nsec&0xff;
	
		sendframe[14].can_id = 0x01101420;
		sendframe[14].can_dlc = 8;
		sendframe[14].data[0] = PTI.IRIG_B1.zoneinfo;
		sendframe[14].data[1] = PTI.IRIG_B1.timestate;
		sendframe[14].data[2] = PTI.IRIG_B1.source_syn_state;
		sendframe[14].data[3] = PTI.IRIG_B2.zoneinfo;
		sendframe[14].data[4] = PTI.IRIG_B2.timestate;
		sendframe[14].data[5] = PTI.IRIG_B2.source_syn_state;
//printf("B1状态：%x, %x, %x, B2状态：%x, %x, %x\n",
//				sendframe[14].data[0],sendframe[14].data[1],sendframe[14].data[2],
//				sendframe[14].data[3],sendframe[14].data[4],sendframe[14].data[5]);
	
		sendframe[15].can_id = 0x01101421;
		sendframe[15].can_dlc  = 8;
		
		sendframe[15].data[0] = (clock_offset.B1.sec&0xff000000)>>24;
		sendframe[15].data[1] = (clock_offset.B1.sec&0x00ff0000)>>16;
		sendframe[15].data[2] = (clock_offset.B1.sec&0x0000ff00)>>8;
		sendframe[15].data[3] = clock_offset.B1.sec&0xff;
		sendframe[15].data[4] = (clock_offset.B1.nsec&0x7f000000)>>24;
		sendframe[15].data[5] = (clock_offset.B1.nsec&0x00ff0000)>>16;
		sendframe[15].data[6] = (clock_offset.B1.nsec&0x0000ff00)>>8;
		sendframe[15].data[7] = clock_offset.B1.nsec&0xff;
		
		sendframe[16].can_id = 0x01101422;
		sendframe[16].can_dlc = 8;
		
		sendframe[16].data[0] = (clock_offset.B2.sec&0xff000000)>>24;
		sendframe[16].data[1] = (clock_offset.B2.sec&0x00ff0000)>>16;
		sendframe[16].data[2] = (clock_offset.B2.sec&0x0000ff00)>>8;
		sendframe[16].data[3] = clock_offset.B2.sec&0xff;
		sendframe[16].data[4] = (clock_offset.B2.nsec&0x7f000000)>>24;
		sendframe[16].data[5] = (clock_offset.B2.nsec&0x00ff0000)>>16;
		sendframe[16].data[6] = (clock_offset.B2.nsec&0x0000ff00)>>8;
		sendframe[16].data[7] = clock_offset.B2.nsec&0xff;

		sendframe[17].can_id = 0x01101770;
		sendframe[17].can_dlc = 8;
		sendframe[17].data[0] = PTI.PTP1.zoneinfo;
		sendframe[17].data[1] = PTI.PTP1.timestate;
		sendframe[17].data[2] = PTI.PTP1.source_syn_state;
		sendframe[17].data[3] = PTI.PTP1.Time_Synmode;	//对时模式
		sendframe[17].data[4] = PTI.PTP1.ethernet_mode;
		sendframe[17].data[5] = PTI.PTP1.run_state;
	
		sendframe[18].can_id = 0x01101771;
		sendframe[18].can_dlc = 8;
		sendframe[18].data[0] = (clock_offset.PTP1.sec&0xff000000)>>24;
		sendframe[18].data[1] = (clock_offset.PTP1.sec&0x00ff0000)>>16;
		sendframe[18].data[2] = (clock_offset.PTP1.sec&0x0000ff00)>>8;
		sendframe[18].data[3] = clock_offset.PTP1.sec&0xff;
		sendframe[18].data[4] = (clock_offset.PTP1.nsec&0x7f000000)>>24;
		sendframe[18].data[5] = (clock_offset.PTP1.nsec&0x00ff0000)>>16;
		sendframe[18].data[6] = (clock_offset.PTP1.nsec&0x0000ff00)>>8;
		sendframe[18].data[7] = clock_offset.PTP1.nsec&0xff;
		
	
		sendframe[19].can_id = 0x01101772;
		sendframe[19].can_dlc = 8;
		for(i = 0 ; i < 6; i++)
			sendframe[19].data[i] = PTI.PTP1.BestsourceMac[i];
	
		sendframe[20].can_id = 0x01101773;
		sendframe[20].can_dlc = 8;
		for(i = 0; i < 4; i++)
			sendframe[20].data[i] = PTI.PTP1.linkdelay[i];//链路延时
		for(i = 4; i < 8; i++)
			sendframe[20].data[i] = PTI.PTP1.modified_filed[i-4]; //修正域
		
		sendframe[21].can_id = 0x01101774;
		sendframe[21].can_dlc = 8;
		sendframe[21].data[0] = PTI.PTP2.zoneinfo;
		sendframe[21].data[1] = PTI.PTP2.timestate;
		sendframe[21].data[2] = PTI.PTP2.source_syn_state;
		sendframe[21].data[3] = PTI.PTP2.Time_Synmode;	//对时模式
		sendframe[21].data[4] = PTI.PTP2.ethernet_mode;
		sendframe[21].data[5] = PTI.PTP2.run_state;
	
		sendframe[22].can_id = 0x01101775;
		sendframe[22].can_dlc = 8;
		sendframe[22].data[0] = (clock_offset.PTP2.sec&0xff000000)>>24;
		sendframe[22].data[1] = (clock_offset.PTP2.sec&0x00ff0000)>>16;
		sendframe[22].data[2] = (clock_offset.PTP2.sec&0x0000ff00)>>8;
		sendframe[22].data[3] = clock_offset.PTP2.sec&0xff;
		sendframe[22].data[4] = (clock_offset.PTP2.nsec&0x7f000000)>>24;
		sendframe[22].data[5] = (clock_offset.PTP2.nsec&0x00ff0000)>>16;
		sendframe[22].data[6] = (clock_offset.PTP2.nsec&0x0000ff00)>>8;
		sendframe[22].data[7] = clock_offset.PTP2.nsec&0xff;
	
		sendframe[23].can_id = 0x01101776;
		sendframe[23].can_dlc = 8;
		for(i = 0 ; i < 6; i++)
			sendframe[23].data[i] = PTI.PTP2.BestsourceMac[i];
	
		sendframe[24].can_id = 0x01101777;
		sendframe[24].can_dlc = 8;
		for(i = 0; i < 4; i++)
			sendframe[24].data[i] = PTI.PTP2.linkdelay[i];//链路延时
		for(i = 4; i < 8; i++)
			sendframe[24].data[i] = PTI.PTP2.modified_filed[i-4]; //修正域

		sendframe[25].can_id = 0x01101f30;
		sendframe[25].can_dlc = 8;
		sendframe[25].data[0] = myclog.freque1%256;
		sendframe[25].data[1] = (myclog.freque1/256)%256;
		sendframe[25].data[2] = (myclog.freque1/65536)%256;
		sendframe[25].data[3] = 0;
		sendframe[25].data[4] = 0;
		sendframe[25].data[5] = 0;
		sendframe[25].data[6] = 0;
		sendframe[25].data[7] = 0;
		//for(i = 0; i < 4; i++)
		//	sendframe[25].data[i] = PTI.Channel1.frequency[i];

		sendframe[26].can_id = 0x01101f31;
		sendframe[26].can_dlc = 8;
		for(i = 0; i < 3; i++)
			sendframe[26].data[i] = PTI.Channel1.freqClock[i];
		for(i = 3; i < 6; i++)
			sendframe[26].data[i] = PTI.Channel1.freqClock[i-3];

		sendframe[27].can_id = 0x01101f32;
		sendframe[27].can_dlc = 8;
		sendframe[27].data[0] = myclog.freque2%256;
		sendframe[27].data[1] = (myclog.freque2/256)%256;
		sendframe[27].data[2] = (myclog.freque2/65536)%256;
		sendframe[27].data[3] = 0;
		sendframe[27].data[4] = 0;
		sendframe[27].data[5] = 0;
		sendframe[27].data[6] = 0;
		sendframe[27].data[7] = 0;
		//for(i = 0; i < 4; i++)
		//	sendframe[27].data[i] = PTI.Channel2.frequency[i];
//printf("电网频率=%d, %d\n",
//				sendframe[27].data[0]+sendframe[27].data[1]*256+sendframe[27].data[2]*65536,	
//				sendframe[27].data[0]+sendframe[27].data[1]*256+sendframe[27].data[2]*65536);

		sendframe[28].can_id = 0x01101f33;
		sendframe[28].can_dlc = 8;
		for(i = 0; i < 3; i++)
			sendframe[28].data[i] = PTI.Channel2.freqClock[i];
		for(i = 3; i < 6; i++)
			sendframe[28].data[i] = PTI.Channel2.freqClock[i-3];
		
		sendframe[29].can_id = 0x01102310;
		sendframe[29].can_dlc = 8;
		for(i = 0; i < 4; i++)
			sendframe[29].data[i] = PTI.E1_7.Hzval[i];
		sendframe[29].data[4] = PTI.E1_7.warnMessage;
		sendframe[29].data[5] = PTI.E1_7.SSMmessage;

		sendframe[30].can_id = 0x01102311;
		sendframe[30].can_dlc = 8;
		for(i = 0; i < 4; i++)
			sendframe[30].data[i] = PTI.E1_8.Hzval[i];
		sendframe[30].data[4] = PTI.E1_8.warnMessage;
		sendframe[30].data[5] = PTI.E1_8.SSMmessage;
		

}
/*串口，can都要用*/

//Mclklog myclog;保存所有状态日志类型的当前的状态值,,可能会在compare_log时修改；
//





void Package_dynsendframe_data(ToIn_Loc_Data* data)
{	
	LOG_MESSAGE log_buf;//写调试日志buf
	__u8    CRandom_Source_off=0;//任何一路时间源不可用	
//	struct tm ptr1, ptr2, ptr3, ptr4, ptr5;
//	CTime 	locatetime, bdstime, gpstime ,b1time, b2time;
//	int  bds_off = 0, gps_off = 0, b1_off = 0, b2_off = 0;
//	int  loc_year = 0, bds_year = 0, gps_year = 0, b1_year = 0, b2_year = 0;
//	
	if( (myclog.BDS_source_state==1 && param.event_param[0].flag)//file_param.control.global_control1&0x01)
		||(myclog.GPS_source_state==1 && param.event_param[1].flag)//file_param.control.global_control1&0x02) 
		||(myclog.B1_source_state== 1 && param.event_param[2].flag)//file_param.control.global_control1&0x04)
		||(myclog.B2_source_state== 1 && param.event_param[3].flag)//file_param.control.global_control1&0x08) 
		||(myclog.PTP1_source_state== 1 && param.event_param[4].flag)//file_param.control.global_control1&0x10) 
		||(myclog.PTP2_source_state== 1 && param.event_param[5].flag))//file_param.control.global_control1&0x20))
		{
			CRandom_Source_off = 1;
		}
	data->FaultTrigcond = 0;	
	if(param.event_param[8].flag)
		data->FaultTrigcond |= myclog.BDS_recv_state<< 7;				//北斗模块异常
	if(param.event_param[9].flag)
		data->FaultTrigcond |= myclog.GPS_recv_state<< 6;				//GPS模块异常
	if(param.event_param[6].flag &&
		(myclog.BDS_ant_state==3||myclog.BDS_ant_state==4))
		data->FaultTrigcond |= 0x20;									//北斗天线异常
	if(param.event_param[7].flag &&
		(myclog.GPS_ant_state==3 || myclog.GPS_ant_state==4))
		data->FaultTrigcond |= 0x10;									//GPS天线异常
	if(file_param.control.fault_control&0x01)
		data->FaultTrigcond |= myclog.core_err << 3;					//CPU核心板异常
	if(file_param.control.fault_control&0x01 && param.event_param[17].now_state && (param.tame&0x1))	//晶振：b1:1 驯服(频率源同步)异常 b0:0初始化 b0:1初始化完成
		data->FaultTrigcond |=  0x04;												//晶振驯服异常超过60秒
	else 
		data->FaultTrigcond &=  0xFB;	
	//myprintf("FaultTrigcond=%d\n",data->FaultTrigcond);
	if(file_param.control.fault_control&0x04 && param.event_param[28].flag)	
		data->FaultTrigcond |=  myclog.source_frist_err_off_30m << 1;
	if(file_param.control.fault_control&0x08 && param.event_param[29].flag)	
		data->FaultTrigcond |= myclog.source_err_off_24h;
		
	data->WarnTrigcond = 0;
	if(file_param.control.warning_control&0x80)
		data->WarnTrigcond  |= myclog.Power_state << 7;			//任一电源消失
	if(param.event_param[0].flag)	
		data->WarnTrigcond  |= myclog.BDS_source_state  << 6;	//北斗失锁（信号异常）
	if(param.event_param[1].flag)	
		data->WarnTrigcond  |= myclog.GPS_source_state << 5;	//GPS失锁（信号异常）	
	if(file_param.control.warning_control&0x01)	
		data->WarnTrigcond  |= myclog.IRIG_Q  << 4;				//B码质量低于本机
	if(file_param.control.warning_control&0x02)	
		data->WarnTrigcond  |= (myclog.Lo_TimeAbnormal&0x01) << 3;	//时间连续性异常(修改记录2017.11.28)
	if(file_param.control.warning_control&0x04)		
		data->WarnTrigcond  |= CRandom_Source_off << 2;			//任一时钟源不可用
	if(file_param.control.warning_control&0x08)		
		data->WarnTrigcond  |= myclog.source_frist_err_30m<< 1;	//--所有独立时源不可用小于30分钟（首次同步）
	if(file_param.control.warning_control&0x10)	
	data->WarnTrigcond  |= myclog.source_err_24h; 				//所有独立时源不可用小于24小时（曾经同步过）
	if(data->WarnTrigcond!=param.warn)
	{
		//sprintf(log_buf.name,"告警控制字发生变化：%02x-->%02x",param.warn,data->WarnTrigcond);
		//add_my_log(log_buf);
		param.warn=data->WarnTrigcond;
	}
	if(data->FaultTrigcond!=param.fault)
	{
		sprintf(log_buf.name,"故障控制字发生变化：%02x-->%02x",param.fault,data->FaultTrigcond);
		add_my_log(log_buf);
		param.fault=data->FaultTrigcond;
	}
	return;

}



	
