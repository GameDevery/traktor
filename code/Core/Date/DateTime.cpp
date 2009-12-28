#include <ctime>
#include "Core/Date/DateTime.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#if defined(WINCE)
#include "time_ce.h"
#endif

namespace traktor
{
	namespace
	{

void getLocalTime(time_t t, struct tm* T)
{
#if defined(_MSC_VER)
#	if !defined(WINCE)
	::localtime_s(T, &t);
#	else
	struct tm* tmp = localtime_ce(&t);
	T_ASSERT (tmp);
	std::memcpy(T, tmp, sizeof(struct tm));
#	endif
#else
	struct tm* tmp = ::localtime(&t);
	T_ASSERT (tmp);
	std::memcpy(T, tmp, sizeof(struct tm));
#endif
}

	}

DateTime::DateTime()
:	m_epoch(0)
{
}

DateTime::DateTime(uint64_t seconds)
:	m_epoch(seconds)
{
}

DateTime::DateTime(uint16_t year, uint8_t month, uint16_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
	T_ASSERT (month >= 1 && month <= 12);
	T_ASSERT (day >= 1 && day <= 31);
	T_ASSERT (hour <= 23);
	T_ASSERT (minute <= 59);
	T_ASSERT (second <= 59);

	struct tm t;
	t.tm_sec = second;
	t.tm_min = minute;
	t.tm_hour = hour;
	t.tm_mday = day - 1;
	t.tm_mon = month - 1;
	t.tm_year = year - 1900;
	t.tm_wday = 0;
	t.tm_yday = 0;
	t.tm_isdst = 0;

#if !defined(WINCE)
	m_epoch = mktime(&t);
#else
	m_epoch = mktime_ce(&t);
#endif
}

DateTime DateTime::now()
{
#if !defined(WINCE)
	time_t t; ::time(&t);
	return DateTime(uint64_t(t));
#else
	time_t t; ::time_ce(&t);
	return DateTime(uint64_t(t));
#endif
}

DateTime DateTime::parse(const std::wstring& str)
{
	T_BREAKPOINT;
	return DateTime();
}

uint16_t DateTime::getYear() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_year + 1900;
}

uint8_t DateTime::getMonth() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_mon + 1;
}

uint8_t DateTime::getDay() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_mday;
}

uint8_t DateTime::getWeekDay() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_wday;
}

uint16_t DateTime::getYearDay() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_yday;
}

bool DateTime::isLeapYear() const
{
	int32_t year = getYear();

	if (year % 400 == 0)
		return true;

	if (year % 100 == 0)
		return false;

	if (year % 4 == 0)
		return true;

	return false;
}

uint8_t DateTime::getHour() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_hour;
}

uint8_t DateTime::getMinute() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_min;
}

uint8_t DateTime::getSecond() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_sec;
}

uint64_t DateTime::getSecondsSinceEpoch() const
{
	return m_epoch;
}

std::wstring DateTime::format(const std::wstring& fmt) const
{
	struct tm T;
	getLocalTime(m_epoch, &T);

	wchar_t buf[256];
	wcsftime(buf, sizeof_array(buf), fmt.c_str(), &T);

	return buf;
}

bool DateTime::operator == (const DateTime& dt) const
{
	return m_epoch == dt.m_epoch;
}

bool DateTime::operator != (const DateTime& dt) const
{
	return !(*this == dt);
}

bool DateTime::serialize(ISerializer& s)
{
	return s >> Member< uint64_t >(L"epoch", m_epoch);
}

}
