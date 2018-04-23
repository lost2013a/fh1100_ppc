//****************************************************************************//
//* Copyright (c) 2007, ���ӵ������޹�˾                                      
//* All rights reserved.                                                     
//* �ļ� platform.h                                               
//* ���� 2015-8-19        
//* ���� rwp                 
//* ע�� ƽ̨���                                 
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
#define  sleep(x)                  Sleep(x*1000)  //������Linux����ͬ����ʽ  
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
typedef  unsigned long             THREAD_ID;      //windows���߳�IDΪDWORD(unsiged long)
typedef  HANDLE                    THREAD_HANDLE;  //windows���̴߳����󷵻�ΪHANDLE (void *)
typedef  unsigned int              THREAD_FUNC;    //windows���߳���ں�����������
#define INVALID_THREAD             NULL            //CreateThread()ʧ�ܷ���NULL
#define THREAD_RETURN              0               //�̷߳���ֵΪDWORD

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
typedef pthread_t               THREAD_ID;      //linux���߳�IDΪpthread_t (unsigned int)
typedef pthread_t               THREAD_HANDLE;  //linux���̴߳����󷵻�Ϊint
typedef void *                  THREAD_FUNC;    //Linux���߳���ں�����������
#define WINAPI                                  //Linux��û��WINAPI(__stdcall) 
#define INVALID_THREAD          -1              //Linux���̴߳���ʧ�ܷ���-1
#define THREAD_RETURN           NULL            //Linux���̵߳ķ���ֵ   	
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
* @brief	�̴߳��������͵Ķ���
*/
#ifdef WIN32
	/**	
	* @brief	�������Եĳ������塣
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
