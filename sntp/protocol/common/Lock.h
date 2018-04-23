
#ifndef LOCK_H_HEADER_INCLUDED_BAD5A27F
#define LOCK_H_HEADER_INCLUDED_BAD5A27F

#define FILE_NAME_MAX_LEN 256
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <process.h>
#include <conio.h>
#include <windows.h>
//lock
typedef CRITICAL_SECTION			MUTEX;          //windows�µĻ���������
#else
//mutex
#include <sys/types.h>  
#include <netinet/in.h>  
#include <sys/socket.h>  
#include <sys/wait.h>  
#include <arpa/inet.h>       
#include <unistd.h>         
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h> 
#include <termios.h>
#include <netdb.h>
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

typedef pthread_mutex_t         MUTEX;          //Linux���̻߳���������
typedef pthread_cond_t			HCOND;

#endif
//##ModelId=4519DA5403C4
//##Documentation
//## �������࣬����̼߳����������
class CLock
{
public:
  //���캯��
  CLock();

  //��������
  ~CLock();

  //��ʼ����
  //##ModelId=4519E3D00375
  bool init(char * phost = NULL);

  //ɾ����
  //##ModelId=4519E3E701F2
  bool del();

  //�����ȴ�ֱ��ȡ��ӵ��Ȩ
  //##ModelId=4519E3F40088
  bool lock();

  //�����Ƿ�ȡ��ӵ��Ȩ,�������Ϸ���
  bool trylock();

  //�ó�ӵ��Ȩ
  //##ModelId=4519E40E003F
  bool unlock();

private:
  //##ModelId=4519E338018C
  MUTEX m_mutex;

  //��mutex����������
  char  ch_host[FILE_NAME_MAX_LEN];

  //��ʼ����־
  bool  bInit;

};



#endif /* LOCK_H_HEADER_INCLUDED_BAD5A27F */
