#pragma once

#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Timer/Timer.h"

namespace traktor
{

#define T_WIDEN_X(x) L ## x
#define T_WIDEN(x) T_WIDEN_X(x)

#if defined(T_ENABLE_MEASURE)

#	define T_MEASURE_BEGIN() \
		Timer s_timer; s_timer.start(); \
		double __M_start = s_timer.getElapsedTime(); \
		double __M_last = __M_start;

#	define T_MEASURE_UNTIL(maxTimeUntil) \
		{ \
			double __M_this = s_timer.getElapsedTime(); \
			if (__M_this - __M_last > (maxTimeUntil)) \
				log::warning << L"Time until \"" << mbstows( T_FILE_LINE ) << L"\" reached exceeded max " << int32_t(maxTimeUntil * 10000.0) / 10.0f << L" ms, " << int32_t((__M_this - __M_last) * 10000.0) / 10.0f << L" ms" << Endl; \
			__M_last = __M_this; \
		}

#	define T_MEASURE_UNTIL_M(maxTimeUntil, message) \
		{ \
			double __M_this = s_timer.getElapsedTime(); \
			if (__M_this - __M_last > (maxTimeUntil)) \
				log::warning << L"Time until \"" << mbstows( T_FILE_LINE ) << L"\" (" << (message) << L") reached exceeded max " << int32_t(maxTimeUntil * 10000.0) / 10.0f << L" ms, " << int32_t((__M_this - __M_last) * 10000.0) / 10.0f << L" ms" << Endl; \
			__M_last = __M_this; \
		}

#	define T_MEASURE_STATEMENT(statement, maxDuration) \
		{ \
			double start = s_timer.getElapsedTime(); \
			(statement); \
			double end = s_timer.getElapsedTime(); \
			if ((end - start) > maxDuration) \
				log::warning << L"Statement \"" << T_WIDEN(#statement) << L"\" exceeded max " << int32_t(maxDuration * 10000.0) / 10.0f << L" ms, " << int32_t((end - start) * 10000.0) / 10.0f << L" ms" << Endl; \
		}

#	define T_MEASURE_STATEMENT_M(statement, maxDuration, message) \
		{ \
			double start = s_timer.getElapsedTime(); \
			(statement); \
			double end = s_timer.getElapsedTime(); \
			if ((end - start) > maxDuration) \
				log::warning << L"Statement \"" << T_WIDEN(#statement) << L"\" (" << (message) << L") exceeded max " << int32_t(maxDuration * 10000.0) / 10.0f << L" ms, " << int32_t((end - start) * 10000.0) / 10.0f << L" ms" << Endl; \
		}

#	define T_MEASURE_SCOPE_M(maxDuration, message) \
		struct __measure_scope_raii__ \
		{ \
			std::wstring msg; \
			Timer timer; \
			double start; \
			__measure_scope_raii__(const std::wstring& _msg) { \
				msg = _msg; \
				timer.start(); \
				start = timer.getElapsedTime(); \
			} \
			~__measure_scope_raii__() { \
				double end = timer.getElapsedTime(); \
				if ((end - start) > (maxDuration)) \
					log::warning << L"Scope \"" << mbstows( T_FILE_LINE ) << L"\" (" << msg << L") exceeded max " << int32_t(maxDuration * 10000.0) / 10.0f << L" ms, " << int32_t((end - start) * 10000.0) / 10.0f << L" ms" << Endl; \
			} \
		} __M_scope((message));

#else

#	define T_MEASURE_BEGIN()
#	define T_MEASURE_UNTIL(maxTimeUntil)
#	define T_MEASURE_UNTIL_M(maxTimeUntil, message)
#	define T_MEASURE_STATEMENT(statement, maxDuration) { (statement); }
#	define T_MEASURE_STATEMENT_M(statement, maxDuration, message) { (statement); }
#	define T_MEASURE_SCOPE_M(maxDuration, message)

#endif

}

