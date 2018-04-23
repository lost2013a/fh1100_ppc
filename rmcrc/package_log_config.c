#include "serial.h"
#include "bsp.h"

#define Tranverse32(X)   ((((__u32)(X) & 0xff000000) >> 24) | (((__u32)(X) & 0x00ff0000) >> 8) | (((__u32)(X) & 0x0000ff00) << 8) | (((__u32)(X) & 0x000000ff) << 24))

#include <arpa/inet.h>
#include "log.h"

LOG_MESSAGE log_message;

//以下是配置的 ----------------------远程帧数据
	
void Package_master_slaver_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
	cfsendframe->can_id = 0x02100100 | CAN_EFF_FLAG;
	cfsendframe->can_dlc = 8;
	cfsendframe->data[0] = read_UTP.Sysset.Master_Slaver;
	cfsendframe->data[1] = read_UTP.Sysset.Source_Mode;
}
void Package_uartset_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
	cfsendframe->can_id = 0x02100200| CAN_EFF_FLAG;
	cfsendframe->can_dlc = 8;
	cfsendframe->data[0] = read_UTP.Sysset.uart1.Uart_baud;
	cfsendframe->data[1] = read_UTP.Sysset.uart1.Uart_protocol;
	cfsendframe->data[2] = read_UTP.Sysset.uart2.Uart_baud; 
	cfsendframe->data[3] = read_UTP.Sysset.uart2.Uart_protocol;
	cfsendframe->data[4] = read_UTP.Sysset.IRIG_B_verify;
}
void Package_Priority_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
	int i;
	cfsendframe->can_id = 0x02100300 | CAN_EFF_FLAG;
	cfsendframe->can_dlc = 8;
	for(i = 0; i < 3; i++)
	{
		cfsendframe->data[i] = read_UTP.Sysset.Priority[i];

	}
}
void Package_delayoffset_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
	int i;
	cfsendframe[0].can_id = 0x02100440| CAN_EFF_FLAG;
	cfsendframe[0].can_dlc = 8;
	for(i = 0;  i < 4; i++)
		cfsendframe[0].data[i] = read_UTP.Sysset.Mod_value.BDSoffsetVal[i];
	for(i = 4; i < 8; i++)
	cfsendframe[0].data[i] = read_UTP.Sysset.Mod_value.GPSoffsetVal[i-4];

	cfsendframe[1].can_id = 0x02100441| CAN_EFF_FLAG;
	cfsendframe[1].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[1].data[i] = read_UTP.Sysset.Mod_value.B1offsetVal[i];
	for(i = 4; i < 8; i++)
	cfsendframe[1].data[i] = read_UTP.Sysset.Mod_value.B2offsetVal[i-4];

	cfsendframe[2].can_id = 0x02100442| CAN_EFF_FLAG;
	cfsendframe[2].can_dlc = 8;
	for(i = 0; i < 4; i++)
		cfsendframe[2].data[i] = read_UTP.Sysset.Mod_value.PTP1offsetVal[i];
	for(i = 4; i < 8; i++)
		cfsendframe[2].data[i] = read_UTP.Sysset.Mod_value.PTP2offsetVal[i-4];

	cfsendframe[3].can_id = 0x02100443| CAN_EFF_FLAG;
	cfsendframe[3].can_dlc = 8;
	for(i = 0; i < 4; i++)
		cfsendframe[3].data[i] = read_UTP.Sysset.Mod_value.ClockoffsetVal[i];
	for(i = 4; i < 8; i++)
		cfsendframe[3].data[i] = read_UTP.Sysset.Mod_value.PulseoffsetVal[i-4];

	cfsendframe[4].can_id = 0x02100444| CAN_EFF_FLAG;
	cfsendframe[4].can_dlc = 8;
	for(i = 0; i < 4; i++)
		cfsendframe[4].data[i] = read_UTP.Sysset.Mod_value.SmesoffsetVal[i];
	for(i = 4; i < 8; i++)
		cfsendframe[4].data[i] = read_UTP.Sysset.Mod_value.RIGBoffsetVal[i-4];
}

void Package_Net1set_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
 	int i = 0;
	cfsendframe[0].can_id = 0x02100940| CAN_EFF_FLAG;
	cfsendframe[0].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[0].data[i] = read_UTP.Net_set1.ip[i];
	for(i = 4; i < 8; i++)
	cfsendframe[0].data[i] = read_UTP.Net_set1.mask[i-4];

	cfsendframe[1].can_id = 0x02100941| CAN_EFF_FLAG;
	cfsendframe[1].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[1].data[i] = read_UTP.Net_set1.wg[i];
	for(i = 4; i < 8; i++)
	cfsendframe[1].data[i] = read_UTP.Net_set1.remoteip[i-4];

	cfsendframe[2].can_id = 0x02100942| CAN_EFF_FLAG;
	cfsendframe[2].can_dlc = 8;
	for(i = 0; i < 2; i++)
	cfsendframe[2].data[i] = read_UTP.Net_set1.localport[i];
	for(i = 2; i < 4; i++)
	cfsendframe[2].data[i] = read_UTP.Net_set1.remoteport[i-2];
	cfsendframe[2].data[4] = read_UTP.Net_set1.ethernet_mode;
	cfsendframe[2].data[5] = read_UTP.Net_set1.spreadmode;
	cfsendframe[2].data[6] = read_UTP.Net_set1.spreadgaps;

	cfsendframe[3].can_id = 0x02100943| CAN_EFF_FLAG;
	cfsendframe[3].can_dlc = 8;
	for(i = 0; i < 6; i++)
	cfsendframe[3].data[i] = read_UTP.Net_set1.mac[i];
	cfsendframe[3].data[6] = read_UTP.Net_set1.master_slaverflag;
	cfsendframe[3].data[7] = read_UTP.Net_set1.ptpd.DOMAINNUMBER;

	cfsendframe[4].can_id = 0x02100944| CAN_EFF_FLAG;
	cfsendframe[4].can_dlc = 8;
	cfsendframe[4].data[0] = read_UTP.Net_set1.ptpd.ANNOUNCE_INTERVAL;
	cfsendframe[4].data[1] = read_UTP.Net_set1.ptpd.ANNOUNCE_RECEIPT_TIMEOUT;
	cfsendframe[4].data[2] = read_UTP.Net_set1.ptpd.SYNC_INTERVAL;
	cfsendframe[4].data[3] = read_UTP.Net_set1.ptpd.PDELAYREQ_INTERVAL;
	cfsendframe[4].data[4] = read_UTP.Net_set1.ptpd.DELAYREQ_INTERVAL;
	cfsendframe[4].data[5] = read_UTP.Net_set1.ptpd.SLAVE_PRIORITY1;
	cfsendframe[4].data[6] = read_UTP.Net_set1.ptpd.SLAVE_PRIORITY2;
	cfsendframe[4].data[7] = read_UTP.Net_set1.ptpd.reserve;
}

void Package_Net2set_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
 	int i = 0;
	cfsendframe[0].can_id = 0x02110940| CAN_EFF_FLAG;
	cfsendframe[0].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[0].data[i] = read_UTP.Net_set2.ip[i];
	for(i = 4; i < 8; i++)
	cfsendframe[0].data[i] = read_UTP.Net_set2.mask[i-4];

	cfsendframe[1].can_id = 0x02110941| CAN_EFF_FLAG;
	cfsendframe[1].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[1].data[i] = read_UTP.Net_set2.wg[i];
	for(i = 4; i < 8; i++)
	cfsendframe[1].data[i] = read_UTP.Net_set2.remoteip[i-4];

	cfsendframe[2].can_id = 0x02110942| CAN_EFF_FLAG;
	cfsendframe[2].can_dlc = 8;
	for(i = 0; i < 2; i++)
	cfsendframe[2].data[i] = read_UTP.Net_set2.localport[i];
	for(i = 2; i < 4; i++)
	cfsendframe[2].data[i] = read_UTP.Net_set2.remoteport[i-2];
	cfsendframe[2].data[4] = read_UTP.Net_set2.ethernet_mode;
	cfsendframe[2].data[5] = read_UTP.Net_set2.spreadmode;
	cfsendframe[2].data[6] = read_UTP.Net_set2.spreadgaps;

	cfsendframe[3].can_id = 0x02110943| CAN_EFF_FLAG;
	cfsendframe[3].can_dlc = 8;
	for(i = 0; i < 6; i++)
	cfsendframe[3].data[i] = read_UTP.Net_set2.mac[i];
	cfsendframe[3].data[6] = read_UTP.Net_set2.master_slaverflag;
	cfsendframe[3].data[7] = read_UTP.Net_set2.ptpd.DOMAINNUMBER;

	cfsendframe[4].can_id = 0x02110944| CAN_EFF_FLAG;
	cfsendframe[4].can_dlc = 8;
	cfsendframe[4].data[0] = read_UTP.Net_set2.ptpd.ANNOUNCE_INTERVAL;
	cfsendframe[4].data[1] = read_UTP.Net_set2.ptpd.ANNOUNCE_RECEIPT_TIMEOUT;
	cfsendframe[4].data[2] = read_UTP.Net_set2.ptpd.SYNC_INTERVAL;
	cfsendframe[4].data[3] = read_UTP.Net_set2.ptpd.PDELAYREQ_INTERVAL;
	cfsendframe[4].data[4] = read_UTP.Net_set2.ptpd.DELAYREQ_INTERVAL;
	cfsendframe[4].data[5] = read_UTP.Net_set2.ptpd.SLAVE_PRIORITY1;
	cfsendframe[4].data[6] = read_UTP.Net_set2.ptpd.SLAVE_PRIORITY2;
	cfsendframe[4].data[7] = read_UTP.Net_set2.ptpd.reserve;
}
void Package_Net3set_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
 	int i = 0;
	cfsendframe[0].can_id = 0x02120940| CAN_EFF_FLAG;
	cfsendframe[0].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[0].data[i] = read_UTP.Net_set3.ip[i];
	for(i = 4; i < 8; i++)
	cfsendframe[0].data[i] = read_UTP.Net_set3.mask[i-4];

	cfsendframe[1].can_id = 0x02120941| CAN_EFF_FLAG;
	cfsendframe[1].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[1].data[i] = read_UTP.Net_set3.wg[i];
	for(i = 4; i < 8; i++)
	cfsendframe[1].data[i] = read_UTP.Net_set3.remoteip[i-4];

	cfsendframe[2].can_id = 0x02120942| CAN_EFF_FLAG;
	cfsendframe[2].can_dlc = 8;
	for(i = 0; i < 2; i++)
	cfsendframe[2].data[i] = read_UTP.Net_set3.localport[i];
	for(i = 2; i < 4; i++)
	cfsendframe[2].data[i] = read_UTP.Net_set3.remoteport[i-2];
	cfsendframe[2].data[4] = read_UTP.Net_set3.ethernet_mode;
	cfsendframe[2].data[5] = read_UTP.Net_set3.spreadmode;
	cfsendframe[2].data[6] = read_UTP.Net_set3.spreadgaps;

	cfsendframe[3].can_id = 0x02120943| CAN_EFF_FLAG;
	cfsendframe[3].can_dlc = 8;
	for(i = 0; i < 6; i++)
	cfsendframe[3].data[i] = read_UTP.Net_set3.mac[i];
	cfsendframe[3].data[6] = read_UTP.Net_set3.master_slaverflag;
	cfsendframe[3].data[7] = read_UTP.Net_set3.ptpd.DOMAINNUMBER;

	cfsendframe[4].can_id = 0x02120944| CAN_EFF_FLAG;
	cfsendframe[4].can_dlc = 8;
	cfsendframe[4].data[0] = read_UTP.Net_set3.ptpd.ANNOUNCE_INTERVAL;
	cfsendframe[4].data[1] = read_UTP.Net_set3.ptpd.ANNOUNCE_RECEIPT_TIMEOUT;
	cfsendframe[4].data[2] = read_UTP.Net_set3.ptpd.SYNC_INTERVAL;
	cfsendframe[4].data[3] = read_UTP.Net_set3.ptpd.PDELAYREQ_INTERVAL;
	cfsendframe[4].data[4] = read_UTP.Net_set3.ptpd.DELAYREQ_INTERVAL;
	cfsendframe[4].data[5] = read_UTP.Net_set3.ptpd.SLAVE_PRIORITY1;
	cfsendframe[4].data[6] = read_UTP.Net_set3.ptpd.SLAVE_PRIORITY2;
	cfsendframe[4].data[7] = read_UTP.Net_set3.ptpd.reserve;
}

void Package_Net4set_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
 	int i = 0;
	cfsendframe[0].can_id = 0x02130940| CAN_EFF_FLAG;
	cfsendframe[0].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[0].data[i] = read_UTP.Net_set4.ip[i];
	for(i = 4; i < 8; i++)
	cfsendframe[0].data[i] = read_UTP.Net_set4.mask[i-4];

	cfsendframe[1].can_id = 0x02130941| CAN_EFF_FLAG;
	cfsendframe[1].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[1].data[i] = read_UTP.Net_set4.wg[i];
	for(i = 4; i < 8; i++)
	cfsendframe[1].data[i] = read_UTP.Net_set4.remoteip[i-4];

	cfsendframe[2].can_id = 0x02130942| CAN_EFF_FLAG;
	cfsendframe[2].can_dlc = 8;
	for(i = 0; i < 2; i++)
	cfsendframe[2].data[i] = read_UTP.Net_set4.localport[i];
	for(i = 2; i < 4; i++)
	cfsendframe[2].data[i] = read_UTP.Net_set4.remoteport[i-2];
	cfsendframe[2].data[4] = read_UTP.Net_set4.ethernet_mode;
	cfsendframe[2].data[5] = read_UTP.Net_set4.spreadmode;
	cfsendframe[2].data[6] = read_UTP.Net_set4.spreadgaps;

	cfsendframe[3].can_id = 0x02130943| CAN_EFF_FLAG;
	cfsendframe[3].can_dlc = 8;
	for(i = 0; i < 6; i++)
	cfsendframe[3].data[i] = read_UTP.Net_set4.mac[i];
	cfsendframe[3].data[6] = read_UTP.Net_set4.master_slaverflag;
	cfsendframe[3].data[7] = read_UTP.Net_set4.ptpd.DOMAINNUMBER;

	cfsendframe[4].can_id = 0x02130944| CAN_EFF_FLAG;
	cfsendframe[4].can_dlc = 8;
	cfsendframe[4].data[0] = read_UTP.Net_set4.ptpd.ANNOUNCE_INTERVAL;
	cfsendframe[4].data[1] = read_UTP.Net_set4.ptpd.ANNOUNCE_RECEIPT_TIMEOUT;
	cfsendframe[4].data[2] = read_UTP.Net_set4.ptpd.SYNC_INTERVAL;
	cfsendframe[4].data[3] = read_UTP.Net_set4.ptpd.PDELAYREQ_INTERVAL;
	cfsendframe[4].data[4] = read_UTP.Net_set4.ptpd.DELAYREQ_INTERVAL;
	cfsendframe[4].data[5] = read_UTP.Net_set4.ptpd.SLAVE_PRIORITY1;
	cfsendframe[4].data[6] = read_UTP.Net_set4.ptpd.SLAVE_PRIORITY2;
	cfsendframe[4].data[7] = read_UTP.Net_set4.ptpd.reserve;
}
void Package_Net5set_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
 	int i = 0;
	cfsendframe[0].can_id = 0x02140940| CAN_EFF_FLAG;
	cfsendframe[0].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[0].data[i] = read_UTP.Net_set5.ip[i];
	for(i = 4; i < 8; i++)
	cfsendframe[0].data[i] = read_UTP.Net_set5.mask[i-4];

	cfsendframe[1].can_id = 0x02140941| CAN_EFF_FLAG;
	cfsendframe[1].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[1].data[i] = read_UTP.Net_set5.wg[i];
	for(i = 4; i < 8; i++)
	cfsendframe[1].data[i] = read_UTP.Net_set5.remoteip[i-4];

	cfsendframe[2].can_id = 0x02140942| CAN_EFF_FLAG;
	cfsendframe[2].can_dlc = 8;
	for(i = 0; i < 2; i++)
	cfsendframe[2].data[i] = read_UTP.Net_set5.localport[i];
	for(i = 2; i < 4; i++)
	cfsendframe[2].data[i] = read_UTP.Net_set5.remoteport[i-2];
	cfsendframe[2].data[4] = read_UTP.Net_set5.ethernet_mode;
	cfsendframe[2].data[5] = read_UTP.Net_set5.spreadmode;
	cfsendframe[2].data[6] = read_UTP.Net_set5.spreadgaps;

	cfsendframe[3].can_id = 0x02140943| CAN_EFF_FLAG;
	cfsendframe[3].can_dlc = 8;
	for(i = 0; i < 6; i++)
	cfsendframe[3].data[i] = read_UTP.Net_set5.mac[i];
	cfsendframe[3].data[6] = read_UTP.Net_set5.master_slaverflag;
	cfsendframe[3].data[7] = read_UTP.Net_set5.ptpd.DOMAINNUMBER;

	cfsendframe[4].can_id = 0x02140944| CAN_EFF_FLAG;
	cfsendframe[4].can_dlc = 8;
	cfsendframe[4].data[0] = read_UTP.Net_set5.ptpd.ANNOUNCE_INTERVAL;
	cfsendframe[4].data[1] = read_UTP.Net_set5.ptpd.ANNOUNCE_RECEIPT_TIMEOUT;
	cfsendframe[4].data[2] = read_UTP.Net_set5.ptpd.SYNC_INTERVAL;
	cfsendframe[4].data[3] = read_UTP.Net_set5.ptpd.PDELAYREQ_INTERVAL;
	cfsendframe[4].data[4] = read_UTP.Net_set5.ptpd.DELAYREQ_INTERVAL;
	cfsendframe[4].data[5] = read_UTP.Net_set5.ptpd.SLAVE_PRIORITY1;
	cfsendframe[4].data[6] = read_UTP.Net_set5.ptpd.SLAVE_PRIORITY2;
	cfsendframe[4].data[7] = read_UTP.Net_set5.ptpd.reserve;
}
void Package_Net6set_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
 	int i = 0;
	cfsendframe[0].can_id = 0x02150940| CAN_EFF_FLAG;
	cfsendframe[0].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[0].data[i] = read_UTP.Net_set6.ip[i];
	for(i = 4; i < 8; i++)
	cfsendframe[0].data[i] = read_UTP.Net_set6.mask[i-4];

	cfsendframe[1].can_id = 0x02150941| CAN_EFF_FLAG;
	cfsendframe[1].can_dlc = 8;
	for(i = 0; i < 4; i++)
	cfsendframe[1].data[i] = read_UTP.Net_set6.wg[i];
	for(i = 4; i < 8; i++)
	cfsendframe[1].data[i] = read_UTP.Net_set6.remoteip[i-4];

	cfsendframe[2].can_id = 0x02150942| CAN_EFF_FLAG;
	cfsendframe[2].can_dlc = 8;
	for(i = 0; i < 2; i++)
	cfsendframe[2].data[i] = read_UTP.Net_set6.localport[i];
	for(i = 2; i < 4; i++)
	cfsendframe[2].data[i] = read_UTP.Net_set6.remoteport[i-2];
	cfsendframe[2].data[4] = read_UTP.Net_set6.ethernet_mode;
	cfsendframe[2].data[5] = read_UTP.Net_set6.spreadmode;
	cfsendframe[2].data[6] = read_UTP.Net_set6.spreadgaps;

	cfsendframe[3].can_id = 0x02150943| CAN_EFF_FLAG;
	cfsendframe[3].can_dlc = 8;
	for(i = 0; i < 6; i++)
	cfsendframe[3].data[i] = read_UTP.Net_set6.mac[i];
	cfsendframe[3].data[6] = read_UTP.Net_set6.master_slaverflag;
	cfsendframe[3].data[7] = read_UTP.Net_set6.ptpd.DOMAINNUMBER;

	cfsendframe[4].can_id = 0x02150944| CAN_EFF_FLAG;
	cfsendframe[4].can_dlc = 8;
	cfsendframe[4].data[0] = read_UTP.Net_set6.ptpd.ANNOUNCE_INTERVAL;
	cfsendframe[4].data[1] = read_UTP.Net_set6.ptpd.ANNOUNCE_RECEIPT_TIMEOUT;
	cfsendframe[4].data[2] = read_UTP.Net_set6.ptpd.SYNC_INTERVAL;
	cfsendframe[4].data[3] = read_UTP.Net_set6.ptpd.PDELAYREQ_INTERVAL;
	cfsendframe[4].data[4] = read_UTP.Net_set6.ptpd.DELAYREQ_INTERVAL;
	cfsendframe[4].data[5] = read_UTP.Net_set6.ptpd.SLAVE_PRIORITY1;
	cfsendframe[4].data[6] = read_UTP.Net_set6.ptpd.SLAVE_PRIORITY2;
	cfsendframe[4].data[7] = read_UTP.Net_set6.ptpd.reserve;
}

void Package_Zoneinfo_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
	cfsendframe->can_id = 0x02100E00 | CAN_EFF_FLAG;
	cfsendframe->can_dlc = 8;
	cfsendframe->data[0] = read_UTP.Sysset.Zoneinfo;
	cfsendframe->data[1] = read_UTP.Sysset.Time_offset_05hour;
}
void Package_PPXtype_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
	cfsendframe->can_id = 0x02100F00 | CAN_EFF_FLAG;
	cfsendframe->can_dlc = 8;
	cfsendframe->data[0] = read_UTP.Sysset.PPXtype;
}
void Package_AM_proportion_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
	cfsendframe->can_id = 0x02101000 | CAN_EFF_FLAG;
	cfsendframe->can_dlc = 8;
	cfsendframe->data[0] = read_UTP.Sysset.AM_proportion;
}
void Package_settime_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
	cfsendframe->can_id = 0x02101100 | CAN_EFF_FLAG;
	cfsendframe->can_dlc = 8;
	cfsendframe->data[0] = read_UTP.Sysset.Source_outputflag;
	cfsendframe->data[1] = read_UTP.Sysset.setSystime.year[1];
	cfsendframe->data[2] = read_UTP.Sysset.setSystime.year[0];
	cfsendframe->data[3] = read_UTP.Sysset.setSystime.month;
	cfsendframe->data[4] = read_UTP.Sysset.setSystime.day;
	cfsendframe->data[5] = read_UTP.Sysset.setSystime.hour;
	cfsendframe->data[6] = read_UTP.Sysset.setSystime.min;
	cfsendframe->data[7] = read_UTP.Sysset.setSystime.second;
}
void Package_Slot_State_frame(S_CanFrame *cfsendframe)
{
	int i = 0;
	cfsendframe[0].can_id = 0x02101510 | CAN_EFF_FLAG;
	cfsendframe[0].can_dlc = 8;
	for(i = 0; i < 8; i++)
		cfsendframe[0].data[i] = FTP.Slot_state[i];
	
	cfsendframe[1].can_id = 0x02101511 | CAN_EFF_FLAG;
	cfsendframe[1].can_dlc = 8;
	for(i = 8; i < 15; i++)
		cfsendframe[1].data[i-8] = FTP.Slot_state[i];
}
void Package_BD3dSET_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{
	int i = 0;
	cfsendframe[0].can_id = 0x02101220 | CAN_EFF_FLAG;
	cfsendframe[0].can_dlc = 8;
	for(i = 0; i < 6; i++)
		cfsendframe[0].data[i] = read_UTP.Bd3Dset.Longitude[i];

	cfsendframe[1].can_id = 0x02101221 | CAN_EFF_FLAG;
	cfsendframe[1].can_dlc = 8;
	for(i = 0; i < 6; i++)
		cfsendframe[1].data[i] = read_UTP.Bd3Dset.Latitude[i];

	cfsendframe[2].can_id = 0x02101222 | CAN_EFF_FLAG;
	cfsendframe[2].can_dlc = 8;
	for(i = 0; i < 6; i++)
		cfsendframe[2].data[i] = read_UTP.Bd3Dset.High[i];
}

void Package_summerTime_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP)
{	
	cfsendframe[0].can_id = 0x02101710 | CAN_EFF_FLAG;
	cfsendframe[0].can_dlc = 8;
	cfsendframe[0].data[0] = read_UTP.summerTime.flag;
	//cfsendframe[0].data[1] = read_UTP.summerTime.start.year[1];
	//cfsendframe[0].data[2] = read_UTP.summerTime.start.year[0];
	int year = 2000+read_UTP.summerTime.start.year;
	cfsendframe[0].data[1] = year%256;
	cfsendframe[0].data[2] = year/256;
	cfsendframe[0].data[3] = read_UTP.summerTime.start.month;
	cfsendframe[0].data[4] = read_UTP.summerTime.start.day;
	cfsendframe[0].data[5] = read_UTP.summerTime.start.hour;
	cfsendframe[0].data[6] = read_UTP.summerTime.start.min;
	cfsendframe[0].data[7] = read_UTP.summerTime.start.second;
	
	cfsendframe[1].can_id = 0x02101711 | CAN_EFF_FLAG;
	cfsendframe[1].can_dlc = 8;
	cfsendframe[1].data[0] = 0;
	//cfsendframe[1].data[1] = read_UTP.summerTime.end.year[1];
	//cfsendframe[1].data[2] = read_UTP.summerTime.end.year[0];
	year = 2000+read_UTP.summerTime.end.year;
	cfsendframe[1].data[1] = year%256;
	cfsendframe[1].data[2] = year/256;
	cfsendframe[1].data[3] = read_UTP.summerTime.end.month;
	cfsendframe[1].data[4] = read_UTP.summerTime.end.day;
	cfsendframe[1].data[5] = read_UTP.summerTime.end.hour;
	cfsendframe[1].data[6] = read_UTP.summerTime.end.min;
	cfsendframe[1].data[7] = read_UTP.summerTime.end.second;
}



void Package_statelog_frame(S_CanFrame *cfsendframe, int order,int log_id,int fram_num)
{
	int i	= order;
	int id  = log_id;
	int buff[9];
	
	buff[0]=id+1;    
	buff[1]=mem_record[id].index;     
	buff[2]=mem_record[id].value;  
	buff[3]=mem_record[id].warn_time.year;    
	buff[4]=mem_record[id].warn_time.mon;
	buff[5]=mem_record[id].warn_time.day;
	buff[6]=mem_record[id].warn_time.hour;
	buff[7]=mem_record[id].warn_time.min;
	buff[8]=mem_record[id].warn_time.sec; 
	
	
	cfsendframe[2*i].can_id = (0x02101900 + i*2 )| CAN_EFF_FLAG | (fram_num<<4);
	cfsendframe[2*i].can_dlc = 8;
	cfsendframe[2*i].data[0] = (__u8)buff[0];
	cfsendframe[2*i].data[1] = (__u8)(buff[3]>> 8 & 0xff );
	cfsendframe[2*i].data[2] = (__u8)(buff[3]&0xff) ;
	cfsendframe[2*i].data[3] = (__u8)buff[4];
	cfsendframe[2*i].data[4] = (__u8)buff[5];
	cfsendframe[2*i].data[5] = (__u8)buff[6];
	cfsendframe[2*i].data[6] = (__u8)buff[7];
	cfsendframe[2*i].data[7] = (__u8)buff[8];
	cfsendframe[1+2*i].can_id = (0x02101900 + i*2+1) | CAN_EFF_FLAG | (fram_num<<4);;
	cfsendframe[1+2*i].can_dlc = 8;
	cfsendframe[1+2*i].data[0] = (__u8)buff[1];
	cfsendframe[1+2*i].data[1] = (__u8)buff[2];
}

void Package_Versionsmsg_frame(S_CanFrame *cfsendframe)
{
	LOG_MESSAGE log_buf;//写调试日志buf
	FILE *fp;
	unsigned char file_buf[1000]={0};
	int file_len,i;
	if((fp = fopen(DEV_VERSIONS_FILE,"r")) == NULL)
			{
				sprintf(log_buf.name,"版本信息文件打开失败");
				add_my_log(log_buf);
				return;
			}	
	fseek(fp,0L,SEEK_END);
	file_len=ftell(fp);
	if(file_len==0)
	{
		fclose(fp);
		sprintf(log_buf.name,"版本信息文件长度为0");
		add_my_log(log_buf);
		return;
	}
	fseek(fp,0L,SEEK_SET);	
	fread(file_buf, 1,file_len, fp);
	fclose(fp);
	for(i = 0; i < 16 ; i++)
	{
		cfsendframe[i].can_id = (0x02102af0 + i )| CAN_EFF_FLAG;
		cfsendframe[i].can_dlc = 8;
		cfsendframe[i].data[0] = (__u8)file_buf[8*i+0];
		cfsendframe[i].data[1] = (__u8)file_buf[8*i+1];
		cfsendframe[i].data[2] = (__u8)file_buf[8*i+2];
		cfsendframe[i].data[3] = (__u8)file_buf[8*i+3];
		cfsendframe[i].data[4] = (__u8)file_buf[8*i+4];
		cfsendframe[i].data[5] = (__u8)file_buf[8*i+5];
		cfsendframe[i].data[6] = (__u8)file_buf[8*i+6];
		cfsendframe[i].data[7] = (__u8)file_buf[8*i+7];
	}	
}


void Package_CtrlByte_frame(S_CanFrame *cfsendframe)
{
	cfsendframe[0].can_id = (0x02103a10)| CAN_EFF_FLAG;
	cfsendframe[0].can_dlc = 8;
	cfsendframe[0].data[0] = file_param.control.global_control1;
	cfsendframe[0].data[1] = file_param.control.global_control2;
	cfsendframe[0].data[2] = file_param.control.bds_control;
	cfsendframe[0].data[3] = file_param.control.gps_control;
	cfsendframe[0].data[4] = file_param.control.b1_control;
	cfsendframe[0].data[5] = file_param.control.b2_control;
	cfsendframe[0].data[6] = file_param.control.ptp1_control;
	cfsendframe[0].data[7] = file_param.control.ptp2_control;
	
	cfsendframe[1].can_id = (0x02103a11)| CAN_EFF_FLAG;
	cfsendframe[1].can_dlc = 8;
	cfsendframe[1].data[0] = file_param.control.bak_control1;
	cfsendframe[1].data[1] = file_param.control.bak_control2;
	cfsendframe[1].data[2] = file_param.control.fault_control;
	cfsendframe[1].data[3] = file_param.control.warning_control;
	cfsendframe[1].data[4] = file_param.control.bak_control3;
	cfsendframe[1].data[5] = file_param.control.bak_control4;
	cfsendframe[1].data[6] = file_param.control.bak_control5;
	cfsendframe[1].data[7] = file_param.control.bak_control6;	
}




