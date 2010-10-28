#ifndef traktor_online_PsnStatistics_H
#define traktor_online_PsnStatistics_H

#include "Online/Provider/IStatisticsProvider.h"

namespace traktor
{
	namespace online
	{

class PsnStatistics : public IStatisticsProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::map< std::wstring, float >& outStats);

	virtual bool set(const std::wstring& statId, float value);
};

	}
}

#endif	// traktor_online_PsnStatistics_H
