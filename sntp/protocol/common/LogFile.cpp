// LogFile.cpp: implementation of the CString class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4275)
#include "LogFile.h"
#include <stdarg.h>

CLogFile::CLogFile()
  :m_nLevel(CLogFile::trace)
{

}

CLogFile::~CLogFile()
{

}

bool CLogFile::Clear()
{
  return true;
}

bool CLogFile::SetLogLevel( int nLevel )
{
  m_nLevel = nLevel;
  return true;
}

int CLogFile::GetLogLevel()
{
  return 0;
}

bool CLogFile::SetLogPath( const char * pszPath )
{
  return true;
}

bool CLogFile::SetLogSize( long nSize )
{
  return true;
}

bool CLogFile::Open()
{
  return true;
}

bool CLogFile::Open( const char * pszFileName )
{
  return true;
}

bool CLogFile::Close()
{
  return true;
}

int CLogFile::FormatAdd( int nCurLevel,const char* formatString, ... )
{
	// if the format string is NULL ,return 
	if (formatString == NULL)
	{
		return -1;
	}
		
	va_list argList;

	// Set va_list to the beginning of optional arguments
	va_start(argList, formatString);

	const char * ptr = formatString;
	char * str = NULL;

	//save the max len of the formatstring
	int    nMaxLen = 0;

	while(*ptr != '\0')
	{
		str = NULL;

		if(*ptr == '%')
		{
			switch(*(ptr+1))
			{
			case 's':
			case 'S':
				str = va_arg(argList,char*);

				if( NULL == str)
					nMaxLen ++;
				else
					nMaxLen += strlen(str);
				ptr++;
				break;

			case 'c':
			case 'C':
				va_arg(argList,char);
				nMaxLen +=2;
				ptr++;
				break;

			case 'd':
			case 'D':
				va_arg(argList, int);
				nMaxLen +=11;
				ptr++;
				break;

			case 'u':
			case 'U':
				va_arg(argList, unsigned int);
				nMaxLen +=10;
				ptr++;
				break;

			case 'l':
			case 'L':
				ptr++;
				if(*(ptr+1) == 'd')
				{
					va_arg(argList, long);
					nMaxLen +=11;
				}
				else if(*(ptr+1) == 'u')
				{
					va_arg(argList, unsigned long);
					nMaxLen +=10;
				}
				ptr++;
				break;
			case 'f':
			case 'F':
				va_arg(argList, double);
				nMaxLen += 31;
				ptr++;
				break;
			case 'x':
			case 'X':
				va_arg(argList, void*);
				nMaxLen += 2*sizeof(void*);
				ptr++;
				break;
			default:
				nMaxLen+=1;
			}
		} //  if(*ptr == '%')
		else
		{
			nMaxLen +=1;
		}
		// Increment pointer..
		ptr++;
	}

	// end va_list
	va_end(argList);

	// allocate memory
	//char * pchar = new char[nMaxLen+1];

	//if(pchar == NULL)
	//	return 0;
	nMaxLen += 255;       // 防止特殊情况长度计算错误；
    string strDes=""; 
	strDes.resize(nMaxLen); 

	try{
		// get parament
		va_start(argList, formatString);  
		
		// format
		//vsprintf(pchar, formatString, argList);
        /*
        * ----- commented by qingch   3/12/2009 ------
          vsprintf((char*)strDes.c_str(), formatString, argList);
        */
		vsprintf((char*)&strDes[0], formatString, argList);
		// if the curlevel >= setted level,not write 
		// the number of level more bigger indicate the level is more lower
		if(nCurLevel <= m_nLevel)
		{
			// tty output	
			//printf("%s\n",pchar);
			printf("%s\n",strDes.c_str());
			
			// write file
			//Flush(pchar);
			Flush(strDes.c_str(),nCurLevel);
		}
		
		//if(pchar != NULL)
		//	delete [] pchar;
		
		va_end(argList);
	}
	catch(...)
	{
		return 0;
	}

	return nMaxLen;
}

void CLogFile::Add( const char * pchLog,int nCurLevel/*= trace*/ )
{
  if(nCurLevel <= m_nLevel)
  {
    //tty output
    printf("%s\n",pchLog);

    Flush(pchLog,nCurLevel);
  }
}

void CLogFile::AddFixLen( const char * pchLog,int nLen,int nCurLevel /*= trace*/ )
{
  if( (nLen <= 0) || (pchLog == NULL))
    return ;

  // if the curlevel >= setted level,not write 
  // the number of level more bigger indicate the level is more lower
  if(nCurLevel <= m_nLevel)
  {
    //tty output
    printf("%s\n",pchLog);

    Flush(pchLog,nLen,nCurLevel);
  }

}

void CLogFile::Flush( const char* data,int nCurLevel )
{

}

void CLogFile::Flush( const char* data,int nLen,int nCurLevel )
{

}
