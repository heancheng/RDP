// TimeLen.cpp: implementation of the TimeLen class.
//
//////////////////////////////////////////////////////////////////////

#include "timelen.h"
#include <cstring>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// 兼容性比较强的是ftime(timeb *tm)函数，可以取得微秒级的时长。
// 兼容windows系统和unix系统，但是存在时区区分，需要对比时区。

TimeLen::TimeLen()
{
	Start();
}

TimeLen::~TimeLen()
{

}

void TimeLen::Start()
{
	SYSTEMTIME 	stime;
	FILETIME	ftime;
	GetSystemTime(&stime);
	SystemTimeToFileTime(&stime,&ftime);
	memcpy(&m_start,&ftime,sizeof(FILETIME));
}

double TimeLen::Length()
{
	SYSTEMTIME 	stime;
	FILETIME	ftime;
	GetSystemTime(&stime);
	SystemTimeToFileTime(&stime,&ftime);
	memcpy(&m_finish,&ftime,sizeof(FILETIME));
	return (double)(m_finish.QuadPart - m_start.QuadPart) / 10000000;
}

