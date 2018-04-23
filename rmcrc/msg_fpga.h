#ifndef __MSG_FPGA_H
#define __MSG_FPGA_H
#include <linux/types.h>
#include "gongyong.h"

#define	PPS_K 8	//FPGA��һ�����ڶ�Ӧ��ns
/*ʱ��Դ״̬��λ*/
#define	SS_SIGNAL  		0x01		//�ź�״̬ 0����
#define	SS_SYNC  		0x02		//ͬ��״̬
#define	SS_CONTINUITY  	0x04		//������



typedef struct
{
	unsigned char 	bds_flag;	//�ź�(ʧ��)��־
	unsigned char 	gps_flag; 
	unsigned char 	b1_flag;
	unsigned char 	b2_flag;
	unsigned char 	ptp1_flag;
	unsigned char 	ptp2_flag;
	unsigned int  	bds_sec;	//�ź�ʧ����ʱ���¼����λ��
	unsigned int  	gps_sec;
	unsigned int  	b1_sec;
	unsigned int  	b2_sec;
	unsigned int  	ptp1_sec;
	unsigned int  	ptp2_sec;
}__attribute__((packed)) LOSE_LOCK;


typedef struct 
{
	CTime			time;			 //��ʱ��
	unsigned char	zoneinfo;		 //ʱ�� 
	unsigned char	timestate;		 //Դ״̬��Ϣ,b7����Ԥ��,b6�����־
	unsigned char  	satellites;		 //������
	unsigned char	source_ant_state;//ģ��״̬0δ��ʼ��,1δ֪,2����,3��·,4��·
	unsigned char	source_syn_state;//ʱ��Դ״̬b2������b1ͬ��b0�ź�
	unsigned char 	latitude[6];     //γ��
	unsigned char 	longitude[6];    //����
	unsigned char 	high[6];         //�߶�
	unsigned char 	ppsErrval[4];	 //PPS������Ӳ�ֵ
	unsigned char 	TBD[2];
}__attribute__((packed)) sourcestate;

typedef struct 
{
	CTime			time;		 //��ʱ��
	unsigned char	zoneinfo;	 //ʱ��
	unsigned char	timestate;	 //BDԴ״̬��Ϣ
	unsigned char	source_syn_state;	 //Դͬ��״̬��
	//PTI	unsigned char		Errval[4];//Դ�����ϵ���Բ�ֵPTI
	unsigned char 	ppsErrval[4];//PPS������Ӳ�ֵ
	unsigned char 	TBD[2];
}__attribute__((packed)) B_state;

typedef struct
{
	CTime	time;				//��ʱ��
	unsigned char	zoneinfo;	//ʱ��
	unsigned char	timestate;	//Դ״̬��Ϣ
	unsigned char	source_syn_state;//Դͬ��״̬��	
	unsigned char 	ppsErrval[4];	 //PPS������Ӳ�ֵ
	unsigned char  	Time_Synmode;    //��ʱģʽ
	unsigned char 	ethernet_mode;	 //����ģʽ 0����ETH/ETE��1����ETH-PTP;2����UDP-ETE;3����UDP-PTP��4��NTP��
	unsigned char	run_state;		 //����״̬
	//PTI	unsigned char	Errval[4];//ʱ����Ϣ����ڱ���ʱ����Ϣ�����ֵPTI
	unsigned char	BestsourceMac[6];//�����ʱ��MAC��ַ
	unsigned char 	locatemac[6];    //����MAC��ַ	
	unsigned char	linkdelay[4];	 //��·��ʱ
	unsigned char 	modified_filed[4]; //������
	unsigned char 	TBD[2];
}__attribute__((packed)) Ptpstate;
 typedef struct
{
	unsigned char Hzval[4];		//
	unsigned char warnMessage;  //�澯��Ϣ
	unsigned char SSMmessage;	//SSM��Ϣ
	unsigned char TBD[2];
}__attribute__((packed)) SSMstate;
typedef struct 
{
	unsigned char	frequency[4];	  //������Ƶֵ
	unsigned char	freqClock[3];	  //��Ƶ��
	unsigned char	freqClockDiffe[3];//��Ƶ�Ӳ�ֵ
}__attribute__((packed)) powerFreq;

typedef struct
{
	unsigned char  		Self_Checkstate;	//װ���Լ�״̬			
	unsigned char		Slot_state[15];		//���� ���1~15�İ忨״̬
	unsigned char		CryOscistate[8];	//��������,Ƶ��,ƫ�����, 
	unsigned char		AtomTimestate[8]; 	//ԭ����״̬ ԭ��������,Ƶ��,Ƶ�ʹ���
	CTime 				Locatetime;			//����ʱ����Ϣ
	unsigned char		LoZoneinfo;			//ʱ��:B7(0��+��1��-),B6,B5B4B3B2B1B0(ʱ��ֵ)
	unsigned char		LoTimestate;		//B7����Ԥ�桢B6�����־��B5����ʱԤ�桢B4����ʱ��־��B3B2B1B0ʱ������λ
	unsigned char  		LoSourceTime;		//ʱ��Դѡ����0��BDԴ��1��gps��2����ʱ�ӵ�������(CABLE)3����ʱ���ȱ�ʱԴ(STBY);4������ʱ5����ʱ��B1Դ��6����ʱ��B2Դ��7��PTP1Դ��8��PTP2Դ��
	unsigned char  		LoTimeAbnormal;		//ʱ���������״̬�쳣
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