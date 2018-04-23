//****************************************************************************//
//* Copyright (c) 2007, 府河电气有限公司                                      
//* All rights reserved.                                                     
//* 文件 FHCom103Inface.h                                              
//* 日期 2015-7-25        
//* 作者 rwp                 
//* 注释 府河串口103规约接口                               
//****************************************************************************//

#include "FHSNTPInface.h"
#include "SNTPMainFlow.h"

CsntpMainFlow g_sntpMainFlow;

bool StartSNTP(SNTP_CONFIG& pConfig)
{
  return (g_sntpMainFlow.Start(pConfig)==0);
}

bool EndSNTP()
{
  g_sntpMainFlow.End();
  return true;
}

int LeapSec(char li)
{
  return g_sntpMainFlow.LeapSec(li);
}
