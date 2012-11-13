// timelen.h: interface for the Clock class.
// Ŀ����Ϊ��ȡ�ø߾��ȵļ�ʱ����ȷ������ʱ����ʱ����������windows
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

