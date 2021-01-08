#pragma once

#include "Runtime/Target/TargetPerformance.h"
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Timer/Profiler.h"

namespace traktor
{

class ILogTarget;

	namespace net
	{

class BidirectionalObjectTransport;

	}

	namespace script
	{

class IScriptDebuggerSessions;

	}

	namespace runtime
	{

class TargetScriptDebugger;
class TargetScriptProfiler;

/*! \brief
 * \ingroup Runtime
 */
class TargetConnection : public Object
{
	T_RTTI_CLASS;

public:
	struct IProfilerEventsCallback
	{
		virtual void receivedProfilerDictionary(const SmallMap< uint16_t, std::wstring >& dictionary) = 0;

		virtual void receivedProfilerEvents(double, const AlignedVector< Profiler::Event >& events) = 0;

		virtual void receivedPerfSets() = 0;
	};

	TargetConnection(
		const std::wstring& name,
		net::BidirectionalObjectTransport* transport,
		ILogTarget* targetLog,
		script::IScriptDebuggerSessions* targetDebuggerSessions
	);

	virtual ~TargetConnection();

	void destroy();

	void shutdown();

	bool update();

	const std::wstring& getName() const { return m_name; }

	net::BidirectionalObjectTransport* getTransport() const { return m_transport; }

	const TargetPerfSet* getPerformance(const TypeInfo& perfSetType);

	template < typename PerfType >
	const PerfType& getPerformance() {
		return *(const PerfType*)getPerformance(type_of< PerfType >());
	}

	void setProfilerEventsCallback(IProfilerEventsCallback* profilerEventsCallback);

private:
	std::wstring m_name;
	Ref< net::BidirectionalObjectTransport > m_transport;
	Ref< ILogTarget > m_targetLog;
	Ref<script::IScriptDebuggerSessions> m_targetDebuggerSessions;
	Ref< TargetScriptDebugger > m_targetDebugger;
	Ref< TargetScriptProfiler > m_targetProfiler;
	SmallMap< const TypeInfo*, Ref< TargetPerfSet > > m_performance;
	SmallMap< uint16_t, std::wstring > m_dictionary;
	IProfilerEventsCallback* m_profilerEventsCallback;
	Semaphore m_lock;
};

	}
}

