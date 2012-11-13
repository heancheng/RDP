// Clock.h: interface for the Clock class.
// Ŀ����Ϊ��ȡ�ø߾��ȵļ�ʱ����ȷ���ãУյ�ʱ��Ƭ
//////////////////////////////////////////////////////////////////////

#if !defined(__CLOCK_H__)
#define __CLOCK_H__

#include <time.h>

class Clock  
{
public:
	double Length();
	void Start();
	Clock();
	virtual ~Clock();
private:
	clock_t m_start;
	clock_t m_finish;
};

#endif // !defined(AFX_Clock_H__1B3F310D_05C1_429B_A48E_26F7A63343E3__INCLUDED_)
