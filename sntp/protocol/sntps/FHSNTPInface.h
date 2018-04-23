//****************************************************************************//
//* Copyright (c) 2007, ���ӵ������޹�˾                                      
//* All rights reserved.                                                     
//* �ļ� FHSNTPInface.h                                               
//* ���� 2015-8-31        
//* ���� rwp                 
//* ע�� ����SNTPЭ��ӿ�                                 
//****************************************************************************//
#ifndef FHProSNTPAPI
#define FHProSNTPAPI

//#pragma  warning (disable:4786) 
//#pragma  warning (disable:4275)

#include "stdlib.h"
#include "string.h"
#include "stdio.h"



#ifdef WIN32
#define PRO_EXPORT extern "C"  __declspec( dllexport )
#else
#define PRO_EXPORT __attribute__ ((visibility ("default")))
#endif

#ifdef WIN32
#define LIBNAME_TIMESNTP "FHTIMESNTP.dll"
#else
#define LIBNAME_TIMESNTP "libFHTIMESNTP.so"
#endif

#define SNTP_PORT 123
#define SNTPROLE_UKWN_DEVICE 0  //δ֪
#define SNTPROLE_DFR_DEVICE  1  //¼����
#define SNTPROLE_GPS_DEVICE  2  //GPSװ��
#define SNTPROLE_TMS_DEVICE  3  //TSM�նˣ�ʱ�����ϵͳ��


typedef struct _SNTP_CONFIG{
  bool debug;     //���Ա�־
  int  offsetUS;  //+-����ֵ��us��
  int  sntpPort;  //SNTPЭ��˿�,һ��Ϊ123�����ⲿָ���˿ڿɴ���ָ���˿�
  int  role;      //Ӧ�ý�ɫ����ɫ��ͬʱ�в�ͬ�����߼�

  _SNTP_CONFIG()  {
	  memset(this,0,sizeof(_SNTP_CONFIG));
  }
}SNTP_CONFIG;

#ifdef __cplusplus
extern "C" {
#endif
 
  PRO_EXPORT  bool StartSNTP(SNTP_CONFIG& pConfig);
  PRO_EXPORT  bool EndSNTP();
  PRO_EXPORT  int  LeapSec(char li);

#ifdef __cplusplus
}
#endif


#endif // FHProSNTPAPI