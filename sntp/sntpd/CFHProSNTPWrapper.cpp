//****************************************************************************//
//* Copyright (c) 2007, 府河电气有限公司                                      
//* All rights reserved.                                                     
//* 文件 CFHProSNTPWrapper.cpp                                               
//* 日期 2015-9-22        
//* 作者 rwp                 
//* 注释 封装SNTP协议类                                 
//****************************************************************************//
#include "CFHProSNTPWrapper.h"
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <linux/types.h>

#define  LOG_LEVEL 3


CPSNTPWrapper::CPSNTPWrapper()
  :m_pFunc_StartSNTP(NULL),
  m_pFunc_EndSNTP(NULL),
  m_pFunc_LeapSec(NULL)
{
  
}

CPSNTPWrapper::~CPSNTPWrapper()
{

}

bool CPSNTPWrapper::InitLibrary()
{
	m_bInit = false;
	m_hHandle = lib_load_library(LIBNAME_TIMESNTP);
	if (m_hHandle == NULL){                                                                                                                                                                                                                                                                                                                 
		WriteLog(1,"SNTP动态库链接失败");
		return false;
	}
	
	m_pFunc_StartSNTP = (FUNC_StartSNTP)lib_get_symbol(m_hHandle,"StartSNTP");
	if (m_pFunc_StartSNTP == NULL){
		WriteLog(1,"SNTP动态库符号链接出错");
		return false;
	}

	m_pFunc_EndSNTP = (FUNC_EndSNTP)lib_get_symbol(m_hHandle,"EndSNTP");
	if (m_pFunc_EndSNTP == NULL){
		WriteLog(1,"SNTP动态库符号链接出错");
		return false;
	}

	m_pFunc_LeapSec = (FUNC_LeapSec)lib_get_symbol(m_hHandle,"LeapSec");
	if (m_pFunc_LeapSec == NULL){
		WriteLog(1,"SNTP动态库符号链接出错");
		return false;
	}
	m_bInit = true;
	return true;
}

t_handle CPSNTPWrapper::lib_load_library(const char *filename)
{
	t_handle handle;
	if (filename == NULL){
		return NULL;
	}
#ifdef OS_WINDOWS
	handle = LoadLibrary(filename); 
#else
	handle = dlopen(filename, RTLD_LAZY);
	if (!handle){
		printf("lib load error(%s)\n",dlerror());
		return NULL;
	}
#endif
	return handle;
}

void *CPSNTPWrapper::lib_get_symbol(t_handle handle, const char *symbol)
{
	void *ptr=NULL; 
	if (handle==NULL || symbol==NULL){ 
		return NULL; 
	}
#ifdef OS_WINDOWS    
	ptr=(void *)GetProcAddress((HINSTANCE)handle, symbol); 
#else
	ptr = (void *)dlsym(handle, symbol);
	if (ptr == NULL)
		return NULL;
#endif
	return ptr;
}


int CPSNTPWrapper::lib_free_library(t_handle handle)
{
	if (handle == NULL){
		return -1;
	}
#ifdef OS_WINDOWS	
	FreeLibrary((HINSTANCE)handle);
#else
	if (dlclose(handle) != 0)
		return -1;
	else
		return 0;
#endif
	return 0;
}

void CPSNTPWrapper::WriteLog(int iLevel,const char *format, ...)
{
	if (iLevel>LOG_LEVEL){
		return;
	}	
#define LOGLENGTH 1024
	char    buffer[LOGLENGTH];
  	va_list ap;
	va_start(ap,format);
	vsnprintf(buffer,LOGLENGTH,format,ap);//vsprintf的长度限制版
	va_end(ap);
#if(LOG_LEVEL >= 3)	

	struct timeval  tv;
    struct timezone tz;
	gettimeofday(&tv,&tz);
	struct tm *tm = localtime(&tv.tv_sec);
	char time_prt[50]="";
	sprintf(time_prt,"%.4d-%.2d-%.2d %.2d:%.2d:%.2d.%03ld ",
		tm->tm_year + 1900, tm->tm_mon + 1,tm->tm_mday, tm->tm_hour,
		tm->tm_min, tm->tm_sec,tv.tv_usec/1000);
#endif

	printf("[CPSNTPWrapper] %s %s\n",time_prt,buffer);
			
}

bool CPSNTPWrapper::StartSNTP( SNTP_CONFIG& pConfig )
{
  if (m_pFunc_StartSNTP)
  {
     return m_pFunc_StartSNTP(pConfig);
  }
  return false;
}

bool CPSNTPWrapper::EndSNTP()
{
  if (m_pFunc_EndSNTP)
  {
     return m_pFunc_EndSNTP();
  }
  return false;
}

int CPSNTPWrapper::LeapSec(char flag)
{
  if (m_pFunc_LeapSec)
  {
    return m_pFunc_LeapSec(flag);
  }
  return 0;
}
