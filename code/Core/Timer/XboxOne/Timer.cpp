#include "Core/System.h"
#include "Core/Timer/Timer.h"

namespace traktor
{
	namespace
	{

int64_t s_frequency = 0;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Timer", Timer, Object)

Timer::Timer()
{
	if (!s_frequency)
		QueryPerformanceFrequency(reinterpret_cast <LARGE_INTEGER* >(&s_frequency));

	m_frequency = s_frequency;
	reset();
}

void Timer::reset()
{
	QueryPerformanceCounter(reinterpret_cast< LARGE_INTEGER* >(&m_first));
	m_last = m_first;
}

double Timer::getElapsedTime() const
{
	int64_t curr;
	QueryPerformanceCounter(reinterpret_cast< LARGE_INTEGER* >(&curr));
	return double(curr - m_first) / m_frequency;
}

double Timer::getDeltaTime()
{
	int64_t curr;
	QueryPerformanceCounter(reinterpret_cast< LARGE_INTEGER* >(&curr));

	double delta = double(curr - m_last) / m_frequency;
	m_last = curr;

	return delta >= 0.0 ? delta : 0.0;
}

}