//****************************************************************************//
//* Copyright (c) 2007, 府河电气有限公司                                      
//* All rights reserved.                                                     
//* 文件 platform.h                                               
//* 日期 2015-8-19        
//* 作者 rwp                 
//* 注释 平台相关                                 
//****************************************************************************//
#if !defined(_PLATFORM_H__)
#define _PLATFORM_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <stdio.h>  
#include <stdlib.h>  
#include <time.h>
#include <signal.h>
#include <memory>
#include <string.h>
#include <errno.h>

#ifdef WIN32
#define  sleep(x)                  Sleep(x*1000)  //采用与Linux下相同的形式  
#include <process.h>
#include <conio.h>
#include <Windows.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <pthread.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <dirent.h>

#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <dlfcn.h>  
#include <termios.h>    
#endif

#ifdef WIN32
//THREAD define
typedef  unsigned long             THREAD_ID;      //windows下线程ID为DWORD(unsiged long)
typedef  HANDLE                    THREAD_HANDLE;  //windows下线程创建后返回为HANDLE (void *)
typedef  unsigned int              THREAD_FUNC;    //windows下线程入口函数返回类型
#define INVALID_THREAD             NULL            //CreateThread()失败返回NULL
#define THREAD_RETURN              0               //线程返回值为DWORD

	typedef struct 
	{
		unsigned int count;
		unsigned int lastop; //0 none,1 signal,2 broadcast
		HANDLE		hEvent;
	}HCOND;

	//semaphore define
	typedef HANDLE					HSEM;
	typedef HANDLE					HNSEM;
	
#else
typedef pthread_t               THREAD_ID;      //linux下线程ID为pthread_t (unsigned int)
typedef pthread_t               THREAD_HANDLE;  //linux下线程创建后返回为int
typedef void *                  THREAD_FUNC;    //Linux下线程入口函数返回类型
#define WINAPI                                  //Linux下没有WINAPI(__stdcall) 
#define INVALID_THREAD          -1              //Linux下线程创建失败返回-1
#define THREAD_RETURN           NULL            //Linux下线程的返回值   	
typedef void                    *LPVOID;
//LPTSTR define
typedef char*					LPTSTR;
typedef unsigned int                    UINT;

	
		//semaphore define
	typedef int						HSEM;
	typedef sem_t					HNSEM;
#endif

/**
* @name		__THREAD_ROUTINE
* @brief	线程处理函数类型的定义
*/
#ifdef WIN32
	/**	
	* @brief	函数属性的常量定义。
	*/
	#define OUTPUTAPI __stdcall
typedef unsigned int(OUTPUTAPI *__THREAD_ROUTINE)(void *);
#else
typedef void *(*__THREAD_ROUTINE)(void *);
#endif

int rw_thread_create(THREAD_HANDLE *phandle, THREAD_ID *pid, __THREAD_ROUTINE routine, void *param);
int rw_thread_detach(THREAD_HANDLE handle);
int rw_thread_join(THREAD_HANDLE handle, void **pretcode);
void rw_thread_exit(unsigned int retval);
int rw_noname_sem_destroy(HNSEM *sem);
int rw_noname_sem_init(HNSEM *sem, unsigned int value);
int rw_noname_sem_post(HNSEM *sem);
int rw_noname_sem_wait(HNSEM *sem);
#endif // !defined(_PLATFORM_H__)
