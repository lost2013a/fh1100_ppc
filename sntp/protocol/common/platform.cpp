//****************************************************************************//
//* Copyright (c) 2007, 府河电气有限公司                                      
//* All rights reserved.                                                     
//* 文件 platform.h                                               
//* 日期 2015-8-19        
//* 作者 rwp                 
//* 注释 平台相关                                 
//****************************************************************************//
#include "platform.h"

//0 success; -1 error
int rw_thread_create(THREAD_HANDLE *phandle, THREAD_ID *pid, __THREAD_ROUTINE routine, void *param)
{
#ifdef WIN32

  unsigned int dwId;

  THREAD_HANDLE h;

  h = (THREAD_HANDLE)_beginthreadex(0, 0, routine, param, 0, &dwId);

  if(h == (THREAD_HANDLE)-1)
    return -1;

  if(phandle)
  {
    *phandle = h;
    if (pid)
      *pid = dwId;

    return 0;
  }
  else
  {
    CloseHandle(h);

    return -1;
  }
#else

  pthread_t t;

  if (pthread_create(&t, 0, routine, param) != 0)
    return -1;

  if (phandle)
  {
    *phandle = t;
    if (pid)
      *pid = t;
  }
  else
  {
    if (pid)
      *pid = t;
    pthread_detach(t);
  }

  return 0;
#endif
}


//0 success; -1 error
int rw_thread_detach(THREAD_HANDLE handle)
{
#ifdef WIN32
  {
    CloseHandle(handle);
    return 0;
  }
#else
  {
    return pthread_detach(handle);
  }
#endif
}


//0 success; -1 error
int rw_thread_join(THREAD_HANDLE handle, void **pretcode)
{
#ifdef WIN32
{
  if(WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0)
  {
    if(pretcode)
      GetExitCodeThread(handle, (unsigned long*)pretcode);

    return 0;
  }

  return -1;
}
#else
{
  return pthread_join(handle, pretcode);
}
#endif
}

void rw_thread_exit(unsigned int retval)
{
#ifdef WIN32
  {
    _endthreadex(retval);
  }
#else
  {
    pthread_exit((void *)retval);
  }
#endif

}

int rw_noname_sem_destroy(HNSEM *sem)
{
  #ifdef WIN32
{
  if(CloseHandle(*sem))
    return 0;
  return -1;
}
#else
{
  return sem_destroy(sem);
}
#endif
}

int rw_noname_sem_init(HNSEM *sem, unsigned int value)
{
  #ifdef WIN32
  *sem = CreateSemaphore(0, value, 0x7FFFFFFF, NULL);
  if(*sem == NULL)
    return -1;

  return 0;
#else
  return sem_init(sem, 0, value);
  #endif
}

int rw_noname_sem_post(HNSEM *sem)
{
#ifdef WIN32
  if(ReleaseSemaphore(*sem, 1, 0))
    return 0;

  return -1;
#else
  return sem_post(sem);
#endif
}

int rw_noname_sem_wait(HNSEM *sem)
{
  #ifdef WIN32
  int ret = WaitForSingleObject(*sem, INFINITE);

  if(ret == WAIT_OBJECT_0)
    return 0;
  else
    return -1;
#else
  return sem_wait(sem);
#endif
}