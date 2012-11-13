// Clock.cpp: implementation of the Clock class.
//
//////////////////////////////////////////////////////////////////////

#include "clock.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Clock::Clock()
{
	Start();
}

Clock::~Clock()
{

}

void Clock::Start()
{
	m_start=clock();
	if(m_start==-1)
		throw "ERROR::Clock the amount of elapsed time is unavailable.";
}

double Clock::Length()
{
	m_finish = clock();
	return (double)(m_finish - m_start) / CLOCKS_PER_SEC;
}
