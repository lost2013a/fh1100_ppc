//****************************************************************************//
//* Copyright (c) 2007, 府河电气有限公司                                      
//* All rights reserved.                                                     
//* 文件 sntp.h                                               
//* 日期 2015-8-31        
//* 作者 rwp                 
//* 注释 sntp服务端                                 
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
  int li;     							//1.	LI 闰秒标识器
  int vn;     							//2.	VN 版本号：这是一个三bits的整数，表示NTP的版本号，现在为3
  int mode;   							//3.	Mode 模式：这是一个三bits的整数
  int stratum;							//4.	stratum（层）：这是一个8bits的整数（无符号），表示本地时钟的层次水平
  int poll;   							//5．Poll 测试间隔：八位signed integer，表示连续信息之间的最大间隔，精确到秒的平 方及。
  signed char precision;//6．Precision 精度：八位signed integer，表示本地时钟精度，精确到秒的平方级。值从         -6（主平）到-20（微妙级时钟）。
  double root_delay;						  	//7.	Root Delay根时延：32位带符号定点小数，表示在主参考源之间往返的总共时延，以小数位后15~16bits。
  double root_dispersion;				//8.	Root Dispersion根离散：32位带符号定点小数，表示在主参考源有关的名义错误，以小数位后15~16bits。范围：0~几百毫秒。
  unsigned char identifier[4];//9.	Reference Identifier参考时钟标识符：32bits，用来标识特殊的参考源。在stratum 0（未指定）或stratum 1（基本参考）的情况下，该字段以四个八位字节，左对齐，零填充的string表示。
  double reference;					//10.	参考时间戳：系统时钟最后一次被设定或更新的时间
  double originate;					//11.	原始时间戳：客户端发送的时间
  double receive;					//12.	接受时间戳：服务端接受到的时间
  double transmit;				 	//13.	传送时间戳：服务端送出应答时的时间
};

#define SNTP_HEADER_SIZE    48
#define SNTP_MOD_NULL  		0	//保留               
#define SNTP_MOD_ACT  		1   //对称性激活         
#define SNTP_MOD_PAS        2   //被动的对称性       
#define SNTP_MOD_CLIENT     3   //客户端             
#define SNTP_MOD_SERVER     4   //服务器             
#define SNTP_MOD_BDC        5   //广播               
#define SNTP_MOD_RESERVE    6   //为NTP控制性系保留  
#define SNTP_MOD_NOUSE      7   //为自用保留    

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
