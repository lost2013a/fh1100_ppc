#ifndef __MYCAN_H
#define __MYCAN_H

#include "msg_fpga.h"
#include "can.h"

#define SEND_Retry  150
#define SEND_DYNFRAME_COUNT  31
#define OK 1
#define NO 0
#define MASTER_SLAVERSET 	0x01
#define UARTSET      		0x02
#define PRIORIRITYSET  		0x03
#define DELAYOFFSET			0x04
#define NETSET 				0x09
#define ZONEINFO			0x0E
#define PPXSET				0x0F
#define AMSET				0x10
#define	TIMESET				0x11
#define	BD3DSET				0x12  
#define SLOTSTATE			0x15
#define STATELOG            0x19
#define SUMMERTIME			0X17
#define USER_LOG			0x29
#define VER_MESS			0x2a
#define CTRL_BYTE			0x3a

typedef struct
{
	unsigned char FaultTrigcond;	//故障触发条件		
	unsigned char WarnTrigcond; 	//告警触发条件	
}ToIn_Loc_Data;


/*"package_dyn_can.c"*/
void Package_dynframe(S_CanFrame *sendframe, ToIn_Loc_Data *data);
void Package_dynsendframe_data(ToIn_Loc_Data* data);

/*"package_log_config.c"*/
void Package_master_slaver_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_uartset_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_Priority_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_delayoffset_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_Net1set_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_Net2set_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_Net3set_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_Net4set_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_Net5set_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_Net6set_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_Zoneinfo_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_PPXtype_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_AM_proportion_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_settime_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_Slot_State_frame(S_CanFrame *cfsendframe);
void Package_BD3dSET_frame(S_CanFrame *cfsendframe, LCD2PPC_MSG read_UTP);
void Package_summerTime_frame(S_CanFrame *cfsendframe,LCD2PPC_MSG read_UTP);
void Package_statelog_frame(S_CanFrame *cfsendframe, int order, int log_id,int fram_num);
void Package_Versionsmsg_frame(S_CanFrame *cfsendframe);
void Package_CtrlByte_frame(S_CanFrame *cfsendframe);


/*"mycan.c"*/
int SetIfAddr(char *ifname,unsigned char *ipaddr,unsigned char *netmask,unsigned char *gwip);

//int SetIfAddr(char *ifname, unsigned char *Ipaddr, unsigned char *mask,unsigned char *gateway);


#endif
