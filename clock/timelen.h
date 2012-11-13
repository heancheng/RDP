// timelen.h: interface for the Clock class.
// 目的是为了取得高精度的计时，精确到两个时间点的时间差。仅仅用于windows
//////////////////////////////////////////////////////////////////////

#if !defined(__TIME_LEN_H__)
#define __TIME_LEN_H__

#include <windows.h>

class TimeLen  
{
public:
	double Length();
	void Start();
	TimeLen();
	virtual ~TimeLen();
private:
	ULARGE_INTEGER  m_start;
	ULARGE_INTEGER  m_finish;
};

#endif // !defined(AFX_Clock_H__1B3F310D_05C1_429B_A48E_26F7A63343E3__INCLUDED_)

