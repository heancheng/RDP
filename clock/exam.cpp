#include "timelen.h"
#include <cstdio>

int main(void)
{
	TimeLen tlen;
	tlen.Start();
	Sleep(1000);
	printf("TimeLen : %.4f\n",tlen.Length());
	return 0;
}
