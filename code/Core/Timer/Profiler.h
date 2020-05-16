#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"
#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

// Uncomment this line if you want scope profiling enabled.
#define T_PROFILER_ENABLE

/*! \ingroup Core */
//@{

class Thread;
class OutputStream;

/*! Runtime profiler.
 * \ingroup Core
 *
 * The runtime profiler measures time spent in
 * scopes.
 */
class T_DLLCLASS Profiler
:	public Object
,	public ISingleton
{
	T_RTTI_CLASS;

public:
	struct Event
	{
		std::wstring name;
		uint32_t threadId;
		uint16_t depth;
		double start;
		double end;
		int32_t alloc;
	};

	/*! Profiler report listener.
	 */
	class IReportListener : public IRefCount
	{
	public:
		virtual void reportProfilerEvents(double currentTime, const AlignedVector< Event >& events) = 0;
	};

	/*! JSON report listener.
	 *
	 * Format of JSON is Chromium tracer compatible.
	 */
	class T_DLLCLASS JSONReportListener : public RefCountImpl< IReportListener >
	{
	public:
		JSONReportListener(OutputStream* output);

		virtual ~JSONReportListener();

		virtual void reportProfilerEvents(double currentTime, const AlignedVector< Event >& events) override final;

	private:
		Ref< OutputStream > m_output;
	};

	typedef void* handle_t;

	static Profiler& getInstance();

	/*! Set report listener.
	 */
	void setListener(IReportListener* listener);

	/*! Begin recording event.
	 */
	void beginEvent(const wchar_t* const name);

	/*! End recording event.
	 */
	void endEvent();

	/*! Get current time.
	 */
	double getTime() const;

protected:
	Profiler();

	virtual void destroy() override final;

private:
	struct ThreadEvents
	{
		AlignedVector< Event > events;
	};

	Ref< IReportListener > m_listener;
	Semaphore m_lock;
	AlignedVector< Event > m_events;
	AlignedVector< ThreadEvents* > m_threadEvents;
	ThreadLocal m_localThreadEvents;
	Timer m_timer;
};

/*! Scoped profiling event.
 * \ingroup Core
 */
class ProfilerScoped
{
public:
	ProfilerScoped(const wchar_t* const name)
	{
		Profiler::getInstance().beginEvent(name);
	}

	~ProfilerScoped()
	{
		Profiler::getInstance().endEvent();
	}
};

#if defined(T_PROFILER_ENABLE)
#	define T_PROFILER_BEGIN(name)	{ Profiler::getInstance().beginEvent(name); }
#	define T_PROFILER_END()			{ Profiler::getInstance().endEvent(); }
#	define T_PROFILER_SCOPE(name)	T_ANONYMOUS_VAR(ProfilerScoped)(name);
#else
#	define T_PROFILER_BEGIN(name)	{}
#	define T_PROFILER_END()			{}
#	define T_PROFILER_SCOPE(name)
#endif

//@}

}

