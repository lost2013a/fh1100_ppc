#include "bsp.h"

void save_net_param(NetDev_Type* Net_set,unsigned char* file_buff)
{
	int i;
	for(i=0;i<4;i++)
	{
		file_buff[i+0] = Net_set->ip[i];
		file_buff[i+4] = Net_set->mask[i];
		file_buff[i+8] = Net_set->wg[i];
		file_buff[i+12]= Net_set->remoteip[i]; 
	}
	file_buff[16] = Net_set->localport[0];
	file_buff[17] = Net_set->localport[1];
	file_buff[18] = Net_set->remoteport[0];
	file_buff[19] = Net_set->remoteport[1];
	file_buff[20] = Net_set->ethernet_mode;
	file_buff[21] = Net_set->spreadmode;
	file_buff[22] = Net_set->spreadgaps;
	file_buff[23] = Net_set->mac[0];
	file_buff[24] = Net_set->mac[1];
	file_buff[25] = Net_set->mac[2];
	file_buff[26] = Net_set->mac[3];
	file_buff[27] = Net_set->mac[4];
	file_buff[28] = Net_set->mac[5];
	file_buff[29] = Net_set->master_slaverflag;
	file_buff[30] = Net_set->ptpd.DOMAINNUMBER;
	file_buff[31] = Net_set->ptpd.ANNOUNCE_INTERVAL;
	file_buff[32] = Net_set->ptpd.ANNOUNCE_RECEIPT_TIMEOUT;
	file_buff[33] = Net_set->ptpd.SYNC_INTERVAL;
	file_buff[34] = Net_set->ptpd.PDELAYREQ_INTERVAL;
	file_buff[35] = Net_set->ptpd.DELAYREQ_INTERVAL;
	file_buff[36] = Net_set->ptpd.SLAVE_PRIORITY1;
	file_buff[37] = Net_set->ptpd.SLAVE_PRIORITY2;
	file_buff[38] = Net_set->ptpd.reserve;
}

int	save_param(void)//保存file_param到文件
{
	LOG_MESSAGE log_buf;//写调试日志buf
	FILE *fp;
	
	int i;
	unsigned char file_buff[500]={0};
	char buf0[CONFIG_PATH] =  DEV_PARM_FILE;
	char buf1[CONFIG_PATH] =  DEV_PARM_BACK1_FILE;
	char buf2[CONFIG_PATH] =  DEV_PARM_BACK2_FILE;
	
	rename(buf1,buf2);
	rename(buf0,buf1);
	
	file_buff[0] = file_param.Sysset.Master_Slaver; 
	file_buff[1] = file_param.Sysset.Source_Mode;
	file_buff[2] = file_param.Sysset.uart1.Uart_baud;
	file_buff[3] = file_param.Sysset.uart1.Uart_protocol;
	file_buff[4] = file_param.Sysset.uart2.Uart_baud;
	file_buff[5] = file_param.Sysset.uart2.Uart_protocol;
	file_buff[6] = file_param.Sysset.IRIG_B_verify;
	file_buff[7] = file_param.Sysset.Priority[0];
	file_buff[8] = file_param.Sysset.Priority[1];
	file_buff[9] = file_param.Sysset.Priority[2];
	for(i=0;i<4;i++)
	{
		file_buff[10+i] = file_param.Sysset.Mod_value.BDSoffsetVal[i];
		file_buff[14+i] = file_param.Sysset.Mod_value.GPSoffsetVal[i];
		file_buff[18+i] = file_param.Sysset.Mod_value.B1offsetVal[i];
		file_buff[22+i] = file_param.Sysset.Mod_value.B2offsetVal[i];
		file_buff[26+i] = file_param.Sysset.Mod_value.PTP1offsetVal[i];
		file_buff[30+i] = file_param.Sysset.Mod_value.PTP2offsetVal[i];
		file_buff[34+i] = file_param.Sysset.Mod_value.ClockoffsetVal[i];
		file_buff[38+i] = file_param.Sysset.Mod_value.PulseoffsetVal[i];
		file_buff[42+i] = file_param.Sysset.Mod_value.SmesoffsetVal[i];
		file_buff[46+i] = file_param.Sysset.Mod_value.RIGBoffsetVal[i];
	}
	file_buff[50] = file_param.Sysset.PPXtype;
	file_buff[51] = file_param.Sysset.Zoneinfo;
	file_buff[52] = file_param.Sysset.Time_offset_05hour;
	file_buff[53] = file_param.Sysset.AM_proportion;
	file_buff[54] = file_param.Sysset.Source_outputflag;
	file_buff[55] = file_param.Sysset.setSystime.year[0];
	file_buff[56] = file_param.Sysset.setSystime.year[1];
	file_buff[57] = file_param.Sysset.setSystime.month;
	file_buff[58] = file_param.Sysset.setSystime.day;
	file_buff[59] = file_param.Sysset.setSystime.hour;
	file_buff[60] = file_param.Sysset.setSystime.min;
	file_buff[61] = file_param.Sysset.setSystime.second;
	for(i=0;i<6;i++)
	{
		file_buff[62+i] = file_param.Bd3Dset.Longitude[i];
		file_buff[68+i] = file_param.Bd3Dset.Latitude[i];
		file_buff[74+i] = file_param.Bd3Dset.High[i];
	}
	file_buff[80] = file_param.summerTime.flag;
	int year = 2000+file_param.summerTime.start.year;
	file_buff[81] = year%256;
	file_buff[82] = year/256;
	file_buff[83] = file_param.summerTime.start.month;
	file_buff[84] = file_param.summerTime.start.day;
	file_buff[85] = file_param.summerTime.start.hour;
	file_buff[86] = file_param.summerTime.start.min;
	file_buff[87] = file_param.summerTime.start.second;
	year = 2000+file_param.summerTime.end.year;
	file_buff[88] = year%256;
	file_buff[89] = year/256;
	file_buff[90] = file_param.summerTime.end.month;
	file_buff[91] = file_param.summerTime.end.day;
	file_buff[92] = file_param.summerTime.end.hour;
	file_buff[93] = file_param.summerTime.end.min;
	file_buff[94] = file_param.summerTime.end.second;
	
	file_buff[95]  = file_param.control.global_control1;
	file_buff[96]  = file_param.control.global_control2;
	file_buff[97]  = file_param.control.bds_control;
	file_buff[98]  = file_param.control.gps_control;
	file_buff[99]  = file_param.control.b1_control;
	file_buff[100] = file_param.control.b2_control;
	file_buff[101] = file_param.control.ptp1_control;
	file_buff[102] = file_param.control.ptp2_control;
	file_buff[103] = file_param.control.bak_control1;
	file_buff[104] = file_param.control.bak_control2;
	file_buff[105] = file_param.control.fault_control;
	file_buff[106] = file_param.control.warning_control;
	file_buff[107] = file_param.control.bak_control3;
	file_buff[108] = file_param.control.bak_control4;
	file_buff[109] = file_param.control.bak_control5;
	file_buff[110] = file_param.control.bak_control6;
	i = 120;
	save_net_param(&file_param.Net_set1,&file_buff[i]);
	save_net_param(&file_param.Net_set2,&file_buff[i+=40]);
	save_net_param(&file_param.Net_set3,&file_buff[i+=40]);
	save_net_param(&file_param.Net_set4,&file_buff[i+=40]);
	save_net_param(&file_param.Net_set5,&file_buff[i+=40]);
	save_net_param(&file_param.Net_set6,&file_buff[i+=40]);
	if((fp = fopen(buf0,"w+")) == NULL) //
	{
		perror(buf0);
		sprintf(log_buf.name,"参数保存失败");
		add_my_log(log_buf);
		return -1;
	}
	
	fwrite(file_buff, 1,400, fp);
	/*
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);	
	*/
	direct_write(fp,"write_param");
	return 0;
}






void read_net_param(NetDev_Type* Net_set,unsigned char* file_buff)
{
	int i;
	for(i=0;i<4;i++){
		Net_set->ip[i] 			= file_buff[0+i];
		Net_set->mask[i]		= file_buff[4+i];
		Net_set->wg[i]			= file_buff[8+i];
		Net_set->remoteip[i] 	= file_buff[12+i]; 
	}
	Net_set->localport[0]  = file_buff[16];
	Net_set->localport[0]  = file_buff[17];
	Net_set->remoteport[0] = file_buff[18];
	Net_set->remoteport[1] = file_buff[19];
	Net_set->ethernet_mode = file_buff[20];
	Net_set->spreadmode	   = file_buff[21];
	Net_set->spreadgaps    = file_buff[22];
	Net_set->mac[0] = file_buff[23];
	Net_set->mac[1] = file_buff[24];
	Net_set->mac[2] = file_buff[25];
	Net_set->mac[3] = file_buff[26];
	Net_set->mac[4] = file_buff[27];
	Net_set->mac[5] = file_buff[28];
	Net_set->master_slaverflag = file_buff[29];
	Net_set->ptpd.DOMAINNUMBER = file_buff[30];
	Net_set->ptpd.ANNOUNCE_INTERVAL 				= file_buff[31];
	Net_set->ptpd.ANNOUNCE_RECEIPT_TIMEOUT  = file_buff[32];
	Net_set->ptpd.SYNC_INTERVAL 			= file_buff[33];
	Net_set->ptpd.PDELAYREQ_INTERVAL 	= file_buff[34];
	Net_set->ptpd.DELAYREQ_INTERVAL  	= file_buff[35];
	Net_set->ptpd.SLAVE_PRIORITY1 		= file_buff[36];
	Net_set->ptpd.SLAVE_PRIORITY2		= file_buff[37];
	Net_set->ptpd.reserve 				= file_buff[38];
}




int read_param(void)	//从文件中读取配置信息，读取成功则修改装置配置，和给FPGA的配置
{
	FILE *fp;
	int i;
	unsigned char flag=0;
	unsigned char file_buff[500]={0};
	
	char buf0[CONFIG_PATH] =  DEV_PARM_FILE;
	char buf1[CONFIG_PATH] =  DEV_PARM_BACK1_FILE;
	char buf2[CONFIG_PATH] =  DEV_PARM_BACK2_FILE;
	
	if((fp = fopen(buf0,"rb")) != NULL)	{	//以二进制方式打开
		if(fread(file_buff, 1,400, fp)==400)
			flag=1;							//打开成功的标志	
		fclose(fp);
	}	
	if(flag==0){							//失败则尝试打开备份
		if((fp = fopen(buf1,"rb")) != NULL){
			if(fread(file_buff, 1,400, fp)==400)
				flag=1;
			fclose(fp);
		}	
	}	
	if(flag==0){
		if((fp = fopen(buf2,"rb")) != NULL){
			if(fread(file_buff, 1,400, fp)==400)
				flag=1;
			fclose(fp);
		}	
	}	
	if(0==flag){
		//myprintf("读取装置参数文件失败\n");
		return -1;
	}	
	else{	//读取成功
		//myprintf("读取装置参数文件成功\n");
		file_param.Sysset.Master_Slaver = file_buff[0]; 
		file_param.Sysset.Source_Mode   = file_buff[1];
		file_param.Sysset.uart1.Uart_baud= file_buff[2];
		file_param.Sysset.uart1.Uart_protocol = file_buff[3];
		file_param.Sysset.uart2.Uart_baud     = file_buff[4];
		file_param.Sysset.uart2.Uart_protocol = file_buff[5];
		file_param.Sysset.IRIG_B_verify = file_buff[6];
		file_param.Sysset.Priority[0]   = file_buff[7];
		file_param.Sysset.Priority[1]   = file_buff[8];
		file_param.Sysset.Priority[2]   = file_buff[9];
		for(i=0;i<4;i++)
		{
			file_param.Sysset.Mod_value.BDSoffsetVal[i]  = file_buff[10+i];
			file_param.Sysset.Mod_value.GPSoffsetVal[i]  = file_buff[14+i];
			file_param.Sysset.Mod_value.B1offsetVal[i]   = file_buff[18+i];
			file_param.Sysset.Mod_value.B2offsetVal[i]   = file_buff[22+i];
			file_param.Sysset.Mod_value.PTP1offsetVal[i] = file_buff[26+i];
			file_param.Sysset.Mod_value.PTP2offsetVal[i] = file_buff[30+i];
			file_param.Sysset.Mod_value.ClockoffsetVal[i]= file_buff[34+i];
			file_param.Sysset.Mod_value.PulseoffsetVal[i]= file_buff[38+i];
			file_param.Sysset.Mod_value.SmesoffsetVal[i] = file_buff[42+i];
			file_param.Sysset.Mod_value.RIGBoffsetVal[i] = file_buff[46+i];
		}	
		file_param.Sysset.PPXtype      			= file_buff[50];
		file_param.Sysset.Zoneinfo     			= file_buff[51];
		file_param.Sysset.Time_offset_05hour 	= file_buff[52];
		file_param.Sysset.AM_proportion 		= file_buff[53];
		file_param.Sysset.Source_outputflag 	= file_buff[54];
		file_param.Sysset.setSystime.year[0] 	= file_buff[55];
		file_param.Sysset.setSystime.year[1] 	= file_buff[56];
		file_param.Sysset.setSystime.month   	= file_buff[57];
		file_param.Sysset.setSystime.day     	= file_buff[58];
		file_param.Sysset.setSystime.hour 		= file_buff[59];
		file_param.Sysset.setSystime.min		= file_buff[60];
		file_param.Sysset.setSystime.second		= file_buff[61];
		for(i=0;i<6;i++)
		{
			file_param.Bd3Dset.Longitude[i] = file_buff[62+i];
			file_param.Bd3Dset.Latitude[i]  = file_buff[68+i];
			file_param.Bd3Dset.High[i]      = file_buff[74+i];
		}
		file_param.summerTime.flag = file_buff[80];
		int year = file_buff[81] + 256*file_buff[82] - 2000;
		file_param.summerTime.start.year  = year;
		file_param.summerTime.start.month = file_buff[83];
		file_param.summerTime.start.day   = file_buff[84];
		file_param.summerTime.start.hour  = file_buff[85];
		file_param.summerTime.start.min   = file_buff[86];
		file_param.summerTime.start.second= file_buff[87];
		year = file_buff[88] + 256*file_buff[89] - 2000;
		file_param.summerTime.end.year  = year;
		file_param.summerTime.end.month = file_buff[90];
		file_param.summerTime.end.day   = file_buff[91];
		file_param.summerTime.end.hour  = file_buff[92];
		file_param.summerTime.end.min   = file_buff[93];
		file_param.summerTime.end.second= file_buff[94];
		
		file_param.control.global_control1 = file_buff[95];
		file_param.control.global_control2 = file_buff[96];
		file_param.control.bds_control     = file_buff[97];
		file_param.control.gps_control     = file_buff[98];
		file_param.control.b1_control      = file_buff[99];
		file_param.control.b2_control      = file_buff[100];
		file_param.control.ptp1_control    = file_buff[101];
		file_param.control.ptp2_control    = file_buff[102];
		file_param.control.bak_control1    = file_buff[103];
		file_param.control.bak_control2    = file_buff[104];
		file_param.control.fault_control   = file_buff[105];
		file_param.control.warning_control = file_buff[106];
		file_param.control.bak_control3    = file_buff[107];
		file_param.control.bak_control4    = file_buff[108];
		file_param.control.bak_control5    = file_buff[109];
		file_param.control.bak_control6    = file_buff[110];
		
		i=120;
		read_net_param(&file_param.Net_set1,&file_buff[i]);
		read_net_param(&file_param.Net_set2,&file_buff[i+=40]);
		read_net_param(&file_param.Net_set3,&file_buff[i+=40]);
		read_net_param(&file_param.Net_set4,&file_buff[i+=40]);
		read_net_param(&file_param.Net_set5,&file_buff[i+=40]);
		read_net_param(&file_param.Net_set6,&file_buff[i+=40]);
		
		param.bad_priorty = (file_param.Sysset.Priority[0]>>4)&0x0f;
		param.gps_priorty = file_param.Sysset.Priority[0]&0x0f;
		param.b1_priorty  = (file_param.Sysset.Priority[1]>>4)&0x0f;
		param.b2_priorty  = file_param.Sysset.Priority[1]&0x0f;
		param.PTP1_priorty  = (file_param.Sysset.Priority[2]>>4)&0x0f;
		param.PTP2_priorty  = file_param.Sysset.Priority[2]&0x0f;
		
		memcpy(&ToFpga.Sysset,&file_param.Sysset,sizeof(Dev_S_SysMode));
		memcpy(&ToFpga.summerTime,&file_param.summerTime,sizeof(Dev_S_TimeSummer));
		memcpy(&ToFpga.Bd3Dset,&file_param.Bd3Dset,sizeof(Dev_S_BD3D));
		int time_offset;
		time_offset=((file_param.Sysset.Mod_value.BDSoffsetVal[0]&0x7f)<<24) +
								 (file_param.Sysset.Mod_value.BDSoffsetVal[1]<<16) +
								 (file_param.Sysset.Mod_value.BDSoffsetVal[2]<<8)  +
									file_param.Sysset.Mod_value.BDSoffsetVal[3];
		time_offset = time_offset/PPS_K;
		ToFpga.Sysset.Mod_value.BDSoffsetVal[3] = time_offset&0xff;
		ToFpga.Sysset.Mod_value.BDSoffsetVal[2] = (time_offset>>8)&0xff;
		ToFpga.Sysset.Mod_value.BDSoffsetVal[1] = (time_offset>>16)&0xff;
		ToFpga.Sysset.Mod_value.BDSoffsetVal[0] &= 0x80;
		ToFpga.Sysset.Mod_value.BDSoffsetVal[0] |= (time_offset>>24)&0x7f;		
		
		time_offset=((file_param.Sysset.Mod_value.GPSoffsetVal[0]&0x7f)<<24) +
								 (file_param.Sysset.Mod_value.GPSoffsetVal[1]<<16) +
								 (file_param.Sysset.Mod_value.GPSoffsetVal[2]<<8)  +
									file_param.Sysset.Mod_value.GPSoffsetVal[3];
		time_offset = time_offset/PPS_K;
		ToFpga.Sysset.Mod_value.GPSoffsetVal[3] = time_offset&0xff;
		ToFpga.Sysset.Mod_value.GPSoffsetVal[2] = (time_offset>>8)&0xff;
		ToFpga.Sysset.Mod_value.GPSoffsetVal[1] = (time_offset>>16)&0xff;
		ToFpga.Sysset.Mod_value.GPSoffsetVal[0] &= 0x80;
		ToFpga.Sysset.Mod_value.GPSoffsetVal[0] |= (time_offset>>24)&0x7f;		
		
		time_offset=((file_param.Sysset.Mod_value.B1offsetVal[0]&0x7f)<<24) +
								 (file_param.Sysset.Mod_value.B1offsetVal[1]<<16) +
								 (file_param.Sysset.Mod_value.B1offsetVal[2]<<8)  +
									file_param.Sysset.Mod_value.B1offsetVal[3];
		time_offset = time_offset/PPS_K;
		ToFpga.Sysset.Mod_value.B1offsetVal[3] = time_offset&0xff;
		ToFpga.Sysset.Mod_value.B1offsetVal[2] = (time_offset>>8)&0xff;
		ToFpga.Sysset.Mod_value.B1offsetVal[1] = (time_offset>>16)&0xff;
		ToFpga.Sysset.Mod_value.B1offsetVal[0] &= 0x80;
		ToFpga.Sysset.Mod_value.B1offsetVal[0] |= (time_offset>>24)&0x7f;		
		
		time_offset=((file_param.Sysset.Mod_value.B2offsetVal[0]&0x7f)<<24) +
								 (file_param.Sysset.Mod_value.B2offsetVal[1]<<16) +
								 (file_param.Sysset.Mod_value.B2offsetVal[2]<<8)  +
									file_param.Sysset.Mod_value.B2offsetVal[3];
		time_offset = time_offset/PPS_K;
		ToFpga.Sysset.Mod_value.B2offsetVal[3] = time_offset&0xff;
		ToFpga.Sysset.Mod_value.B2offsetVal[2] = (time_offset>>8)&0xff;
		ToFpga.Sysset.Mod_value.B2offsetVal[1] = (time_offset>>16)&0xff;
		ToFpga.Sysset.Mod_value.B2offsetVal[0] &= 0x80;
		ToFpga.Sysset.Mod_value.B2offsetVal[0] |= (time_offset>>24)&0x7f;		
		
		time_offset=((file_param.Sysset.Mod_value.PTP1offsetVal[0]&0x7f)<<24) +
								 (file_param.Sysset.Mod_value.PTP1offsetVal[1]<<16) +
								 (file_param.Sysset.Mod_value.PTP1offsetVal[2]<<8)  +
									file_param.Sysset.Mod_value.PTP1offsetVal[3];
		time_offset = time_offset/PPS_K;
		ToFpga.Sysset.Mod_value.PTP1offsetVal[3] = time_offset&0xff;
		ToFpga.Sysset.Mod_value.PTP1offsetVal[2] = (time_offset>>8)&0xff;
		ToFpga.Sysset.Mod_value.PTP1offsetVal[1] = (time_offset>>16)&0xff;
		ToFpga.Sysset.Mod_value.PTP1offsetVal[0] &= 0x80;
		ToFpga.Sysset.Mod_value.PTP1offsetVal[0] |= (time_offset>>24)&0x7f;		
		
		time_offset=((file_param.Sysset.Mod_value.PTP2offsetVal[0]&0x7f)<<24) +
								 (file_param.Sysset.Mod_value.PTP2offsetVal[1]<<16) +
								 (file_param.Sysset.Mod_value.PTP2offsetVal[2]<<8)  +
									file_param.Sysset.Mod_value.PTP2offsetVal[3];
		time_offset = time_offset/PPS_K;
		ToFpga.Sysset.Mod_value.PTP2offsetVal[3] = time_offset&0xff;
		ToFpga.Sysset.Mod_value.PTP2offsetVal[2] = (time_offset>>8)&0xff;
		ToFpga.Sysset.Mod_value.PTP2offsetVal[1] = (time_offset>>16)&0xff;
		ToFpga.Sysset.Mod_value.PTP2offsetVal[0] &= 0x80;
		ToFpga.Sysset.Mod_value.PTP2offsetVal[0] |= (time_offset>>24)&0x7f;		
		
		time_offset=((file_param.Sysset.Mod_value.ClockoffsetVal[0]&0x7f)<<24) +
								 (file_param.Sysset.Mod_value.ClockoffsetVal[1]<<16) +
								 (file_param.Sysset.Mod_value.ClockoffsetVal[2]<<8)  +
									file_param.Sysset.Mod_value.ClockoffsetVal[3];
		time_offset = time_offset/PPS_K;
		ToFpga.Sysset.Mod_value.ClockoffsetVal[3] = time_offset&0xff;
		ToFpga.Sysset.Mod_value.ClockoffsetVal[2] = (time_offset>>8)&0xff;
		ToFpga.Sysset.Mod_value.ClockoffsetVal[1] = (time_offset>>16)&0xff;
		ToFpga.Sysset.Mod_value.ClockoffsetVal[0] &= 0x80;
		ToFpga.Sysset.Mod_value.ClockoffsetVal[0] |= (time_offset>>24)&0x7f;		
		
		time_offset=((file_param.Sysset.Mod_value.PulseoffsetVal[0]&0x7f)<<24) +
								 (file_param.Sysset.Mod_value.PulseoffsetVal[1]<<16) +
								 (file_param.Sysset.Mod_value.PulseoffsetVal[2]<<8)  +
									file_param.Sysset.Mod_value.PulseoffsetVal[3];
		time_offset = time_offset/PPS_K;
		ToFpga.Sysset.Mod_value.PulseoffsetVal[3] = time_offset&0xff;
		ToFpga.Sysset.Mod_value.PulseoffsetVal[2] = (time_offset>>8)&0xff;
		ToFpga.Sysset.Mod_value.PulseoffsetVal[1] = (time_offset>>16)&0xff;
		ToFpga.Sysset.Mod_value.PulseoffsetVal[0] &= 0x80;
		ToFpga.Sysset.Mod_value.PulseoffsetVal[0] |= (time_offset>>24)&0x7f;	
		
		time_offset=((file_param.Sysset.Mod_value.SmesoffsetVal[0]&0x7f)<<24) +
								 (file_param.Sysset.Mod_value.SmesoffsetVal[1]<<16) +
								 (file_param.Sysset.Mod_value.SmesoffsetVal[2]<<8)  +
									file_param.Sysset.Mod_value.SmesoffsetVal[3];
		time_offset = time_offset/PPS_K;
		ToFpga.Sysset.Mod_value.SmesoffsetVal[3] = time_offset&0xff;
		ToFpga.Sysset.Mod_value.SmesoffsetVal[2] = (time_offset>>8)&0xff;
		ToFpga.Sysset.Mod_value.SmesoffsetVal[1] = (time_offset>>16)&0xff;
		ToFpga.Sysset.Mod_value.SmesoffsetVal[0] &= 0x80;
		ToFpga.Sysset.Mod_value.SmesoffsetVal[0] |= (time_offset>>24)&0x7f;	
		
		time_offset=((file_param.Sysset.Mod_value.RIGBoffsetVal[0]&0x7f)<<24) +
								 (file_param.Sysset.Mod_value.RIGBoffsetVal[1]<<16) +
								 (file_param.Sysset.Mod_value.RIGBoffsetVal[2]<<8)  +
									file_param.Sysset.Mod_value.RIGBoffsetVal[3];
		time_offset = time_offset/PPS_K;
		ToFpga.Sysset.Mod_value.RIGBoffsetVal[3] = time_offset&0xff;
		ToFpga.Sysset.Mod_value.RIGBoffsetVal[2] = (time_offset>>8)&0xff;
		ToFpga.Sysset.Mod_value.RIGBoffsetVal[1] = (time_offset>>16)&0xff;
		ToFpga.Sysset.Mod_value.RIGBoffsetVal[0] &= 0x80;
		ToFpga.Sysset.Mod_value.RIGBoffsetVal[0] |= (time_offset>>24)&0x7f;		

		param.man_flag 	  =  file_param.Sysset.Master_Slaver;
		param.bad_priorty = (file_param.Sysset.Priority[0]>>4)&0x0f;
		param.gps_priorty = file_param.Sysset.Priority[0]&0x0f;
		param.b1_priorty  = (file_param.Sysset.Priority[1]>>4)&0x0f;
		param.b2_priorty  = file_param.Sysset.Priority[1]&0x0f;
		param.PTP1_priorty  = (file_param.Sysset.Priority[2]>>4)&0x0f;
		param.PTP2_priorty  = file_param.Sysset.Priority[2]&0x0f;


		return 0;
	}	
}



