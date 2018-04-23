//****************************************************************************//
//* Copyright (c) 2007, ���ӵ������޹�˾                                      
//* All rights reserved.                                                     
//* �ļ� FHCom103Inface.h                                              
//* ���� 2015-7-25        
//* ���� rwp                 
//* ע�� ���Ӵ���103��Լ�ӿ�                               
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
