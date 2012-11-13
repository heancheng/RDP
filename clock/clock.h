// Clock.h: interface for the Clock class.
// 目的是为了取得高精度的计时，精确到ＣＰＵ的时间片
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
