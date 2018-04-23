//****************************************************************************//
//* Copyright (c) 2007, ���ӵ������޹�˾                                      
//* All rights reserved.                                                     
//* �ļ� sntp.h                                               
//* ���� 2015-8-31        
//* ���� rwp                 
//* ע�� sntp�����                                 
//****************************************************************************//
#ifndef _SNTP_H
#define _SNTP_H

#include "stdlib.h"
	#include <string.h>
#include <vector>

using namespace std;

#ifdef OS_LINUX
#include <sys/time.h>
#endif

#define DIFF19001970 2208988800.0	/* 2208988800 1970 - 1900 in seconds */


struct sntp {
  int li;     							//1.	LI �����ʶ��
  int vn;     							//2.	VN �汾�ţ�����һ����bits����������ʾNTP�İ汾�ţ�����Ϊ3
  int mode;   							//3.	Mode ģʽ������һ����bits������
  int stratum;							//4.	stratum���㣩������һ��8bits���������޷��ţ�����ʾ����ʱ�ӵĲ��ˮƽ
  int poll;   							//5��Poll ���Լ������λsigned integer����ʾ������Ϣ֮������������ȷ�����ƽ ������
  signed char precision;//6��Precision ���ȣ���λsigned integer����ʾ����ʱ�Ӿ��ȣ���ȷ�����ƽ������ֵ��         -6����ƽ����-20��΢�ʱ�ӣ���
  double root_delay;						  	//7.	Root Delay��ʱ�ӣ�32λ�����Ŷ���С������ʾ�����ο�Դ֮���������ܹ�ʱ�ӣ���С��λ��15~16bits��
  double root_dispersion;				//8.	Root Dispersion����ɢ��32λ�����Ŷ���С������ʾ�����ο�Դ�йص����������С��λ��15~16bits����Χ��0~���ٺ��롣
  unsigned char identifier[4];//9.	Reference Identifier�ο�ʱ�ӱ�ʶ����32bits��������ʶ����Ĳο�Դ����stratum 0��δָ������stratum 1�������ο���������£����ֶ����ĸ���λ�ֽڣ�����룬������string��ʾ��
  double reference;					//10.	�ο�ʱ�����ϵͳʱ�����һ�α��趨����µ�ʱ��
  double originate;					//11.	ԭʼʱ������ͻ��˷��͵�ʱ��
  double receive;					//12.	����ʱ���������˽��ܵ���ʱ��
  double transmit;				 	//13.	����ʱ�����������ͳ�Ӧ��ʱ��ʱ��
};

#define SNTP_HEADER_SIZE    48
#define SNTP_MOD_NULL  		0	//����               
#define SNTP_MOD_ACT  		1   //�Գ��Լ���         
#define SNTP_MOD_PAS        2   //�����ĶԳ���       
#define SNTP_MOD_CLIENT     3   //�ͻ���             
#define SNTP_MOD_SERVER     4   //������             
#define SNTP_MOD_BDC        5   //�㲥               
#define SNTP_MOD_RESERVE    6   //ΪNTP������ϵ����  
#define SNTP_MOD_NOUSE      7   //Ϊ���ñ���    

class CSNTPMsgAttach
{
public:
  CSNTPMsgAttach ();
  virtual ~CSNTPMsgAttach();
  public:
  void sntp_pack(unsigned char *, const struct sntp *);
  void sntp_unpack(struct sntp *, const unsigned char *);
  void sntp_tstotv(double, struct timeval *);
  double sntp_tvtots( struct timeval *,int delta=0);
  double sntp_now(int delta=0);
  const char *sntp_inspect(const struct sntp *);
private:
  void pack_ul(unsigned char *buf, unsigned long val);
  unsigned long unpack_ul(const unsigned char *buf);
  void pack_ts(unsigned char *buf, double ts);
  double unpack_ts(const unsigned char *buf);
};


#endif
