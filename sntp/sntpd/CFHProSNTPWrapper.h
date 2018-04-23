#ifndef __FH_SNTP_WRAPPER
#define __FH_SNTP_WRAPPER

#include "../protocol/sntps/FHSNTPInface.h"
#ifdef OS_WINDOWS
	#include <Windows.h>
#else
	#include <dlfcn.h>  
#endif

typedef  void* t_handle;
typedef  bool (*FUNC_StartSNTP)(SNTP_CONFIG&);
typedef  bool (*FUNC_EndSNTP)();
typedef  int  (*FUNC_LeapSec)(char);

class CPSNTPWrapper  
{
public:
	CPSNTPWrapper();
	virtual ~CPSNTPWrapper();
	bool InitLibrary();
	bool StartSNTP( SNTP_CONFIG& pConfig );
	bool EndSNTP();
	int  LeapSec(char flag);
private:
	void 	 WriteLog(int iLevel,const char *format, ...);
	t_handle lib_load_library(const char *filename);
	void 	*lib_get_symbol(t_handle handle, const char *module);
	int  	 lib_free_library(t_handle handle);
private:
	t_handle 		m_hHandle;
	bool 			m_bInit;
	FUNC_StartSNTP  m_pFunc_StartSNTP;
	FUNC_EndSNTP 	m_pFunc_EndSNTP;
	FUNC_LeapSec 	m_pFunc_LeapSec;
};

#endif 
