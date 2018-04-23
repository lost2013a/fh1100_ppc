// Lock.cpp: implementation of the CLock class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4275)
#include "Lock.h"

//////////////////////////////////////////////////////////////////////////
//                  Class  CLock   ʵ��                                    
//////////////////////////////////////////////////////////////////////////

/*************************************************************
 �� �� �� : CLock()
 ���ܸ�Ҫ : ���캯��
 �� �� ֵ : �� 
 ��    �� : 
            char * phost : ��־���Ǹ�ģ����ʹ�øû����� 
*************************************************************/
CLock::CLock()
{
	memset(ch_host,0,FILE_NAME_MAX_LEN);

	bInit = false;
}

/*************************************************************
 �� �� �� : CLock()
 ���ܸ�Ҫ : ��������
 �� �� ֵ : �� 
 ��    �� : ��
*************************************************************/
CLock::~CLock()
{
	if(bInit)
		del();
}

/*************************************************************
 �� �� �� : init()
 ���ܸ�Ҫ : ��ʼ�����������
 �� �� ֵ : bool 
 ��    �� : 
            char * phost : ��־���Ǹ�ģ����ʹ�øû�����
*************************************************************/
//##ModelId=4519E3D00375
bool CLock::init(char * phost)
{
	int n =0;

	//��ʼ��ch_host
	if( (phost == NULL) ||
		(strlen(phost) > FILE_NAME_MAX_LEN) )
		strcpy(ch_host,"CLock");
	else
		strcpy(ch_host,phost);

#ifdef OS_WINDOWS 
	//init windows mutex
	//CRITICAL_SECTION Ĭ��֧�ֵݹ���� 
	::InitializeCriticalSection(&m_mutex);
#endif

#ifdef OS_LINUX

	//init linux mutex
	//����mutex����Ϊ֧�ֵݹ�
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	// ֧�ֵݹ�
	n = pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);  
	if(n != 0){
		printf("init the mutex attr of %s failed,reason:%s(%d)\n",\
			   ch_host,strerror(n),n);
		return false;
	}
	n = pthread_mutex_init(&m_mutex,&attr);
	pthread_mutexattr_destroy(&attr);
    
	if(n != 0){
		printf("init the mutex of %s failed,reason:%s(%d)\n", \
			   ch_host,strerror(n),n);
		return false;
	}
	
#endif
	bInit = true;
	return bInit;
}

/*************************************************************
 �� �� �� : del()
 ���ܸ�Ҫ : ɾ�����������
 �� �� ֵ : bool
 ��    �� : ��
*************************************************************/
//##ModelId=4519E3E701F2
bool CLock::del()
{
	int n =0;

#ifdef OS_WINDOWS
	//del windows mutex
	::DeleteCriticalSection(&m_mutex);
#endif

#ifdef OS_LINUX

	//del linux mutex
	n= pthread_mutex_destroy(&m_mutex);
    
	if(n != 0){
		printf("del the mutex of %s failed,reason:%s(%d)\n", \
			   ch_host,strerror(n),n);
		return false;
	}
	
#endif
	bInit = false;
	return true;
}

/*************************************************************
 �� �� �� : lock()
 ���ܸ�Ҫ : �����ȴ����ӵ��Ȩ
 �� �� ֵ : void 
 ��    �� : ��
*************************************************************/
//##ModelId=4519E3F40088
bool CLock::lock()
{
	int n =0;

#ifdef OS_WINDOWS
	//get ownership blocking
	::EnterCriticalSection(&m_mutex);
#endif

#ifdef OS_LINUX

	//get ownership blocking
	n= pthread_mutex_lock(&m_mutex);
    
	if(n != 0){
		printf("blocking get the mutex ownership of %s failed,reason:%s(%d)\n", \
			   ch_host,strerror(n),n);
		return false;
	}

#endif	
	return true;
}

/*************************************************************
 �� �� �� : trylock()
 ���ܸ�Ҫ : �����Ƿ�ȡ��ӵ��Ȩ,������������(��WINDOWS�»�������)
 �� �� ֵ : bool 
            true :  �ɹ��������Ȩ
			false:  �����屻�����߳�����,���ʧ��
 ��    �� : ��
*************************************************************/
bool CLock::trylock()
{
	int n =0;

#ifdef OS_WINDOWS
	//get ownership unblocking
	/*n = TryEnterCriticalSection(&m_mutex);  //WINCE֧��,2000�²�֧��
	if(n == false){
		printf("TryEnterCriticalSection the owneship of %s failed,reason:%s(%d)",\
			    ch_host,strerror(errno),errno);
		return false;
	}*/
    ::EnterCriticalSection(&m_mutex);
#endif

#ifdef OS_LINUX

	//get ownership unblocking
	n= pthread_mutex_trylock(&m_mutex);
    
	if(n != 0){
		printf("unblocking get the mutex ownership of %s failed,reason:%s(%d)\n", \
			   ch_host,strerror(n),n);
		return false;
	}

#endif	
	return true;
}

/*************************************************************
 �� �� �� : unlock()
 ���ܸ�Ҫ : �ó�ӵ��Ȩ
 �� �� ֵ : bool
 ��    �� : ��
*************************************************************/
//##ModelId=4519E40E003F
bool CLock::unlock()
{
	int n =0;

#ifdef OS_WINDOWS
	//release ownership 
	::LeaveCriticalSection(&m_mutex);
#endif

#ifdef OS_LINUX

	//release ownership 
	n= pthread_mutex_unlock(&m_mutex);
    
	if(n != 0){
		printf("release the mutex ownership of %s failed,reason:%s(%d)\n", \
			   ch_host,strerror(n),n);
		return false;
	}
#endif		
	return true;
}
