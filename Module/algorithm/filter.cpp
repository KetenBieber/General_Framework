#include "filter.h"

void LowPassFilter::in(float num)							
{
	last_num = now_num;
	now_num = num;
}

// 低通滤波器
float LowPassFilter::out()							
{
	return (now_num*Trust + last_num * (1 - Trust));
}


void LowPassFilter::operator <<(const float& num)			
{
	in(num);
}


void LowPassFilter::operator >>(float& num)
{
	num = out();
}


float LowPassFilter::f(float num)						
{
	in(num);
	return (out());
}
