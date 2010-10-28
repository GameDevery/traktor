#ifndef traktor_online_Statistics_H
#define traktor_online_Statistics_H

#include <map>
#include "Online/IStatistics.h"

namespace traktor
{
	namespace online
	{

class IStatisticsProvider;
class TaskQueue;

class Statistics : public IStatistics
{
	T_RTTI_CLASS

public:
	virtual bool ready() const;

	virtual bool enumerate(std::set< std::wstring >& outStatIds) const;

	virtual bool get(const std::wstring& statId, float& outValue) const;

	virtual Ref< Result > set(const std::wstring& statId, float value);

	virtual Ref< Result > add(const std::wstring& statId, float valueDelta);

private:
	friend class SessionManager;

	Ref< IStatisticsProvider > m_provider;
	Ref< TaskQueue > m_taskQueue;
	mutable Semaphore m_lock;
	std::map< std::wstring, float > m_statistics;
	bool m_ready;

	Statistics(IStatisticsProvider* provider, TaskQueue* taskQueue);

	void callbackEnumStatistics(const std::map< std::wstring, float >& statistics);
};

	}
}

#endif	// traktor_online_Statistics_H
