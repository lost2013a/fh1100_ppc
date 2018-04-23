//****************************************************************************//
//* Copyright (c) 2007, 府河电气有限公司                                      
//* All rights reserved.                                                     
//* 文件 sntp.c                                               
//* 日期 2015-8-31        
//* 作者 rwp                 
//* 注释 sntp服务端                                 
//****************************************************************************//

#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef OS_LINUX
#include <sys/time.h>
#else
#include <winsock2.h>
#include <time.h>
#endif
#include "sntpMsgAttach.h"

/*
 * Time of day conversion constant.  Ntp's time scale starts in 1900,
 * Unix in 1970.
 */


#define BEIJINGTIME  28800  /* 8hours */

void CSNTPMsgAttach::pack_ul(unsigned char *buf, unsigned long val)
{
  buf[0] = (val >> 24) & 0xff;
  buf[1] = (val >> 16) & 0xff;
  buf[2] = (val >> 8) & 0xff;
  buf[3] = val & 0xff;
}

unsigned long CSNTPMsgAttach::unpack_ul(const unsigned char *buf)
{
  return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

void CSNTPMsgAttach::pack_ts(unsigned char *buf, double ts)
{
  double i, f;

  f = modf(ts, &i);
  pack_ul(buf, (unsigned long) i);
  pack_ul(buf + 4, (unsigned long) (f * 4294967296.0));//32位是4294967296即0到2^32次方-1
}

double CSNTPMsgAttach::unpack_ts(const unsigned char *buf)
{
  return unpack_ul(buf) + unpack_ul(buf + 4) / 4294967296.0;
}

void CSNTPMsgAttach::sntp_pack(unsigned char *buf, const struct sntp *sntp)
{
  buf[0] = (sntp->li << 6) | (sntp->vn << 3) | sntp->mode;
  buf[1] = sntp->stratum;
  buf[2] = sntp->poll;
  buf[3] = (signed char) sntp->precision;
  pack_ul(&buf[4], sntp->root_delay * 65536.0);
  pack_ul(&buf[8], sntp->root_dispersion * 65536.0);
  memcpy(&buf[12], &sntp->identifier, 4);
  pack_ts(&buf[16], sntp->reference);
  pack_ts(&buf[24], sntp->originate);
  pack_ts(&buf[32], sntp->receive);
  pack_ts(&buf[40], sntp->transmit);
}

void CSNTPMsgAttach::sntp_unpack(struct sntp *sntp, const unsigned char *buf)
{
  sntp->li = buf[0] >> 6;
  sntp->vn = (buf[0] >> 3) & 0x07;
  sntp->mode = buf[0] & 0x07;
  sntp->stratum = buf[1];
  sntp->poll = buf[2];
  sntp->precision = buf[3];
  sntp->root_delay = unpack_ul(&buf[4]) / 65536.0;
  sntp->root_dispersion = unpack_ul(&buf[8]) / 65536.0;
  memcpy(&sntp->identifier, &buf[12], 4);
  sntp->reference = unpack_ts(&buf[16]);
  sntp->originate = unpack_ts(&buf[24]);
  sntp->receive = unpack_ts(&buf[32]);
  sntp->transmit = unpack_ts(&buf[40]);
}

void CSNTPMsgAttach::sntp_tstotv(double ts, struct timeval *tv)
{
  double i, f;

  /* don't care the leap seconds */
  f = modf(ts - DIFF19001970, &i);
  tv->tv_sec = i + BEIJINGTIME;
//  tv->tv_sec = i;
  tv->tv_usec = f * 1e6;
}

double CSNTPMsgAttach::sntp_tvtots( struct timeval *tv,int delta/*=0*/)
{
 
#define MOD_MICROSECOND 1000000
  //注意off=0.5*(T2-T1-(T4-T3))，delta值修正的是T3(服务端中的t4)和off值是0.5倍关系
  //先借一秒
  long long tmp_usec = tv->tv_usec+MOD_MICROSECOND+delta;
  //进位判断，理论上为1/0，减去先借位为实际进位
  int sec_carry = (tmp_usec / MOD_MICROSECOND)-1;
  //printf("%d.%d\n",tv->tv_sec,tv->tv_usec);
  tv->tv_usec = tmp_usec %MOD_MICROSECOND;
  tv->tv_sec += sec_carry;
  //printf("%d.%d\n",tv->tv_sec,tv->tv_usec);
  return DIFF19001970 + tv->tv_sec + tv->tv_usec * 1e-6;
}

double CSNTPMsgAttach::sntp_now(int delta)
{
	struct tm *local_time = NULL;
  	struct timeval now; 
#ifdef OS_LINUX
	gettimeofday(&now, NULL);
#else
	DWORD tickcount;
	SYSTEMTIME systime;
	time_t timi;
	TIME_ZONE_INFORMATION tzi;
	GetLocalTime(&systime);
	time(&timi);
	GetTimeZoneInformation(&tzi); 
	if (_timezone == 0)
	{
	timi = timi + tzi.Bias*60;
	}
	else
	{
	  if (tzi.Bias == 0)
	  {
		  timi = timi - tzi.Bias*60;
	  }
	}
	local_time = localtime(&timi);
	now.tv_sec = timi;
	now.tv_usec = (systime.wMilliseconds)*1000;
#endif
	/*返回当前时间并加上延迟补偿的NTP时间戳*/
	//固定补偿30us（组包时间+硬件时间）
	return sntp_tvtots(&now,delta+30*2);

}

const char * CSNTPMsgAttach::sntp_inspect(const struct sntp *sntp)
{
  static char buf[1024];

  sprintf(buf, 
	   "li: %d\n"
	   "vn: %d\n"
	   "mode:%d\n"
	   "stratum: %d\n"
	   "poll: %d\n"
	   "precision: %d\n"
	   "delay: %f\n"
	   "dispersion: %f\n"
	   "identifier: %02x %02x %02x %02x\n"
	   "reference: %f\n"
	   "originate: %f\n"
	   "receive: %f\n"
	   "transmit: %f\n",
	   sntp->li, sntp->vn, sntp->mode,
	   sntp->stratum, sntp->poll, sntp->precision,
	   sntp->root_delay, sntp->root_dispersion,
	   sntp->identifier[0], sntp->identifier[1],
	   sntp->identifier[2], sntp->identifier[3],
	   sntp->reference, sntp->originate, sntp->receive, sntp->transmit);

  return buf;
}

CSNTPMsgAttach::CSNTPMsgAttach()
{

}

CSNTPMsgAttach::~CSNTPMsgAttach()
{

}


