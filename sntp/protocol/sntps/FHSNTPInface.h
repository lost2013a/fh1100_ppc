//****************************************************************************//
//* Copyright (c) 2007, 府河电气有限公司                                      
//* All rights reserved.                                                     
//* 文件 FHSNTPInface.h                                               
//* 日期 2015-8-31        
//* 作者 rwp                 
//* 注释 府河SNTP协议接口                                 
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
#define SNTPROLE_UKWN_DEVICE 0  //未知
#define SNTPROLE_DFR_DEVICE  1  //录波器
#define SNTPROLE_GPS_DEVICE  2  //GPS装置
#define SNTPROLE_TMS_DEVICE  3  //TSM终端（时间管理系统）


typedef struct _SNTP_CONFIG{
  bool debug;     //调试标志
  int  offsetUS;  //+-补偿值（us）
  int  sntpPort;  //SNTP协议端口,一般为123，若外部指定端口可传入指定端口
  int  role;      //应用角色，角色不同时有不同处理逻辑

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